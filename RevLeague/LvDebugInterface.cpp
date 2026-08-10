#include "LvDebugInterface.h"

constexpr const char* DebugPipeName = "\\\\.\\pipe\\RevLeaguePipe";

struct PipeHandlerThreadArgs {
	LvDebugInterface* iface;
	HANDLE hPipe;
	HANDLE hContinueEvent;
};

void LvDebugInterface::PushData(const std::vector<unsigned char>& data)
{
	auto lock = this->dataSyncCS.Acquire();
	this->pendingData.insert(this->pendingData.end(), data.begin(), data.end());
}

inline DWORD CALLBACK LvDebugInterface::PipeHandler(LPVOID lpParameter)
{
	NvAutoWin32Handle pipe = ((PipeHandlerThreadArgs*)lpParameter)->hPipe;
	LvDebugInterface* iface = ((PipeHandlerThreadArgs*)lpParameter)->iface;

	SetEvent(((PipeHandlerThreadArgs*)lpParameter)->hContinueEvent); // lpParameter becomes invalid after this call

	DWORD writtenBytes = 0;

	for (;;)
	{
		auto lock = iface->dataSyncCS.Acquire();

		std::vector<unsigned char> dataToWrite = std::move(iface->pendingData);
		iface->pendingData.clear();

		lock.Release();

		if (!dataToWrite.empty())
		{
			if (!WriteFile(pipe.Get(), dataToWrite.data(), (DWORD)dataToWrite.size(), &writtenBytes, nullptr) || writtenBytes != dataToWrite.size())
			{
				if (GetLastError() == ERROR_NO_DATA || GetLastError() == ERROR_BROKEN_PIPE)
				{
					LogWarning("Debug pipe has been closed");
					return 0;
				}

				LogAssertLastError(!"WriteFile in debug pipe failed", GetLastError());
				return 1;
			}
		}

		Sleep(5);
	}

	return 0;
}

inline DWORD CALLBACK LvDebugInterface::AcceptPipeConnections(LPVOID lpParameter)
{
	LvDebugInterface* iface = (LvDebugInterface*)lpParameter;

	for (;;)
	{
		NvAutoWin32Handle pipe = CreateNamedPipeA(DebugPipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES, 8192, 512, 0, nullptr);

		if (pipe.Get() == INVALID_HANDLE_VALUE)
		{
			LogError("Cannot create named pipe (error {}), debug interface will be unavailable", GetLastError());
			return 1;
		}

		if (!iface->initMessageShown)
		{
			LogInfo("Debug interface initialized using pipe {}", DebugPipeName);
			iface->initMessageShown = true;
		}

		if (ConnectNamedPipe(pipe.Get(), nullptr) || GetLastError() == ERROR_PIPE_CONNECTED)
		{
			if (iface->currentPipeHandlerThread.Get() != nullptr)
			{
				if (WaitForSingleObject(iface->currentPipeHandlerThread.Get(), 0) == WAIT_OBJECT_0)
					iface->currentPipeHandlerThread = nullptr;
			}

			if (iface->currentPipeHandlerThread.Get() != nullptr)
			{
				LogWarning("Cannot accept debug pipe connection (already occupied)");
				continue;
			}

			// since PipeHandlerThreadArgs is stack-allocated, we need to wait for the child thread to consume the argument before we can release the event handle
			NvAutoWin32Handle continueEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);

			PipeHandlerThreadArgs threadArgs;
			threadArgs.hContinueEvent = continueEvent.Get();
			threadArgs.hPipe = pipe.Abandon();
			threadArgs.iface = iface;

			iface->currentPipeHandlerThread = CreateThread(nullptr, 0, PipeHandler, &threadArgs, 0, nullptr);
			LogAssertLastError(iface->currentPipeHandlerThread.Get() != nullptr, GetLastError());

			LogAssert(WaitForSingleObject(continueEvent.Get(), 5000) == WAIT_OBJECT_0);
		}
	}
}

LvDebugInterface::LvDebugInterface()
{
	this->commandProcessThread = CreateThread(nullptr, 0, AcceptPipeConnections, this, 0, nullptr);
}
