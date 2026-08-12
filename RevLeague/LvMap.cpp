#include "LvMap.h"
#include "LvGame.h"
#include "LvProtocol.h"
#include "LvObjectBase.h"
#include "LvObjectHero.h"
#include "LvObjectIterator.h"

#include "Scripts/Maps/MapSRScript.h"

LvMap::LvMap(LvMapId mapId, const std::string& navGridFilePath)
{
	this->objects.reserve(1000);
	this->blueObjects.reserve(1000);
	this->redObjects.reserve(1000);
	this->neutralObjects.reserve(1000);
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

	this->objects.push_back(obj.release()); // kinda ugly but whatever, maybe we'll switch to shared_ptr someday (objects & blueObjects & redObjects etc. must have the same type so LvObjectIterator's views work properly)

	if (objRaw->GetTeam() == TT_BLUE)
		this->blueObjects.push_back(objRaw);
	else if (objRaw->GetTeam() == TT_RED)
		this->redObjects.push_back(objRaw);
	else if (objRaw->GetTeam() == TT_NEUTRAL)
		this->neutralObjects.push_back(objRaw);
}

void LvMap::UpdateVisionGeneratedByObject(LvObjectBase* unit)
{
	if (unit->GetVisionRadius() <= 0.f)
		return;

	//LvPlayer* player = unit->GetType() == OBJ_HERO ? ((LvObjectHero*)unit)->GetPlayer() : nullptr;
	//LvClient* client = player ? player->GetClient() : nullptr;
	//if (client->GetState() != CST_IN_GAME)
	//	client = nullptr;

	for (LvObjectBase* target : LvObjectIterator().NotInTeam(unit->GetTeam()).WithinRange(unit->GetPosition(), unit->GetVisionRadius()).Iterate())
	{
		if (unit->LineOfSightTest(target))
		{
			target->SetTeamVisionGranted(unit->GetTeam(), true);

			//if (client)
			//	client->VisionSetDirectlySeen(target->GetNetworkId(), unit->GetDistance(target));
		}
	}
}

void LvMap::UpdateVision()
{
	for (LvObjectBase* obj : LvObjectIterator().IterateAll())
	{
		obj->SetTeamVisionGranted(TT_BLUE, false);
		obj->SetTeamVisionGranted(TT_RED, false);
		obj->SetTeamVisionGranted(obj->GetTeam(), true);
	}

	for (LvPlayer* player : lvGame->GetInGamePlayers())
		player->GetClient()->PreVisionUpdate();

	for (LvObjectBase* obj : LvObjectIterator().IterateAll())
		this->UpdateVisionGeneratedByObject(obj);

	for (LvPlayer* player : lvGame->GetInGamePlayers())
		player->GetClient()->PostVisionUpdate();
}

void LvMap::Update(double dt)
{
	this->script->Update();

	for (const auto& obj : this->objects)
		obj->Update(dt);

	this->UpdateVision();
}

LvMapId LvMap::GetId()
{
	return this->script->GetId();
}
