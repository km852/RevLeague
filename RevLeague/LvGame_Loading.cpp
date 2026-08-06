#include "LvGame.h"
#include "LvProtocol.h"

void LvGame::GameUpdate_Loading()
{
	if (this->loadingUpdateTimer.Seconds() > 0.7)
	{
		// Periodically broadcast info regarding loading progress to all players
		for (LvPlayer* targetPlayer : this->GetConnectedPlayers())
		{
			LvClient* targetClient = targetPlayer->GetClient();
			if (targetClient->GetState() < CST_LOADING || targetClient->GetState() > CST_POST_LOADED) // don't send data to in-game clients or those still in registration phase
				continue;

			for (LvPlayer* loadingPlayer : this->GetConnectedPlayers())
			{
				if (this->isGameplayRunning && loadingPlayer != targetPlayer) // if game is already running, don't send other players' loading state since we don't have up-to-date info about them
					continue;

				if (loadingPlayer->GetClient()->GetLoadingProgress().isDefaultConstructed) // no loading info about this client yet
					continue;

				targetClient->SendPacket(PCH_LowPriority, LvProtocol::CreatePingLoadInfo(loadingPlayer));
			}
		}

		for (LvPlayer* player : this->GetConnectedPlayers())
		{
			if (player->GetClient()->GetState() == CST_LOADED)
				player->GetClient()->FinishLoading();
		}

		bool shouldStartGame = true;
		for (LvPlayer* player : this->GetPlayers())
			shouldStartGame &= player->GetClient() && player->GetClient()->GetState() == CST_POST_LOADED;

		if (shouldStartGame || this->isGameplayRunning)
		{
			if (!this->isGameplayRunning)
			{
				LogInfo("All players have loaded, game is entering the running state");
				this->isGameplayRunning = true;
			}

			for (LvPlayer* player : this->GetConnectedPlayers())
			{
				if (player->GetClient()->GetState() == CST_POST_LOADED)
				{
					player->GetClient()->SendPacket(PCH_ServerToClient, LvProtocol::CreateStartGame());
					player->GetClient()->SetState(CST_IN_GAME);
				}
			}
		}

		this->loadingUpdateTimer.Reset();
	}
}
