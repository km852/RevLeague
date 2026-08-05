#include "LvMap.h"

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

LvMapId LvMap::GetId()
{
	return this->script->GetId();
}
