#pragma once

#include "LvObjectBase.h"

class LvObjectHero : public LvObjectBase {
	friend LvObjectFactory;
protected:
	LvObjectHero(const LvObjectFactory& builder);
};
