#include "LvObjectBase.h"
#include "LvObjectFactory.h"
#include "LvGame.h"
#include "LvStatsBase.h"

#include "Assets/CharData.h"

LvObjectBase::LvObjectBase(const LvObjectFactory& builder, LvStatsBase* specifiedStats)
{
	this->position = LogAssert(builder.position.has_value()) ? builder.position.value() : Vector3();
	this->rotation = LogAssert(builder.rotation.has_value()) ? builder.rotation.value() : Vector3(0.0f, 0.0f, 1.0f);
	this->team = LogAssert(builder.team.has_value()) ? builder.team.value() : TT_BLUE;
	this->netId = builder.networkId.has_value() ? builder.networkId.value() : lvGame->GetNextNetworkId();
	this->objName = builder.objName.has_value() ? builder.objName.value() : std::format("<{:08x}>", this->netId);

	LogAssert(specifiedStats != nullptr);
	this->stats = std::unique_ptr<LvStatsBase>(specifiedStats);

	std::string charDataName = LogAssert(builder.charDataPreset.has_value()) ? builder.charDataPreset.value() : "_EmptyCharacter"s;
	this->charData = CharData::GetCharData(charDataName);
	if (!this->charData)
	{
		LogError("[{}] Cannot find CharData \"{}\" - falling back to defaults", this, charDataName);

		this->charData = CharData::GetCharData("_EmptyCharacter");
		LogAssert(this->charData != nullptr || !"This will probably crash the server!!!");
	}
}

LvObjectBase::~LvObjectBase()
{
	LogDebug("~ {}", this);
}

void LvObjectBase::PostCreateInit()
{
}

void LvObjectBase::RecalculateStats_AssignBaseStats(LvStatsBase* clonedStats)
{
	clonedStats->SetMaxHealth(this->charData->GetBaseHealth(), 0.0f);
	clonedStats->SetMaxResource(this->charData->GetBaseResource(), 0.0f);
	clonedStats->SetAttackDamage(this->charData->GetBaseAttackDamage(), 0.0f);
	clonedStats->SetAbilityPower(this->charData->GetBaseAbilityPower(), 0.0f);
	clonedStats->SetAttackDelayOffset(this->charData->GetAttackDelayOffset(), 0.0f);
	clonedStats->SetAttackCastDelayOffset(this->charData->GetAttackCastDelayOffset(), 0.0f);
	clonedStats->SetMovementSpeed(this->charData->GetMovementSpeed(), 0.0f);
	clonedStats->SetRange(this->charData->GetBaseRange(), 0.0f);
}

void LvObjectBase::RecalculateStats()
{
	std::unique_ptr<LvStatsBase> statsCopy = this->stats->CloneWithoutReplicator();

	this->RecalculateStats_AssignBaseStats(statsCopy.get());

	// sanity checks to ensure health and PAR do not exceed maximum values
	float maxHealth = statsCopy->GetMaxHealth().GetTotalValue();
	float maxResource = statsCopy->GetMaxResource().GetTotalValue();

	if (statsCopy->GetHealth() > maxHealth)
		statsCopy->SetHealth(maxHealth);

	if (statsCopy->GetResource() > maxResource)
		statsCopy->SetResource(maxResource);

	this->stats->ApplyUpdatedStats(statsCopy.get());
}

bool LvObjectBase::LineOfSightTest(LvObjectBase* target)
{
	return false;
}

bool LvObjectBase::CanSee(LvObjectBase* target)
{
	return false;
}
