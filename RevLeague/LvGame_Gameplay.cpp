#include "LvGame.h"
#include "LvMap.h"

constexpr double WorldUpdateInterval = 0.01; // seconds

void LvGame::GameUpdate_Gameplay()
{
	if (!this->isGameplayRunning)
		return;

	this->nextWorldUpdate += lvClock->GetCurrentFrameDelta();

	while (this->nextWorldUpdate > WorldUpdateInterval)
	{
		for (LvPlayer* player : GetInGamePlayers())
			player->GetClient()->UpdateClock();

		lvMap->Update(WorldUpdateInterval);

		this->nextWorldUpdate -= WorldUpdateInterval;

		lvClock->IncrementFrameId();
	}
}
