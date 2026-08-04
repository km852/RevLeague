#include "NvLib.h"

int LvServerStart(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	NvLib::InitializeMain();
	
	try
	{
		int retval = LvServerStart(argc, argv);
		if (retval != 0)
			LogWarning("Exiting with error code {}", retval);
		else
			LogInfo("Exiting with no error");
	}
	catch (const std::exception& ex)
	{
		LogError("Main thread exception: {}", ex.what());
		return 1;
	}

	return 0;
}
