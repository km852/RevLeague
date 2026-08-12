#pragma once

#include "NvLib.h"
#include "LvTypes.h"

class LvMap final : public NvNonCopyable {
	friend LvObjectIterator;

	std::unique_ptr<IMapScript> script;

	std::vector<LvObjectBase*> objects; // this is the owning container (it's not unique_ptr or shared_ptr it eases the implementation of LvObjectIterator)
	std::vector<LvObjectBase*> blueObjects;
	std::vector<LvObjectBase*> redObjects;
	std::vector<LvObjectBase*> neutralObjects;

	std::unordered_map<NetworkId, LvObjectBase*> objectsLookup;

	void UpdateVisionGeneratedByObject(LvObjectBase* unit);
	void UpdateVision();

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
