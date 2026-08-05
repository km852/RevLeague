#pragma once

#include "NvLib.h"
#include "LvTypes.h"
#include "LvClient.h"
#include "LvPlayer.h"
#include "LvGameSettings.h"

#include "Dependencies/enet/enet.h"

class LvGame final : public NvNonCopyable {
	struct PendingPacket {
		LvClient* targetClient;
		ENetPacket* packet;
		LvPacketChannel channelId;
	};

	const LvGameSettings* settings;

	NetworkId nextNetworkId = 0x40000001; // increment each time new NetId is assigned
	uint32_t nextPacketTick = 1; // many packets have a 32-bit "syncID" field - we generate syncID from this and increment each time it is used. TODO: make it per-LvClient?

	LvGameState state = GST_PRELOADING;

	bool processingNetEvents = false; // if SendPacket is called while processingNetEvents == true, raise a warning. Normally you shouldn't send packets directly in packet handlers

	std::vector<std::unique_ptr<LvPlayer>> players;
	std::vector<LvClient*> clients; // no unique_ptr here because ENetPeer::data owns the client, and it gets managed manually
	std::vector<PendingPacket> pendingPackets;

	void ProcessENetEvents();
	void SendQueuedPackets();
	void GameLoopInternal();

public:
	inline LvGameState GetState() { return this->state; }
	void SetState(LvGameState newState);

	inline unsigned long long GetGameId() { return settings->gameId; }

	void AddPacketToQueue(LvClient* targetClient, LvPacketChannel channelId, ENetPacket* packet);

	auto GetPlayers() const {
		return this->players | std::views::transform([](const std::unique_ptr<LvPlayer>& v) { return v.get(); });
	}

	auto GetConnectedPlayers() const {
		return this->players | std::views::transform([](const std::unique_ptr<LvPlayer>& v) { return v.get(); }) |
			std::views::filter([](LvPlayer* v) { return v->GetClient() != nullptr; });
	}

	auto GetInGamePlayers() const {
		return this->players | std::views::transform([](const std::unique_ptr<LvPlayer>& v) { return v.get(); }) |
			std::views::filter([](LvPlayer* v) { return v->GetClient() && v->GetClient()->GetState() == CST_IN_GAME; });
	}

	LvPlayer* GetPlayerByUserId(unsigned long long userId);

	LvGame(LvGameSettings* settings);

	void GameLoop();
};

inline LvGame* lvGame;
