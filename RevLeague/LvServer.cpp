#include "LvNetwork.h"
#include "LvGameSettings.h"
#include "LvGame.h"
#include "LvGameTimer.h"
#include "LvTypes.h"
#include "AutoVer.h"
#include "Dependencies/json.hpp"

#include <fstream>

using json = nlohmann::json;

static void PrintLogHeader()
{
	std::time_t currentTimestamp = std::time(nullptr);
	std::tm* timeFields = std::localtime(&currentTimestamp);

	LogInfo("RevLeague build " AUTOVER_VERSION ": {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}, PID {}", timeFields->tm_year + 1900, timeFields->tm_mon + 1, timeFields->tm_mday,
		timeFields->tm_hour, timeFields->tm_min, timeFields->tm_sec, GetCurrentProcessId());
}

static json LoadFileAsJson(const std::string& filePath)
{
	std::ifstream f(filePath, std::ios::binary);
	if (!f)
	{
		LogError("Could not open file: {}", filePath);
		return json::value_t::discarded;
	}

	auto size = std::filesystem::file_size(filePath);
	LogAssert(size < 1024 * 1024 * 256);

	std::string result;
	result.resize(size);
	if (!f.read(&result[0], size))
	{
		LogError("Could not open file: {}", filePath);
		return json::value_t::discarded;
	}

	try
	{
		return json::parse(result);
	}
	catch (const json::parse_error& e)
	{
		LogError("Could not parse file as JSON: {}", filePath);
		LogError("{}", e.what());
	}

	return json::value_t::discarded;
}

static bool InitializeAssets(const std::string& assetFilePath)
{
	LogDebug("Loading assets from {}", assetFilePath);
	json j = LoadFileAsJson(assetFilePath);
	if (j.is_discarded())
		return false;

	//CharData::InitializeDatabaseJson(assetFile["Characters"]);
	//SpellData::InitializeDatabaseJson(assetFile["Spells"]);
	//EffectData::InitializeDatabaseJson(assetFile["Effects"]);
	//BuffData::InitializeDatabaseJson(assetFile["Buffs"]);
	//ItemData::InitializeDatabaseJson(assetFile["Items"]);

	return true;
}

int LvServerStart(int argc, char* argv[])
{
	PrintLogHeader();

	json configJson = LoadFileAsJson("F:\\54\\default_config.json");
	if (configJson.is_discarded())
		return 1;

	std::unique_ptr<LvGameSettings> gameSettings;
	try
	{
		gameSettings = std::make_unique<LvGameSettings>(configJson);
	}
	catch (const std::exception& ex)
	{
		LogError("Could not parse server configuration file: {}", ex.what());
		return 1;
	}
	
	if (!InitializeAssets(gameSettings->assetFilePath))
		return 1;
	
	lvClock = new LvGameClock();

	lvNetwork = new LvNetwork(gameSettings->listenHost, gameSettings->listenPort);
	lvNetwork->StartListen();

	lvGame = new LvGame(gameSettings.get());
	lvGame->GameLoop();

	return 0;
}
