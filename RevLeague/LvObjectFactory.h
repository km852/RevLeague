#pragma once

#include "LvTypes.h"
#include "LvMap.h"

#include <optional>

class LvObjectFactory final : public NvNonCopyable {
	friend LvObjectBase;
	friend LvObjectHero;

	std::optional<Vector3> position;
	std::optional<Vector3> rotation;
	std::optional<NetworkId> networkId;
	std::optional<LvTeam> team;
	std::optional<LvPlayer*> player;
	std::optional<std::string> objName;
	std::optional<std::string> charDataPreset;
	std::optional<std::string> spellbookPreset;
	std::optional<float> visionRadius;

public:
	LvObjectFactory& Position(const Vector3& pos) { this->position = pos; return *this; }
	LvObjectFactory& Rotation(const Vector3& rot) { this->rotation = rot; return *this; }
	LvObjectFactory& ForcedNetworkId(NetworkId netId) { this->networkId = netId; return *this; }
	LvObjectFactory& Team(LvTeam team_) { this->team = team_; return *this; }
	LvObjectFactory& Name(const std::string& objectName) { this->objName = objectName; return *this; }
	LvObjectFactory& Player(LvPlayer* player_) { this->player = player_; return *this; }
	LvObjectFactory& CharData(const std::string& presetName) { this->charDataPreset = presetName; return *this; }
	LvObjectFactory& VisionRadius(float radius) { this->visionRadius = radius; return *this; }
	LvObjectFactory& Spellbook(const std::string& presetName) { this->spellbookPreset = presetName; return *this; }

	// sets CharData and Spellbook at the same time
	LvObjectFactory& ModelName(const std::string& presetName) { CharData(presetName); Spellbook(presetName); return *this; }

	template <typename T>
	T* CreateAndAdd()
	{
		T* obj = new T(*this);
		obj->PostCreateInit();
		lvMap->AddObject(std::unique_ptr<T>(obj)); // can't use make_unique here due to constructor visibility reasons

		return obj;
	}
};
