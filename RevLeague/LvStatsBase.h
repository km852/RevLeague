#pragma once

#include <memory>

#include "LvTypes.h"
#include "NvLib.h"
#include "LvStatReplicator.h"

class StatInstance {
	friend class LvStatsBase;

private:
	float baseValue = 0.0f;
	float bonusValue = 0.0f;

	inline void Set(float baseVal, float bonusVal)
	{
		baseValue = baseVal;
		bonusValue = bonusVal;
	}

public:
	inline StatInstance() {}

	inline float GetBaseValue() const { return baseValue; }
	inline float GetBonusValue() const { return bonusValue; }
	inline float GetTotalValue() const { return baseValue + bonusValue; }

	bool operator==(const StatInstance& rhs) const { return baseValue == rhs.baseValue && bonusValue == rhs.bonusValue; }
	bool operator!=(const StatInstance& rhs) const { return !(*this == rhs); }
};

class LvStatsBase : public NvNonCopyable {
	LvObjectBase* associatedObject;
	LvStatReplicator* statReplicator;

	int gold = 0;

	float health = 0.0f;
	StatInstance maxHealth;

	float resource = 0.0f;
	StatInstance maxResource;

	StatInstance movementSpeed;

	StatInstance attackDelayOffset;
	StatInstance attackCastDelayOffset;
	StatInstance attackRange;

	StatInstance attackDamage;
	StatInstance abilityPower;

	virtual void NotifyGold() {}
	virtual void NotifyCanCastBits() {}
	virtual void NotifySetHealth() {}
	virtual void NotifySetMaxHealth() {}
	virtual void NotifySetResource() {}
	virtual void NotifySetMaxResource() {}
	virtual void NotifySetMovementSpeed() {}
	virtual void NotifySetAttackDelayOffset() {}
	virtual void NotifySetAttackCastDelayOffset() {}
	virtual void NotifySetRange() {}
	virtual void NotifySetAttackDamage() {}
	virtual void NotifySetAbilityPower() {}

protected:
	// lower 32 bits form CanCastBits1, upper 32 bits form CanCastBits2
	unsigned long long canCastBits = 0;

public:
	LvStatsBase(LvObjectBase* owner);
	virtual ~LvStatsBase() {}

	inline LvObjectBase* GetAssociatedObject() { return associatedObject; }
	inline LvStatReplicator* GetReplicationManager() { return statReplicator; }
	NvBinaryStreamWrite ReplicateForPlayer(LvPlayer* player);

	// Clones the current stats into a new instance. The new instance does not have a unit or a replicator attached. The clone can be used to adjust stats
	// before calling ApplyUpdatedStats in order to commit them all at once into the original stats object.
	std::unique_ptr<LvStatsBase> CloneWithoutReplicator();

	// Copies all differing stats from the given stats set to the current object.
	void ApplyUpdatedStats(LvStatsBase* newStats);

	// Causes all the NotifyXxx functions to be called. In other words, forces all current stats to be replicated to all players.
	void MarkAllForReplication();

	bool CanCastSpell(SpellSlot slot);
	void SetCanCastSpell(SpellSlot slot, bool canCast);

	inline int GetGold() const { return gold; }
	inline void SetGold(int newGold)
	{
		gold = newGold;
		if (statReplicator) NotifyGold();
	}

	inline void SetRawCanCastBits(unsigned long long newCanCastBits)
	{
		canCastBits = newCanCastBits;
		if (statReplicator) NotifyCanCastBits();
	}

	inline float GetHealth() const { return health; }
	inline void SetHealth(float value)
	{
		health = std::min(std::max(value, 0.0f), maxHealth.GetTotalValue());
		if (statReplicator) NotifySetHealth();
	}

	inline const StatInstance& GetMaxHealth() const { return maxHealth; }
	inline void SetMaxHealth(float baseValue, float bonusValue)
	{
		maxHealth.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetMaxHealth();
	}

	inline float GetResource() const { return resource; }
	inline void SetResource(float value)
	{
		resource = std::min(std::max(value, 0.0f), maxResource.GetTotalValue());
		if (statReplicator) NotifySetResource();
	}

	inline const StatInstance& GetMaxResource() const { return maxResource; }
	inline void SetMaxResource(float baseValue, float bonusValue)
	{
		maxResource.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetMaxResource();
	}

	inline const StatInstance& GetMovementSpeed() const { return movementSpeed; }
	inline void SetMovementSpeed(float baseValue, float bonusValue)
	{
		movementSpeed.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetMovementSpeed();
	}

	inline const StatInstance& GetAttackDelayOffset() const { return attackDelayOffset; }
	inline void SetAttackDelayOffset(float baseValue, float bonusValue)
	{
		attackDelayOffset.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetAttackDelayOffset();
	}

	inline const StatInstance& GetAttackCastDelayOffset() const { return attackCastDelayOffset; }
	inline void SetAttackCastDelayOffset(float baseValue, float bonusValue)
	{
		attackCastDelayOffset.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetAttackCastDelayOffset();
	}

	inline const StatInstance& GetAttackDamage() const { return attackDamage; }
	inline void SetAttackDamage(float baseValue, float bonusValue)
	{
		attackDamage.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetAttackDamage();
	}

	inline const StatInstance& GetAbilityPower() const { return abilityPower; }
	inline void SetAbilityPower(float baseValue, float bonusValue)
	{
		abilityPower.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetAbilityPower();
	}

	inline const StatInstance& GetRange() const { return attackRange; }
	inline void SetRange(float baseValue, float bonusValue)
	{
		attackRange.Set(baseValue, bonusValue);
		if (statReplicator) NotifySetRange();
	}

	float GetAttackSpeedMultiplier() { return (1.0f + this->GetAttackDelayOffset().GetBaseValue()) / (1.0f + this->GetAttackDelayOffset().GetTotalValue()); }
	float GetAttackDelayFloat() { return 1.6f * (1.0f + this->GetAttackDelayOffset().GetTotalValue()); }
	float GetWindupTimeFraction() { return 0.3f + this->GetAttackCastDelayOffset().GetTotalValue(); }
};
