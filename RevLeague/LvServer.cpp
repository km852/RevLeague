#include "LvNetwork.h"
#include "LvGameSettings.h"
#include "LvGame.h"
#include "LvGameClock.h"
#include "LvTypes.h"
#include "LvDebugInterface.h"
#include "LvMap.h"
#include "LvMesh.h"
#include "AutoVer.h"
#include "Dependencies/json.hpp"

#include "Assets/CharData.h"

#include <fstream>

using json = nlohmann::json;

static void PrintLogHeader()
{
	std::time_t currentTimestamp = std::time(nullptr);
	std::tm* timeFields = std::localtime(&currentTimestamp);

	LogInfo("RevLeague build " AUTOVER_VERSION ": {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}, PID {}", timeFields->tm_year + 1900, timeFields->tm_mon + 1, timeFields->tm_mday,
		timeFields->tm_hour, timeFields->tm_min, timeFields->tm_sec, GetCurrentProcessId());
}

static std::optional<std::vector<unsigned char>> LoadFileRaw(const std::string& filePath)
{
	std::ifstream f(filePath, std::ios::binary);
	if (!f)
	{
		LogError("Could not open file: {}", filePath);
		return std::nullopt;
	}

	auto size = std::filesystem::file_size(filePath);
	LogAssert(size < 1024 * 1024 * 256);

	std::vector<unsigned char> result;
	result.resize(size);
	if (!f.read((char*)&result[0], size))
	{
		LogError("Could not open file: {}", filePath);
		return std::nullopt;
	}

	return result;
}

static json LoadFileAsJson(const std::string& filePath)
{
	auto fileDataRaw = LoadFileRaw(filePath);
	if (!fileDataRaw.has_value())
		return json::value_t::discarded;

	try
	{
		return json::parse((char*)fileDataRaw.value().data(), (char*)fileDataRaw.value().data() + fileDataRaw.value().size());
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

	CharData::InitializeDatabaseJson(j["Characters"]);
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

	lvDebug = new LvDebugInterface();

	auto meshFileData = LoadFileRaw(gameSettings->meshFilePath).value_or({});
	lvMesh = new LvMesh(meshFileData);

	lvMap = new LvMap(gameSettings->mapId, "");

	lvGame = new LvGame(gameSettings.get());
	lvGame->GameLoop();

	return 0;
}
