#include "LvObjectHero.h"
#include "LvStatsHero.h"
#include "LvPlayer.h"
#include "LvObjectFactory.h"

LvObjectHero::LvObjectHero(const LvObjectFactory& builder) : LvObjectBase(builder, new LvStatsHero(this))
{
	this->objectType = OBJ_HERO;
	this->player = LogAssert(builder.player.has_value()) ? builder.player.value() : nullptr;
}
