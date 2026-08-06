#pragma once

#include <fstream>

#include "../LvTypes.h"
#include "../NvLib.h"

class CharData final : public NvNonCopyable {
private:
	std::string internalName; // internal name for server use (NOT the same as in client game files)
	std::string inGameName; // character name exactly as in game files
	std::string inheritsFrom; // valid only during initialization; cleared as soon as data is copied from parent

	float attackDamage = 0.0f;
	float attackDamagePerLevel = 0.0f;

	float abilityPower = 0.0f;
	float abilityPowerPerLevel = 0.0f;

	float health = 0.0f;
	float healthPerLevel = 0.0f;

	float resource = 0.0f;
	float resourcePerLevel = 0.0f;

	float healthRegen = 0.0f;
	float healthRegenPerLevel = 0.0f;

	float resourceRegen = 0.0f;
	float resourceRegenPerLevel = 0.0f;

	float armor = 0.0f;
	float armorPerLevel = 0.0f;

	float magicResistance = 0.0f;
	float magicResistancePerLevel = 0.0f;

	float attackRange = 0.0f;
	float movementSpeed = 0.0f;
	float attackDelayOffset = 0.0f;
	float attackCastDelayOffset = 0.0f;

	float gameplayCollisionRadius = 0.0f;
	float selectionRadius = 0.0f;
	float pathfindingCollisionRadius = 0.0f;
	float visionRadius = 0.0f;

	CreateCharacterScript_t charScriptConstructor = nullptr;
	nlohmann::json initialJson;

public:
	const std::string& GetInternalName() { return internalName; }
	const std::string& GetInGameName() { return inGameName; }

	float GetBaseAttackDamage() const { return attackDamage; }
	float GetBaseAttackDamagePerLevel() const { return attackDamagePerLevel; }

	float GetBaseAbilityPower() const { return abilityPower; }
	float GetBaseAbilityPowerPerLevel() const { return abilityPowerPerLevel; }

	float GetBaseHealth() const { return health; }
	float GetBaseResource() const { return resource; }

	float GetBaseRange() const { return attackRange; }
	float GetMovementSpeed() const { return movementSpeed; }
	float GetAttackDelayOffset() const { return attackDelayOffset; }
	float GetAttackCastDelayOffset() const { return attackCastDelayOffset; }

	float GetGameplayCollisionRadius() const { return gameplayCollisionRadius; }
	float GetSelectionRadius() const { return selectionRadius; }
	float GetPathfindingCollisionRadius() const { return pathfindingCollisionRadius; }
	float GetVisionRadius() const { return visionRadius; }

	CreateCharacterScript_t GetScriptConstructor() const { return charScriptConstructor; }

	void InitFromJsonObject(const nlohmann::json& obj, CharData* parent);

	static CharData* GetCharData(const std::string& internalName);
	static void InitializeDatabaseJson(const nlohmann::json& obj);
};
