#include "LvObjectBase.h"
#include "LvObjectFactory.h"
#include "LvGame.h"
#include "LvMap.h"
#include "LvMesh.h"
#include "LvDebugInterface.h"
#include "LvStatsBase.h"

#include "Assets/CharData.h"

LvObjectBase::LvObjectBase(const LvObjectFactory& builder, LvStatsBase* specifiedStats)
{
	this->objectType = OBJ_BASE;

	this->position = LogAssert(builder.position.has_value()) ? builder.position.value() : Vector3();
	this->rotation = LogAssert(builder.rotation.has_value()) ? builder.rotation.value() : Vector3(0.0f, 0.0f, 1.0f);
	this->team = LogAssert(builder.team.has_value()) ? builder.team.value() : TT_BLUE;
	this->netId = builder.networkId.has_value() ? builder.networkId.value() : lvGame->GetNextNetworkId();
	this->objName = builder.objName.has_value() ? builder.objName.value() : std::format("<{:08x}>", this->netId);
	this->visionRange = builder.visionRadius.has_value() ? builder.visionRadius.value() : 500.f;

	LogAssert(specifiedStats != nullptr);
	this->stats = std::unique_ptr<LvStatsBase>(specifiedStats);

	std::string charDataName = LogAssert(builder.charDataPreset.has_value()) ? builder.charDataPreset.value() : "_EmptyCharacter"s;
	this->charData = CharData::GetCharData(charDataName);
	if (!this->charData)
	{
		LogError("[{}] Cannot find CharData \"{}\" - falling back to defaults", this, charDataName);

		this->charData = CharData::GetCharData("_EmptyCharacter"s);
		LogAssert(this->charData != nullptr || !"This will probably crash the server!!!");
	}

	if (this->charData)
	{
		this->gameplayCollisionRadius = this->charData->GetGameplayCollisionRadius();
		this->selectionRadius = this->charData->GetSelectionRadius();
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

std::vector<NvBinaryStreamWrite> LvObjectBase::CreateEnterVisibilityPackets(LvClient* visionClient)
{
	return {};
}

void LvObjectBase::WriteEnterVisibilityPacketSuffix(NvBinaryStreamWrite& visPacket, LvClient* visionClient)
{
	visPacket.Write<char>(0); // inventory info
	visPacket.Write<char>(0); // shield info (if 1, then three floats follow)
	visPacket.Write<float>(0); // some unknown float (movement speed override?)
	visPacket.Write<char>(0); // waypoint list
}

bool LvObjectBase::LineOfSightTest(LvObjectBase* target)
{
	if (target == this)
		return true;

	if (!this->IsWithinDistance(target, this->visionRange))
		return false;

	short grassSectionId = lvMesh->IsWallOfGrass(this->position.X, this->position.Z, this->selectionRadius);
	short enemyGrassSectionId = lvMesh->IsWallOfGrass(target->position.X, target->position.Z, target->selectionRadius);

	bool normalLoS = lvMesh->LineOfSightTest(this->position, target->position, this->visionRange, grassSectionId, enemyGrassSectionId);
	bool inverseLoS = false;

	// for champions, also test LoS coming from the enemy to our unit
	// this is to avoid a situation where map geometry unluckily causes one champion to see an enemy, but not the other way around
	if (!normalLoS && this->objectType == OBJ_HERO)
	{
		float smallerVisionRadius = std::min(this->visionRange, target->visionRange);
		if (this->IsWithinDistance(target, smallerVisionRadius))
		{
			// the order of arguments grassSectionId and enemyGrassSectionId is correct here
			inverseLoS = lvMesh->LineOfSightTest(target->position, this->position, smallerVisionRadius, grassSectionId, enemyGrassSectionId);
		}
	}

	return normalLoS || inverseLoS;
}

bool LvObjectBase::CanSee(LvObjectBase* target)
{
	return false;
}

void LvObjectBase::Update(double dt)
{
	lvDebug->OnObjectUpdate(this);
}

void LvObjectBase::UpdateMovement(double dt)
{
}
