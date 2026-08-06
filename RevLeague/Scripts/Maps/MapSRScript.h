#pragma once

#include "IMapScript.h"

class MapSRScript final : public IMapScript {
public:
	LvMapId GetId() override { return MID_SUMMONERS_RIFT; }
};
