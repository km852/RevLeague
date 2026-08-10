#pragma once

#include "NvLib.h"
#include "LvTypes.h"

class LvDebugInterface final : public NvNonCopyable {
private:
	NvCriticalSection dataSyncCS;

	NvAutoWin32Handle currentPipeHandlerThread = nullptr;
	NvAutoWin32Handle commandProcessThread = nullptr;

	std::vector<unsigned char> pendingData;

	bool initMessageShown = false;

	void PushData(const std::vector<unsigned char>& data);

	static inline DWORD CALLBACK PipeHandler(LPVOID lpParameter);
	static inline DWORD CALLBACK AcceptPipeConnections(LPVOID lpParameter);

public:
	LvDebugInterface();
};

inline LvDebugInterface* lvDebug;
