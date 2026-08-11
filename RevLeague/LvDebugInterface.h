#pragma once

#include "NvLib.h"
#include "LvTypes.h"

enum LvDebugStructType : unsigned char {
	DBG_TYPE_OBJECT_INFO = 0
};

template<> inline void NvBinaryStreamWrite::Write(const LvDebugStructType& val) { Write<unsigned char>(val); }

class LvDebugInterface final : public NvNonCopyable {
private:
	NvCriticalSection dataSyncCS;

	NvAutoWin32Handle currentPipeHandlerThread = nullptr;
	NvAutoWin32Handle commandProcessThread = nullptr;

	std::vector<unsigned char> pendingData;

	bool initMessageShown = false;
	bool isConnected = false;

	void PushData(const std::vector<unsigned char>& data);

	static inline DWORD CALLBACK PipeHandler(LPVOID lpParameter);
	static inline DWORD CALLBACK AcceptPipeConnections(LPVOID lpParameter);

public:
	void OnObjectUpdate(LvObjectBase* object);

	LvDebugInterface();
};

inline LvDebugInterface* lvDebug;
