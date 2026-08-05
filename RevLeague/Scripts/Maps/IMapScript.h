#pragma once

#include "../../NvLib.h"
#include "../../LvTypes.h"

class IMapScript : public NvNonCopyable {
public:
	virtual ~IMapScript() {}

	virtual void Initialize() {}
	virtual void Update() {}

	virtual LvMapId GetId() = 0;
};
