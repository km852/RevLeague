#include "LvGame.h"
#include "LvGameSettings.h"
#include "LvGameTimer.h"
#include "LvClient.h"
#include "LvPlayer.h"
#include "LvNetwork.h"

#include "Dependencies/enet/enet.h"

LvGame::LvGame(LvGameSettings* settings)
{
	this->settings = settings;

	LogDebug("================== Initializing game {} ==================", settings->gameId);

	for (auto&& playerInfo : settings->players)
		this->players.push_back(std::make_unique<LvPlayer>(&playerInfo));
}

LvPlayer* LvGame::GetPlayerByUserId(unsigned long long userId)
{
	for (auto& player : this->players)
		if (player->GetUserId() == userId)
			return player.get();

	return nullptr;
}

void LvGame::ProcessENetEvents()
{
	ENetEvent currentEvent;
	for (;;)
	{
		auto lock = lvNetwork->AcquireLock();

		if (!lvNetwork->PopEvent(&currentEvent)) // TODO: copy the event data into a local buffer and release lock immediately
			return;

		ENetPeer* peer = currentEvent.peer;

		if (currentEvent.type == ENetEventType::ENET_EVENT_TYPE_CONNECT)
		{
			LogDebug("Incoming connection from {}.{}.{}.{}:{}", (uint8_t)(peer->address.host & 0xFF), (uint8_t)((peer->address.host >> 8) & 0xFF),
				(uint8_t)((peer->address.host >> 16) & 0xFF), (uint8_t)((peer->address.host >> 24) & 0xFF), peer->address.port);

			LvClient* client = new LvClient(peer);
			peer->data = client;

			this->clients.push_back(client);
		}
		else if (currentEvent.type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT)
		{
			LvClient* client = (LvClient*)peer->data;
			peer->data = nullptr;

			if (LogAssert(client != nullptr))
			{
				LogDebug("Disconnection event received from {}", client->GetEndpointString());

				auto it = std::find(this->clients.begin(), this->clients.end(), client);
				if (LogAssert(it != this->clients.end()))
					this->clients.erase(it);
			}

			delete client;
		}
		else if (currentEvent.type == ENetEventType::ENET_EVENT_TYPE_RECEIVE)
		{
			ENetPacket* packet = currentEvent.packet;
			LvClient* client = (LvClient*)peer->data;

			lock.Release(); // release ENet lock for potentially lengthy packet processing
			LvPacketProcessingResult packetResult = client->ProcessPacket((LvPacketChannel)currentEvent.channelID, packet->data, packet->dataLength);
			lock = lvNetwork->AcquireLock();

			if (packetResult == PPR_FAIL_PRINT || packetResult == PPR_FAIL_DISCONNECT)
			{
				LogWarning("Packet originating from {} on channel {} failed to be processed!", client->GetEndpointString(), currentEvent.channelID);
				NvLogger::PrintHexBuffer(packet->data, packet->dataLength);
			}
			if (packetResult == PPR_FAIL_DISCONNECT)
			{
				enet_peer_disconnect(peer, 0);
			}

			enet_packet_destroy(packet);
		}
	}
}

void LvGame::SendQueuedPackets()
{
	if (this->pendingPackets.empty())
		return;

	auto lock = lvNetwork->AcquireLock();

	for (const auto& packetInfo : this->pendingPackets)
		enet_peer_send(packetInfo.targetClient->GetENetPeer(), (unsigned char)packetInfo.channelId, packetInfo.packet);

	lock.Release();
	this->pendingPackets.clear();
}

void LvGame::GameLoopInternal()
{
	this->processingNetEvents = true;
	this->ProcessENetEvents();
	this->processingNetEvents = false;
}

void LvGame::GameLoop()
{
	double frameTimeSum = 0;
	double maxFrameTime = 0.0;
	int frameTimeMeasurements = 0;

	LvGameTimer frameDebugTimer;

	LogInfo("Game loop started");

	for (;;)
	{
		lvClock->StartNewFrame();

		this->GameLoopInternal();
		double frameTime = lvClock->GetCurrentFrameTime();

		lvClock->EndFrame();

		if (frameTime > 40.0)
			LogWarning("A frame has taken unusually long to execute [{:.2f} ms]", frameTime);

		frameTimeSum += frameTime;
		maxFrameTime = std::max(maxFrameTime, frameTime);
		++frameTimeMeasurements;

		if (frameDebugTimer.Seconds() > 20.0f)
		{
			//LogDebug("Average frame time within last {:.2f} seconds: {:.3f}, max: {:.3f}", frameDebugTimer.Seconds(), frameTimeSum / frameTimeMeasurements, maxFrameTime);

			frameTimeSum = 0;
			maxFrameTime = 0;
			frameTimeMeasurements = 0;
			frameDebugTimer.Reset();
		}

		this->SendQueuedPackets();

		Sleep(1);
	}
}
