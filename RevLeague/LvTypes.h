#pragma once

#include <cstdint>
#include <bit>
#include <memory>

#include "Dependencies/json.hpp"

// Platform sanity checks, we never expect any of the below to fail.
static_assert(sizeof(short) == sizeof(unsigned short));
static_assert(sizeof(short) == 2);
static_assert(sizeof(int) == sizeof(unsigned int));
static_assert(sizeof(int) == 4);
static_assert(sizeof(long long) == sizeof(unsigned long long));
static_assert(sizeof(long long) == 8);
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);
static_assert(std::endian::native == std::endian::little);

using NetworkId = unsigned int;

class LvGame;
class LvNetwork;
struct LvGameSettings;
class LvGameClock;
class LvGameTimer;
class LvPlayer;
class LvClient;

enum LvTeam {
	TT_NONE,
	TT_BLUE,
	TT_RED,
	TT_NEUTRAL
};

NLOHMANN_JSON_SERIALIZE_ENUM(LvTeam,
{
	{ TT_NONE, "TT_NONE" }, { TT_BLUE, "TT_BLUE" }, { TT_RED, "TT_RED" }, { TT_NEUTRAL, "TT_NEUTRAL" },
});

enum LvGameState {
	GST_PRELOADING,
	GST_LOADING,
	GST_GAMEPLAY
};

enum LvPacketChannel : unsigned char {
	PCH_Registration = 0,
	PCH_ClientToServer = 1,
	PCH_Gameplay = 2,
	PCH_ServerToClient = 3,
	PCH_LowPriority = 4,
	PCH_Communication = 5,
	PCH_LoadingScreen = 6
};

enum LvClientState {
	CST_UNKNOWN, // just connected, no registration packet sent yet
	CST_REGISTERED, // registration packet was valid
	CST_LOADING, // synchversion packet was valid
	CST_LOADED, // clientready packet was valid
	CST_POST_LOADED, // endspawn packet was sent
	CST_IN_GAME // startgame packet was sent
};

enum LvPacketProcessingResult {
	PPR_SUCCESS, // packet successfully processed, do nothing
	PPR_FAIL_PRINT, // packet was not processed, ignore it and print it to console
	PPR_FAIL_DISCONNECT, // packet was not processed, disconnect the peer
	PPR_FAIL_PRINT_DISCONNECT // packet was not processed, print to console and disconnect the peer
};
