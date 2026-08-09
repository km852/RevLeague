#pragma once

#include "LvStatsBase.h"
#include "LvObjectHero.h"

class LvStatsHero final : public LvStatsBase {
protected:
	virtual void NotifyGold() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_Gold, (float)this->GetGold()); }
	virtual void NotifySetHealth() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_CurrentHealth, this->GetHealth()); }
	virtual void NotifySetMaxHealth() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_MaxHealth, this->GetMaxHealth().GetTotalValue()); }
	virtual void NotifySetResource() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_CurrentResource, this->GetResource()); }
	virtual void NotifySetMaxResource() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_MaxResource, this->GetMaxResource().GetTotalValue()); }
	virtual void NotifySetMovementSpeed() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_MovementSpeed, this->GetMovementSpeed().GetTotalValue()); }
	virtual void NotifySetAttackDelayOffset() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_AttackSpeed, this->GetAttackSpeedMultiplier()); }
	virtual void NotifySetRange() { this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_AttackRange, this->GetRange().GetTotalValue()); }
	virtual void NotifySetAttackDamage()
	{
		this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_AttackDamageBase, this->GetAttackDamage().GetBaseValue());
		this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_AttackDamageBonus, this->GetAttackDamage().GetBonusValue());
	}
	virtual void NotifyCanCastBits()
	{
		this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_CanCastBits1, (uint32_t)(this->canCastBits & 0xFFFFFFFF));
		this->GetReplicationManager()->NotifyStatChange(LvStatReplicationId::Hero_CanCastBits2, (uint32_t)(this->canCastBits >> 32));
	}

public:
	LvStatsHero(LvObjectHero* owner) : LvStatsBase(owner) {}
};
