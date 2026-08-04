#include "LvClient.h"
#include "LvPlayer.h"

LvClient::LvClient(ENetPeer* peer)
{
    this->peer = peer;
    this->endpointString = std::format("{}.{}.{}.{}:{}", (uint8_t)(peer->address.host & 0xFF), (uint8_t)((peer->address.host >> 8) & 0xFF),
        (uint8_t)((peer->address.host >> 16) & 0xFF), (uint8_t)((peer->address.host >> 24) & 0xFF), peer->address.port);
}

void LvClient::SetState(LvClientState newState)
{
	this->state = newState;
}

void LvClient::SetPlayer(LvPlayer* player)
{
	this->associatedPlayer = player;
	if (player && this->associatedPlayer->GetClient() != this)
		this->associatedPlayer->SetClient(this);
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

    return PPR_FAIL_PRINT;
}
