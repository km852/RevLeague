#pragma once

#include "NvLib.h"
#include "LvTypes.h"

struct LvGameSettings final : public NvNonCopyable {
	struct PlayerInfo final {
		unsigned long long playerId;
		std::string playerName;
		std::string characterName;
		std::array<unsigned char, 16> netEncryptionKey;
		LvTeam team;

		PlayerInfo(const nlohmann::json& obj);
	};

	std::string assetFilePath;
	unsigned int listenHost; // host-order IPv4 address
	unsigned short listenPort; // host-order
	unsigned long long gameId;
	std::vector<PlayerInfo> players;

	LvGameSettings(const nlohmann::json& obj);
};
