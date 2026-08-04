#include "LvClient.h"
#include "LvGame.h"
#include "LvPlayer.h"

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

	return PPR_SUCCESS;
}
