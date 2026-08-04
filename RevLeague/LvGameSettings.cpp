#include "LvGameSettings.h"

#include "Dependencies/json.hpp"

using json = nlohmann::json;

// Outputs an integer in the host-order (little endian).
static unsigned int IPv4ToInt(const std::string& s)
{
    std::istringstream ss(s);
    unsigned int a = UINT_MAX, b = UINT_MAX, c = UINT_MAX, d = UINT_MAX;
    char dot1 = 0, dot2 = 0, dot3 = 0;

    ss >> a >> dot1 >> b >> dot2 >> c >> dot3 >> d;
    if (!ss.eof() || a > 255 || b > 255 || c > 255 || d > 255 || dot1 != '.' || dot2 != '.' || dot3 != '.')
    {
        LogWarning("Invalid value for \"ListenHost\" in server configuration file. Defaulting to localhost");
        return IPv4ToInt("127.0.0.1");
    }

    return d | (c << 8) | (b << 16) | (a << 24);
}

LvGameSettings::LvGameSettings(const json& obj)
{
    this->assetFilePath = obj.at("AssetFilePath").get<std::string>();
	this->listenHost = IPv4ToInt(obj.value<std::string>("ListenHost", ""));
    this->listenPort = obj.value<unsigned short>("ListenPort", 6266);
    this->gameId = obj.at("GlobalGameId").get<unsigned long long>();

    for (auto&& player : obj.at("Players"))
        this->players.emplace_back(player);
}

LvGameSettings::PlayerInfo::PlayerInfo(const nlohmann::json& obj)
{
    this->playerId = obj.at("Id").get<unsigned long long>();
    this->playerName = obj.at("Name").get<std::string>();

    std::string decodedKey = NvLib::NvUtils::Base64Decode(obj.at("NetEncryptionKey").get<std::string>());
    if (decodedKey.size() != 16)
        throw std::logic_error(std::format("Player {} ({}) must have a base64-encoded encryption key of 16 bytes (24 characters when base64-encoded)", this->playerName, this->playerId));

    std::memcpy(this->netEncryptionKey.data(), decodedKey.data(), this->netEncryptionKey.size());

    this->team = obj.at("Team").get<LvTeam>();
    if (this->team != TT_BLUE && this->team != TT_RED)
        throw std::logic_error(std::format("Player {} ({}) has invalid team value (must be TT_BLUE or TT_RED)", this->playerName, this->playerId));
}
