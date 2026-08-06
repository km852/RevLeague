#pragma once

#include "NvLib.h"
#include "LvTypes.h"

class LvMap : public NvNonCopyable {
	std::unique_ptr<IMapScript> script;

public:
	LvMap(LvMapId mapId, const std::string& navGridFilePath);

	void InitializeDefaultUnits();
	void InitializePlayerHeroes();
	void SendInitialUnitState(LvClient* client);

	void AddObject(LvObjectBase* obj);

	LvMapId GetId();
};

inline LvMap* lvMap;
