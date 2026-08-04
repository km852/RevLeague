#include "LvNetwork.h"

DWORD WINAPI LvNetwork::ListenThread(LPVOID lpParameter)
{
	LvNetwork* thisPtr = (LvNetwork*)lpParameter;
	LogAssert(WaitForSingleObject(thisPtr->hListenStartEvent.Get(), INFINITE) == WAIT_OBJECT_0);

	thisPtr->ListenInternal();
	return 0;
}

void LvNetwork::ListenInternal()
{
	for (;;)
	{
		ENetEvent event;
		int result = 0;

		for (;;)
		{
			auto lock = this->AcquireLock();

			result = enet_host_service(this->host, &event, 0);
			if (result <= 0)
			{
				lock.Release();
				Sleep(1);

				break;
			}

			this->eventQueue.push(event);
		}

		if (result < 0)
		{
			LogError("enet_host_service failed: {}", result);
		}
	}
}

LvNetwork::LvNetwork(uint32_t listenAddress, uint16_t listenPort)
{
	LogAssert(enet_initialize() == 0);
	atexit(enet_deinitialize);

	ENetAddress addr = { .host = htonl(listenAddress), .port = listenPort };
	this->host = enet_host_create(&addr, 32, 0, 0);
	if (!LogAssert(this->host != nullptr))
		return;

	LogAssert((this->hListenStartEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr)).IsValidHandle());
	LogAssert((this->hNetworkThread = CreateThread(nullptr, 0, ListenThread, this, 0, nullptr)).IsValidHandle());

	LogDebug("ENetHost created on {}.{}.{}.{}:{}", (uint8_t)((listenAddress >> 24) & 0xFF),
		(uint8_t)((listenAddress >> 16) & 0xFF), (uint8_t)((listenAddress >> 8) & 0xFF), (uint8_t)(listenAddress & 0xFF), listenPort);
}

bool LvNetwork::PopEvent(ENetEvent* outEvent)
{
	if (this->eventQueue.empty())
		return false;

	std::memcpy(outEvent, &this->eventQueue.front(), sizeof(ENetEvent));
	this->eventQueue.pop();

	return true;
}

void LvNetwork::StartListen()
{
	LogAssert(SetEvent(this->hListenStartEvent.Get()) != 0);
}
