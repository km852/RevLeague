#pragma once

#include "NvLib.h"
#include "Dependencies/enet/enet.h"

#include <queue>

class LvNetwork : public NvNonCopyable {
	NvCriticalSection lock;
	ENetHost* host;

	NvAutoWin32Handle hNetworkThread;
	NvAutoWin32Handle hListenStartEvent;

	std::queue<ENetEvent> eventQueue;

	void ListenInternal();
	static DWORD WINAPI ListenThread(LPVOID lpParameter);
	
public:
	LvNetwork(uint32_t listenAddress, uint16_t listenPort);
	void StartListen();

	bool PopEvent(ENetEvent* outEvent);

	NvAutoCriticalSection AcquireLock() { return lock.Acquire(); }
};

inline LvNetwork* lvNetwork;
