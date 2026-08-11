#pragma once

#include <cstdint>
#include <bit>
#include <memory>
#include <ranges>

#include "Vector3.h"
#include "NvLib.h"
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
class LvClientVisionInfo;
class LvMap;
class LvMeshCell;
class LvMesh;
class LvDebugInterface;
class IMapScript;
class MapSRScript;
class ICharacterScript;
class LvObjectFactory;

class LvObjectBase;
class LvObjectHero;
class LvObjectTurret;
class LvObjectInhibitor;
class LvObjectNexus;

class LvStatReplicator;
class LvStatsBase;
class LvStatsHero;

class CharData;

typedef ICharacterScript* (*CreateCharacterScript_t)(LvObjectBase* object);

enum LvTeam {
	TT_NONE = 0,
	TT_BLUE = 100,
	TT_RED = 200,
	TT_NEUTRAL = 300
};

template<> inline void NvBinaryStreamWrite::Write(const LvTeam& val) { Write<int>(val); }

enum LvObjectType {
	OBJ_BASE,
	OBJ_HERO
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

// TODO: verify that channels 2, 3 and 4 don't really differ in processing
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

enum LvMapId {
	MID_UNKNOWN = -1,
	MID_SUMMONERS_RIFT = 1
};

enum LvMeshCellFlags : unsigned short {
	MCF_GRASS = 1,
	MCF_IMPASSABLE = 2,
	MCF_SEETHROUGH = 64
};

constexpr int MaxSpellLevel = 5;

enum SpellSlot : unsigned char
{
	SpellQ = 0,
	SpellW = 1,
	SpellE = 2,
	SpellR = 3,
	SummonerSpell1 = 4,
	SummonerSpell2 = 5,

	Recall = 10,

	ExtraSpell1 = 42,
	ExtraSpell2 = 43,
	ExtraSpell3 = 44,
	ExtraSpell4 = 45,
	ExtraSpell5 = 46,

	MaxSpellSlotId = 64
};

// isAvatarSpellbook can be nullptr if value is not needed
inline unsigned char SpellSlotToGameValue(SpellSlot slotId, bool* isAvatarSpellbook)
{
	if (isAvatarSpellbook)
		*isAvatarSpellbook = slotId == SummonerSpell1 || slotId == SummonerSpell2;

	if ((slotId >= SpellQ && slotId <= SpellR) || slotId == Recall)
		return slotId;

	if (slotId == SummonerSpell1 || slotId == SummonerSpell2)
		return slotId - SummonerSpell1;

	if (slotId >= MaxSpellSlotId) // basic attacks
		return slotId;

	return 0;
}

NLOHMANN_JSON_SERIALIZE_ENUM(LvMapId,
{
	{ MID_UNKNOWN, "UNKNOWN" }, { MID_SUMMONERS_RIFT, "SUMMONERS_RIFT" },
});
