#include "LvObjectHero.h"
#include "LvStatsHero.h"
#include "LvClient.h"
#include "LvPlayer.h"
#include "LvProtocol.h"
#include "LvObjectFactory.h"

LvObjectHero::LvObjectHero(const LvObjectFactory& builder) : LvObjectBase(builder, new LvStatsHero(this))
{
	this->objectType = OBJ_HERO;
	this->player = LogAssert(builder.player.has_value()) ? builder.player.value() : nullptr;
}

void LvObjectHero::SendOnEnterVisionPackets(LvClient* visionClient)
{
	if (!LogAssert(visionClient->GetState() == CST_IN_GAME))
		return;

	visionClient->SendPacket(PCH_ServerToClient, LvProtocol::CreateEnterVisibility(this, visionClient));
}

void LvObjectHero::SendOnLeaveVisionPackets(LvClient* visionClient)
{
	if (!LogAssert(visionClient->GetState() == CST_IN_GAME))
		return;
}
