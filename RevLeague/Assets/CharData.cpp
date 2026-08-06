#include "CharData.h"

using json = nlohmann::json;

static std::unordered_map<std::string, CharData*> charDataInsts;

static std::unordered_map<std::string, CreateCharacterScript_t> scriptConstructors = {
	
};

void CharData::InitFromJsonObject(const nlohmann::json& obj, CharData* parent)
{
	this->initialJson = obj;

	this->internalName = obj.at("Name").get<std::string>();
	this->inGameName = obj.value<std::string>("InGameName", "UNSPECIFIED_IN_GAME_NAME_" + this->internalName);
	this->inheritsFrom = obj.value<std::string>("InheritsFrom", "");
	this->charScriptConstructor = scriptConstructors[this->internalName];

	this->attackDamage = obj.value<float>("AttackDamage", parent ? parent->attackDamage : 0.f);
	this->attackDamagePerLevel = obj.value<float>("AttackDamagePerLevel", parent ? parent->attackDamagePerLevel : 0.f);
	this->abilityPower = obj.value<float>("AbilityPower", parent ? parent->abilityPower : 0.f);
	this->abilityPowerPerLevel = obj.value<float>("AbilityPowerPerLevel", parent ? parent->abilityPowerPerLevel : 0.f);
	this->health = obj.value<float>("Health", parent ? parent->health : 0.f);
	this->healthPerLevel = obj.value<float>("HealthPerLevel", parent ? parent->healthPerLevel : 0.f);
	this->resource = obj.value<float>("Resource", parent ? parent->resource : 0.f);
	this->resourcePerLevel = obj.value<float>("ResourcePerLevel", parent ? parent->resourcePerLevel : 0.f);
	this->healthRegen = obj.value<float>("HealthRegen", parent ? parent->healthRegen : 0.f);
	this->healthRegenPerLevel = obj.value<float>("HealthRegenPerLevel", parent ? parent->healthRegenPerLevel : 0.f);
	this->resourceRegen = obj.value<float>("ResourceRegen", parent ? parent->resourceRegen : 0.f);
	this->resourceRegenPerLevel = obj.value<float>("ResourceRegenPerLevel", parent ? parent->resourceRegenPerLevel : 0.f);
	this->armor = obj.value<float>("Armor", parent ? parent->armor : 0.f);
	this->armorPerLevel = obj.value<float>("ArmorPerLevel", parent ? parent->armorPerLevel : 0.f);
	this->magicResistance = obj.value<float>("MagicResist", parent ? parent->magicResistance : 0.f);
	this->magicResistancePerLevel = obj.value<float>("MagicResistPerLevel", parent ? parent->magicResistancePerLevel : 0.f);
	this->attackRange = obj.value<float>("AttackRange", parent ? parent->attackRange : 0.f);
	this->movementSpeed = obj.value<float>("MovementSpeed", parent ? parent->movementSpeed : 0.f);
	this->attackDelayOffset = obj.value<float>("AttackDelayOffset", parent ? parent->attackDelayOffset : 0.f);
	this->attackCastDelayOffset = obj.value<float>("AttackCastDelayOffset", parent ? parent->attackCastDelayOffset : 0.f);
	this->gameplayCollisionRadius = obj.value<float>("GameplayCollisionRadius", parent ? parent->gameplayCollisionRadius : 0.f);
	this->selectionRadius = obj.value<float>("SelectionRadius", parent ? parent->selectionRadius : 0.f);
	this->pathfindingCollisionRadius = obj.value<float>("PathfindingCollisionRadius", parent ? parent->pathfindingCollisionRadius : 0.f);
	this->visionRadius = obj.value<float>("VisionRadius", parent ? parent->visionRadius : 0.f);
}

CharData* CharData::GetCharData(const std::string& internalName)
{
	return charDataInsts[internalName];
}

static void LoadSingleCharacter(const json& obj)
{
	std::unique_ptr<CharData> c = std::make_unique<CharData>();
	c->InitFromJsonObject(obj, nullptr);

	std::string name = c->GetInternalName();

	LogAssert(charDataInsts.find(name) == charDataInsts.end());
	charDataInsts[name] = c.release();
}

void CharData::InitializeDatabaseJson(const nlohmann::json& obj)
{
	for (auto&& entry : obj)
		LoadSingleCharacter(entry);

	int itersLeft = 5000;
	for (;;)
	{
		if (--itersLeft < 0)
		{
			LogError("Breaking out of infinite character inheritance loop!!");
			LogError("Characters with unresolved inheritance data:");
			for (const auto& c : charDataInsts)
			{
				if (!c.second->inheritsFrom.empty())
					LogError(" > {}", c.second->internalName);
			}

			throw NvSystemException("CharDB initialization failed: perhaps caused by circular inheritance?");
		}

		bool wasSomethingUpdated = false;

		for (auto& c : charDataInsts)
		{
			if (c.second->inheritsFrom.empty())
				continue;

			wasSomethingUpdated = true;

			CharData* parentType = charDataInsts[c.second->inheritsFrom];
			if (parentType == nullptr)
			{
				LogError("Character \"{}\" references non-existent parent type \"{}\"", c.second->internalName, c.second->inheritsFrom);
				throw NvSystemException("CharDB initialization failed: missing parent type");
			}

			if (!parentType->inheritsFrom.empty()) // the parent type itself inherits from something else; we wait to resolve parent's fields first
				continue;

			c.second->InitFromJsonObject(c.second->initialJson, parentType);
			c.second->inheritsFrom.clear();
		}

		if (!wasSomethingUpdated)
			break;
	}

	for (auto& c : charDataInsts)
		c.second->initialJson.clear();

	LogInfo("CharDB initialized with {} entries", charDataInsts.size());
}
