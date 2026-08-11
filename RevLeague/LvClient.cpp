#include "LvClient.h"
#include "LvPlayer.h"
#include "LvGame.h"
#include "LvMap.h"
#include "LvProtocol.h"

LvClient::LvClient(ENetPeer* peer)
{
    this->peer = peer;
    this->endpointString = std::format("{}.{}.{}.{}:{}", (uint8_t)(peer->address.host & 0xFF), (uint8_t)((peer->address.host >> 8) & 0xFF),
        (uint8_t)((peer->address.host >> 16) & 0xFF), (uint8_t)((peer->address.host >> 24) & 0xFF), peer->address.port);
}

LvClient::~LvClient()
{
	this->SetPlayer(nullptr);
}

void LvClient::SetState(LvClientState newState)
{
	LogDebug("Setting state to {}", (int)newState);
	this->state = newState;
}

void LvClient::SetPlayer(LvPlayer* player)
{
	if (player)
	{
		LogAssert(player->GetClient() == nullptr);
		if (!LogAssert(this->associatedPlayer == nullptr))
			this->associatedPlayer->SetClient(nullptr);

		this->associatedPlayer = player;
		player->SetClient(this);
	}
	else
	{
		if (this->associatedPlayer)
			this->associatedPlayer->SetClient(nullptr);

		this->associatedPlayer = nullptr;
	}
}

void LvClient::FinishLoading()
{
	if (!LogAssert(this->state == CST_LOADED))
		return;

	this->SendPacket(PCH_ServerToClient, LvProtocol::CreateStartSpawn());
	lvMap->SendInitialUnitState(this);
	this->SendPacket(PCH_ServerToClient, LvProtocol::CreateEndSpawn());

	this->SetState(CST_POST_LOADED);
}

void LvClient::UpdateClock()
{
}

void LvClient::SendPacket(LvPacketChannel channelId, ENetPacket* packet, bool encryptPacket)
{
	LogAssert(channelId != PCH_ClientToServer); // did you perhaps mean ServerToClient?

	if (encryptPacket && this->GetPlayer())
		this->GetPlayer()->Encrypt(packet);

	lvGame->AddPacketToQueue(this, channelId, packet);
}

void LvClient::SendPacket(LvPacketChannel channelId, const NvBinaryStreamWrite& packet, unsigned int flags, bool encryptPacket)
{
	const auto& pktData = packet.GetUnderlyingBuffer();
	this->SendPacket(channelId, enet_packet_create(pktData.data(), pktData.size(), flags), encryptPacket);
}

LvPacketProcessingResult LvClient::ProcessPacket(LvPacketChannel channel, unsigned char* packetData, size_t packetSize)
{
	if (packetSize == 0)
		return PPR_SUCCESS;

	if (this->GetState() >= CST_REGISTERED)
		this->GetPlayer()->Decrypt(packetData, packetSize);

	NvBinaryStreamRead packetStream(packetData, packetSize);
	uint8_t packetHeader = *packetData; // this is safe, we checked for packetSize == 0 previously

	if (channel == PCH_Registration)
		return this->HandleRegistration(packetStream);

	if (channel == PCH_ClientToServer)
	{
		switch (packetHeader)
		{
		case LvProtocol::PKT_C2S_QueryStatusReq:
			return this->HandleQueryStatus();

		case LvProtocol::PKT_SynchVersionC2S:
			return this->HandleSynchVersion(packetStream);

		case LvProtocol::PKT_C2S_Ping_Load_Info:
			return this->HandlePingLoadInfo(packetStream);

		case LvProtocol::PKT_C2S_ClientReady:
			return this->HandleClientReady();
		}
	}

	if (channel == PCH_LoadingScreen)
	{
		if (packetHeader == LvProtocol::PKT_C2S_CharSelected)
			return this->HandleCharSelected();
	}

    return PPR_FAIL_PRINT;
}
