#include "LvMap.h"
#include "LvGame.h"
#include "LvProtocol.h"
#include "LvObjectBase.h"

#include "Scripts/Maps/MapSRScript.h"

LvMap::LvMap(LvMapId mapId, const std::string& navGridFilePath)
{
	this->objects.reserve(1000);
	this->blueObjects.reserve(1000);
	this->redObjects.reserve(1000);
	this->objectsLookup.reserve(1000);

	switch (mapId)
	{
	case MID_SUMMONERS_RIFT:
		this->script = std::make_unique<MapSRScript>();
		break;
	default:
		throw NvSystemException(std::format("Undefined map ID: {}", (int)mapId));
	}
}

void LvMap::InitializeDefaultUnits()
{
	this->script->Initialize();
}

void LvMap::InitializePlayerHeroes()
{
	for (LvPlayer* player : lvGame->GetPlayers())
	{
		if (!player->GetHero())
			player->CreateHero();
	}
}

void LvMap::SendInitialUnitState(LvClient* client)
{
	for (LvPlayer* player : lvGame->GetPlayers())
	{
		client->SendPacket(PCH_ServerToClient, LvProtocol::CreateCreateHero(player));
		client->SendPacket(PCH_ServerToClient, LvProtocol::CreateAvatarInfo(player));
	}
}

void LvMap::AddObject(std::unique_ptr<LvObjectBase> obj)
{
	LvObjectBase* objRaw = obj.get();

	if (!this->objectsLookup.insert(std::make_pair(objRaw->GetNetworkId(), objRaw)).second)
	{
		LogError("Cannot add object {} (duplicate NetworkId)", objRaw);
		return;
	}

	this->objects.push_back(std::move(obj));

	if (objRaw->GetTeam() == TT_BLUE)
		this->blueObjects.push_back(objRaw);
	else if (objRaw->GetTeam() == TT_RED)
		this->redObjects.push_back(objRaw);
}

LvMapId LvMap::GetId()
{
	return this->script->GetId();
}
