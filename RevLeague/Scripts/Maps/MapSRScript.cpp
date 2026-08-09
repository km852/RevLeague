#include "MapSRScript.h"

void MapSRScript::Initialize()
{
	// Spawn positions acquired from "Data/Characters/HeroSpawnOffsets.inibin"

	Vector3 orderCenter(25.90, 211.91, 263.55);
	this->orderSpawnPositions[0] = orderCenter + Vector3(166.4f, 0.f, 120.f);
	this->orderSpawnPositions[1] = orderCenter + Vector3(166.4f, 0.f, -100.f);
	this->orderSpawnPositions[2] = orderCenter + Vector3(-42.8f, 0.f, -168.f);
	this->orderSpawnPositions[3] = orderCenter + Vector3(-172.2f, 0.f, 10.f);
	this->orderSpawnPositions[4] = orderCenter + Vector3(-82.8f, 0.f, 188.f);
	this->orderSpawnRotations[0] = NvLib::NvUtils::DegreesToRadians(144.f);
	this->orderSpawnRotations[1] = NvLib::NvUtils::DegreesToRadians(-144.f);
	this->orderSpawnRotations[2] = NvLib::NvUtils::DegreesToRadians(-72.f);
	this->orderSpawnRotations[3] = NvLib::NvUtils::DegreesToRadians(0.f);
	this->orderSpawnRotations[4] = NvLib::NvUtils::DegreesToRadians(72.f);

	Vector3 chaosCenter(13933.79f, 195.16f, 14170.09f);
	this->chaosSpawnPositions[0] = chaosCenter + Vector3(146.4f, 0.f, 120.f);
	this->chaosSpawnPositions[1] = chaosCenter + Vector3(146.4f, 0.f, -100.f);
	this->chaosSpawnPositions[2] = chaosCenter + Vector3(-62.8f, 0.f, -168.f);
	this->chaosSpawnPositions[3] = chaosCenter + Vector3(-192.2f, 0.f, 10.f);
	this->chaosSpawnPositions[4] = chaosCenter + Vector3(-102.8f, 0.f, 188.f);
	this->chaosSpawnRotations[0] = NvLib::NvUtils::DegreesToRadians(144.f);
	this->chaosSpawnRotations[1] = NvLib::NvUtils::DegreesToRadians(-144.f);
	this->chaosSpawnRotations[2] = NvLib::NvUtils::DegreesToRadians(-72.f);
	this->chaosSpawnRotations[3] = NvLib::NvUtils::DegreesToRadians(0.f);
	this->chaosSpawnRotations[4] = NvLib::NvUtils::DegreesToRadians(72.f);
}
