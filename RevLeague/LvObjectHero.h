#pragma once

#include "LvObjectBase.h"

class LvObjectHero final : public LvObjectBase {
	friend LvObjectFactory;

private:
	LvPlayer* player;

protected:
	LvObjectHero(const LvObjectFactory& builder);

public:
	LvPlayer* GetPlayer() { return player; }
};
