#pragma once

#include "IMapScript.h"

class MapSRScript final : public IMapScript {
public:
	virtual void Initialize() override;

	LvMapId GetId() override { return MID_SUMMONERS_RIFT; }
};
