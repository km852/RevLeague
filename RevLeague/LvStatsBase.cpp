#include "LvStatsBase.h"

LvStatsBase::LvStatsBase(LvObjectBase* owner)
{
	this->associatedObject = owner;
	this->statReplicator = this->associatedObject ? new LvStatReplicator(this) : nullptr;
}

NvBinaryStreamWrite LvStatsBase::ReplicateForPlayer(LvPlayer* player)
{
	LogAssert(this->statReplicator != nullptr);
	return this->statReplicator->ReplicateForPlayer(player);
}

std::unique_ptr<LvStatsBase> LvStatsBase::CloneWithoutReplicator()
{
	std::unique_ptr<LvStatsBase> newStats = std::make_unique<LvStatsBase>(nullptr);

	newStats->gold = this->gold;
	newStats->canCastBits = this->canCastBits;
	newStats->health = this->health;
	newStats->maxHealth = this->maxHealth;
	newStats->resource = this->resource;
	newStats->maxResource = this->maxResource;
	newStats->movementSpeed = this->movementSpeed;
	newStats->attackDelayOffset = this->attackDelayOffset;
	newStats->attackCastDelayOffset = this->attackCastDelayOffset;
	newStats->attackRange = this->attackRange;
	newStats->attackDamage = this->attackDamage;
	newStats->abilityPower = this->abilityPower;

	return newStats;
}

void LvStatsBase::ApplyUpdatedStats(LvStatsBase* newStats)
{
	if (this->gold != newStats->gold) this->SetGold(newStats->gold);
	if (this->maxHealth != newStats->maxHealth) this->SetMaxHealth(newStats->maxHealth.baseValue, newStats->maxHealth.bonusValue);
	if (this->maxResource != newStats->maxResource) this->SetMaxResource(newStats->maxResource.baseValue, newStats->maxResource.bonusValue);
	if (this->movementSpeed != newStats->movementSpeed) this->SetMovementSpeed(newStats->movementSpeed.baseValue, newStats->movementSpeed.bonusValue);
	if (this->attackDelayOffset != newStats->attackDelayOffset) this->SetAttackDelayOffset(newStats->attackDelayOffset.baseValue, newStats->attackDelayOffset.bonusValue);
	if (this->attackCastDelayOffset != newStats->attackCastDelayOffset) this->SetAttackCastDelayOffset(newStats->attackCastDelayOffset.baseValue, newStats->attackCastDelayOffset.bonusValue);
	if (this->attackRange != newStats->attackRange) this->SetRange(newStats->attackRange.baseValue, newStats->attackRange.bonusValue);
	if (this->attackDamage != newStats->attackDamage) this->SetAttackDamage(newStats->attackDamage.baseValue, newStats->attackDamage.bonusValue);
	if (this->abilityPower != newStats->abilityPower) this->SetAbilityPower(newStats->abilityPower.baseValue, newStats->abilityPower.bonusValue);

	if (this->canCastBits != newStats->canCastBits) this->SetRawCanCastBits(newStats->canCastBits);
	if (this->health != newStats->health) this->SetHealth(newStats->health);
	if (this->resource != newStats->resource) this->SetResource(newStats->resource);
}

void LvStatsBase::MarkAllForReplication()
{
	this->NotifyCanCastBits();
	this->NotifySetHealth();
	this->NotifySetMaxHealth();
	this->NotifySetResource();
	this->NotifySetMaxResource();
	this->NotifySetMovementSpeed();
	this->NotifySetAttackDelayOffset();
	this->NotifySetAttackCastDelayOffset();
	this->NotifySetRange();
	this->NotifySetAttackDamage();
	this->NotifySetAbilityPower();
}

bool LvStatsBase::CanCastSpell(SpellSlot slot)
{
	unsigned char slotId = SpellSlotToGameValue(slot, nullptr);
	return (this->canCastBits & (1ULL << slotId)) != 0;
}

void LvStatsBase::SetCanCastSpell(SpellSlot slot, bool canCast)
{
	unsigned char slotId = SpellSlotToGameValue(slot, nullptr);
	if (slot == SummonerSpell1 || slot == SummonerSpell2)
		slotId += 32; // summoner spells go to CanCastBits2

	if (canCast)
		this->canCastBits |= 1ULL << slotId;
	else
		this->canCastBits &= ~(1ULL << slotId);

	this->NotifyCanCastBits();
}
