#pragma once

#include "IMapScript.h"

class MapSRScript : public IMapScript {
public:
	LvMapId GetId() override { return MID_SUMMONERS_RIFT; }
};
