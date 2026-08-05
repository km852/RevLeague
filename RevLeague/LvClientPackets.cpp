#include "LvClient.h"
#include "LvGame.h"
#include "LvPlayer.h"
#include "LvProtocol.h"

LvPacketProcessingResult LvClient::HandleRegistration(NvBinaryStreamRead& stream)
{
	if (this->GetState() != CST_UNKNOWN)
		return PPR_SUCCESS;

	unsigned char header = (unsigned char)(stream.Read<unsigned int>() & 0xFF); // always 0
	unsigned int cid = stream.Read<int>(); // always 0 for incoming packet
	unsigned long long cleartextUserId = stream.Read<unsigned long long>();
	unsigned int version = stream.Read<int>(); // always 0
	unsigned long long encryptedUserId = stream.Read<unsigned long long>();
	// further packet data is of no interest

	LvPlayer* player = lvGame->GetPlayerByUserId(cleartextUserId);
	if (player == nullptr)
	{
		LogWarning("Registration packet with player ID {} rejected (no such player)", cleartextUserId);
		return PPR_FAIL_DISCONNECT;
	}

	if (player->Encrypt(cleartextUserId) != encryptedUserId)
	{
		LogWarning("Registration packet with player ID {} rejected (wrong key)", cleartextUserId);
		return PPR_FAIL_DISCONNECT;
	}

	if (player->GetClient() != nullptr)
	{
		if (!LogAssert(player->GetClient() != this)) // this shouldn't ever happen, because a player shouldn't have been assigned a client in CST_UNKNOWN state
			return PPR_SUCCESS;

		LogWarning("Registration packet with player ID {} rejected (player occupied)", cleartextUserId);
		return PPR_FAIL_DISCONNECT;
	}

	this->SetState(CST_REGISTERED);
	this->SetPlayer(player);

	LogInfo("Player {} ({}) connecting from {}", cleartextUserId, player->GetPlayerName(), this->GetEndpointString());

	for (LvPlayer* connPlayer : lvGame->GetConnectedPlayers())
		connPlayer->GetClient()->SendPacket(PCH_Registration, LvProtocol::CreatePeerRegistration(player, player == connPlayer ? encryptedUserId : 0)); // surprisingly this packet is encrypted

	this->SendPacket(PCH_ServerToClient, LvProtocol::CreateSendGameNumber(player));
	return PPR_SUCCESS;
}

LvPacketProcessingResult LvClient::HandleQueryStatus()
{
	if (this->GetState() != CST_REGISTERED)
		return PPR_SUCCESS;

	this->SendPacket(PCH_ServerToClient, LvProtocol::CreateQueryStatusAnswer());
	return PPR_SUCCESS;
}

LvPacketProcessingResult LvClient::HandleSynchVersion(NvBinaryStreamRead& stream)
{
	if (this->GetState() != CST_REGISTERED)
		return PPR_SUCCESS;

	stream.SkipBytes(9); // skip header + netId + useless cid

	char versionStringBuf[256];
	stream.ReadBytes(versionStringBuf, 256);

	versionStringBuf[std::size(versionStringBuf) - 1] = 0;
	if (strcmp(versionStringBuf, EXPECTED_GAME_VERSION_STRING) != 0)
	{
		LogWarning("Player {} ({}): invalid game version: got \"{}\", expected \"{}\"", this->GetPlayer()->GetUserId(), this->GetPlayer()->GetPlayerName(),
			versionStringBuf, EXPECTED_GAME_VERSION_STRING);
		return PPR_FAIL_DISCONNECT;
	}

	this->SetState(CST_LOADING);
	this->SendPacket(PCH_ServerToClient, LvProtocol::CreateSynchVersionAnswer());

	return PPR_SUCCESS;
}

LvPacketProcessingResult LvClient::HandleCharSelected()
{
	if (this->GetState() != CST_LOADING)
		return PPR_SUCCESS;

	this->SendPacket(PCH_LoadingScreen, LvProtocol::CreateLoadingTeamRosterUpdate());

	for (LvPlayer* player : lvGame->GetPlayers())
	{
		this->SendPacket(PCH_LoadingScreen, LvProtocol::CreateLoadingSetName(player));
		this->SendPacket(PCH_LoadingScreen, LvProtocol::CreateLoadingSetSkin(player));
	}

	return PPR_SUCCESS;
}

LvPacketProcessingResult LvClient::HandlePingLoadInfo(NvBinaryStreamRead& stream)
{
	if (this->GetState() != CST_LOADING && this->GetState() != CST_LOADED && this->GetState() != CST_POST_LOADED)
		return PPR_SUCCESS;

	stream.SkipBytes(5); // header + netId (unused)
	stream.SkipBytes(4); // cid (obviously we know it)
	stream.SkipBytes(8); // userId (we know it as well)

	float percentageLoaded = stream.Read<float>();
	float eta = stream.Read<float>();
	unsigned int extraInfo = stream.Read<unsigned int>();
	bool isReady = (stream.Read<unsigned char>() & 1) != 0; // I suspect "isReady" was supposed to be a part of "extraInfo", but due to a League bug it ended up in a separate byte.

	this->loadingProgress = LoadingProgress(percentageLoaded, eta, extraInfo, isReady);

	return PPR_SUCCESS;
}

LvPacketProcessingResult LvClient::HandleClientReady()
{
	if (this->GetState() != CST_LOADING)
		return PPR_SUCCESS;

	this->SetState(CST_LOADED);
	LogInfo("Player {} ({}): loading completed", this->GetPlayer()->GetUserId(), this->GetPlayer()->GetPlayerName());

	return PPR_SUCCESS;
}
