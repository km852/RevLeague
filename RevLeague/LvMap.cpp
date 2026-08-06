#include "LvMap.h"
#include "LvGame.h"
#include "LvProtocol.h"

#include "Scripts/Maps/MapSRScript.h"

LvMap::LvMap(LvMapId mapId, const std::string& navGridFilePath)
{
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
	}
}

void LvMap::AddObject(LvObjectBase* obj)
{

}

LvMapId LvMap::GetId()
{
	return this->script->GetId();
}
