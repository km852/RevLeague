#pragma once

#include "../../NvLib.h"
#include "../../LvTypes.h"

class IMapScript : public NvNonCopyable {
protected:
	Vector3 orderSpawnPositions[5];
	Vector3 chaosSpawnPositions[5];
	float orderSpawnRotations[5];
	float chaosSpawnRotations[5];

public:
	virtual ~IMapScript() {}

	virtual void Initialize() {}
	virtual void Update() {}

	Vector3 GetSpawnPosition(LvTeam team, int teamIndex);
	Vector3 GetSpawnRotation(LvTeam team, int teamIndex);

	virtual LvMapId GetId() = 0;
};
