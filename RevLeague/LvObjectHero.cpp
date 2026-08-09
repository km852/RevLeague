#include "LvObjectHero.h"
#include "LvStatsHero.h"

LvObjectHero::LvObjectHero(const LvObjectFactory& builder) : LvObjectBase(builder, new LvStatsHero(this))
{
}
