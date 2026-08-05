#pragma once

#include "NvLib.h"
#include "LvTypes.h"

class LvMap : public NvNonCopyable {
	std::unique_ptr<IMapScript> script;

public:
	LvMap(LvMapId mapId, const std::string& navGridFilePath);

	LvMapId GetId();
};

inline LvMap* lvMap;
