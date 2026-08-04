#include "LvPlayer.h"
#include "LvClient.h"

static int NextPlayerIndex = 0;
static int NextBluePlayerIndex = 0;
static int NextRedPlayerIndex = 0;

LvPlayer::LvPlayer(LvGameSettings::PlayerInfo* playerInfo)
{
	this->blowfish = std::make_unique<CBlowFish>(playerInfo->netEncryptionKey.data(), playerInfo->netEncryptionKey.size());

	this->userId = playerInfo->playerId;
	this->globalPlayerIndex = NextPlayerIndex++;
	this->teamPlayerIndex = playerInfo->team == TT_BLUE ? NextBluePlayerIndex++ : NextRedPlayerIndex;
	this->initialTeam = playerInfo->team;
	this->currentTeam = this->initialTeam;
	this->spawnPointIndex = 0;
	this->skinIndex = 0;
	this->profileIconId = 8;
	this->playerName = playerInfo->playerName;
	this->characterName = playerInfo->characterName;

	LogDebug("> [{}] Player {}: {}", userId, globalPlayerIndex, playerName);
}

void LvPlayer::SetClient(LvClient* client)
{
	this->associatedClient = client;
	if (client && this->associatedClient->GetPlayer() != this)
		this->associatedClient->SetPlayer(this);
}

unsigned long long LvPlayer::Encrypt(unsigned long long value)
{
	this->Encrypt((unsigned char*)&value, sizeof(unsigned long long));
	return value;
}

void LvPlayer::Encrypt(unsigned char* buffer, size_t bufferLength)
{
	this->blowfish->Encrypt(buffer, bufferLength - (bufferLength % 8));
}

void LvPlayer::Encrypt(ENetPacket* packet)
{
	this->Encrypt(packet->data, packet->dataLength);
}

unsigned long long LvPlayer::Decrypt(unsigned long long value)
{
	this->Decrypt((unsigned char*)&value, sizeof(unsigned long long));
	return value;
}

void LvPlayer::Decrypt(unsigned char* buffer, size_t bufferLength)
{
	this->blowfish->Decrypt(buffer, bufferLength - (bufferLength % 8));
}
