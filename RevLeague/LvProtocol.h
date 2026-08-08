#pragma once

#include "NvLib.h"
#include "LvTypes.h"

#define EXPECTED_GAME_VERSION_STRING "Version 1.0.0.146 [PUBLIC]"

namespace LvProtocol {
	enum PacketHeaders : unsigned char {
		PKT_C2S_QueryStatusReq = 0x14,
		PKT_S2C_QueryStatusAns = 0x89,
		PKT_World_SendGameNumber = 0x92,
		PKT_SynchVersionC2S = 0xBD,
		PKT_SynchVersionS2C = 0x54,
		PKT_C2S_CharSelected = 0x64,
		PKT_Loading_SetSkin = 0x65,
		PKT_Loading_SetName = 0x66,
		PKT_Loading_TeamRosterUpdate = 0x67,
		PKT_C2S_Ping_Load_Info = 0x16,
		PKT_S2C_Ping_Load_Info = 0x95,
		PKT_C2S_ClientReady = 0xBE,
		PKT_S2C_StartSpawn = 0x62,
		PKT_S2C_EndSpawn = 0x11,
		PKT_S2C_StartGame = 0x5C,
		PKT_S2C_CreateHero = 0x4C,
		PKT_AvatarInfo = 0x29,
	};

	NvBinaryStreamWrite CreatePeerRegistration(LvPlayer* player, unsigned long long encryptedKey);
	NvBinaryStreamWrite CreateSendGameNumber(LvPlayer* player);
	NvBinaryStreamWrite CreateQueryStatusAnswer();
	NvBinaryStreamWrite CreateSynchVersionAnswer();
	NvBinaryStreamWrite CreateLoadingTeamRosterUpdate();
	NvBinaryStreamWrite CreateLoadingSetSkin(LvPlayer* player);
	NvBinaryStreamWrite CreateLoadingSetName(LvPlayer* player);
	NvBinaryStreamWrite CreatePingLoadInfo(LvPlayer* player);
	NvBinaryStreamWrite CreateStartSpawn();
	NvBinaryStreamWrite CreateEndSpawn();
	NvBinaryStreamWrite CreateStartGame();
	NvBinaryStreamWrite CreateCreateHero(LvPlayer* player);
	NvBinaryStreamWrite CreateAvatarInfo(LvPlayer* player);
}

template<>
inline void NvBinaryStreamWrite::Write(LvProtocol::PacketHeaders v)
{
	this->Write<unsigned char>(v);
}

template<>
inline void NvBinaryStreamWrite::Write(LvMapId v)
{
	this->Write<int>(v);
}
