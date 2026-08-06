#include "LvObjectBase.h"
#include "LvObjectFactory.h"
#include "LvGame.h"

#include "Assets/CharData.h"

LvObjectBase::LvObjectBase(const LvObjectFactory& builder, LvStatsBase* specifiedStats)
{
	this->position = LogAssert(builder.position.has_value()) ? builder.position.value() : Vector3();
	this->rotation = LogAssert(builder.rotation.has_value()) ? builder.rotation.value() : Vector3(0.0f, 0.0f, 1.0f);
	this->team = LogAssert(builder.team.has_value()) ? builder.team.value() : TT_BLUE;
	this->netId = builder.networkId.has_value() ? builder.networkId.value() : lvGame->GetNextNetworkId();
	this->objName = builder.objName.has_value() ? builder.objName.value() : std::format("<{:08x}>", this->netId);

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
