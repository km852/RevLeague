#pragma once

#include "NvLib.h"
#include "LvGameSettings.h"
#include "LvTypes.h"

#include "Dependencies/blowfish.h"
#include "Dependencies/enet/enet.h"

// LvPlayer is a class containing persistent information about a player (persistent = stored between reconnections)
// For a class containing per-connection data, see LvClient.
class LvPlayer : public NvNonCopyable {
	std::unique_ptr<CBlowFish> blowfish;

	unsigned long long userId;
	int globalPlayerIndex;
	int teamPlayerIndex;

	LvTeam initialTeam;
	LvTeam currentTeam;

	int spawnPointIndex;
	int skinIndex;
	int profileIconId;

	std::string playerName;
	std::string characterName;

	LvClient* associatedClient = nullptr;

public:
	LvPlayer(LvGameSettings::PlayerInfo* playerInfo);

	unsigned long long GetUserId() const { return userId; }
	int GetPlayerIndex() const { return globalPlayerIndex; }
	int GetTeamPlayerIndex() const { return teamPlayerIndex; }

	LvTeam GetInitialTeam() const { return initialTeam; }
	LvTeam GetCurrentTeam() const { return currentTeam; }

	int GetSpawnPointIndex() const { return spawnPointIndex; }
	int GetSkinIndex() const { return skinIndex; }
	int GetProfileIconId() const { return profileIconId; }

	const std::string& GetPlayerName() const { return playerName; }
	const std::string& GetCharacterName() const { return characterName; }

	LvClient* GetClient() { return associatedClient; }

	// DO NOT CALL THIS FUNCTION. This is reserved only for LvClient::SetPlayer. Use that instead.
	void SetClient(LvClient* client);

	unsigned long long Encrypt(unsigned long long value);
	void Encrypt(unsigned char* buffer, size_t bufferLength);
	void Encrypt(ENetPacket* packet);

	unsigned long long Decrypt(unsigned long long value);
	void Decrypt(unsigned char* buffer, size_t bufferLength);
};
