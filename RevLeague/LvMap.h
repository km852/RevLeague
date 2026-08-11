#pragma once

#include "NvLib.h"
#include "LvTypes.h"

class LvMap final : public NvNonCopyable {
	std::unique_ptr<IMapScript> script;

	std::vector<std::unique_ptr<LvObjectBase>> objects;
	std::vector<LvObjectBase*> blueObjects;
	std::vector<LvObjectBase*> redObjects;

	std::unordered_map<NetworkId, LvObjectBase*> objectsLookup;

public:
	LvMap(LvMapId mapId, const std::string& navGridFilePath);

	void InitializeDefaultUnits();
	void InitializePlayerHeroes();
	void SendInitialUnitState(LvClient* client);

	void AddObject(std::unique_ptr<LvObjectBase> obj);

	void Update(double dt);

	LvMapId GetId();
};

inline LvMap* lvMap;
