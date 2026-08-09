#include "IMapScript.h"

Vector3 IMapScript::GetSpawnPosition(LvTeam team, int teamIndex)
{
    if (teamIndex < 0 || teamIndex >= std::size(this->orderSpawnPositions))
        return Vector3();

    if (team == TT_BLUE)
        return this->orderSpawnPositions[teamIndex];
    else if (team == TT_RED)
        return this->chaosSpawnPositions[teamIndex];

    return Vector3();
}

Vector3 IMapScript::GetSpawnRotation(LvTeam team, int teamIndex)
{
    if (teamIndex < 0 || teamIndex >= std::size(this->orderSpawnRotations))
        return Vector3(0.f, 0.f, 1.f);

    if (team == TT_BLUE)
        return Vector3(0.f, 0.f, 1.f).RotatedAroundY(this->orderSpawnRotations[teamIndex]);
    else if (team == TT_RED)
        return Vector3(0.f, 0.f, -1.f).RotatedAroundY(this->chaosSpawnRotations[teamIndex]);

    return Vector3(0.f, 0.f, 1.f);
}
