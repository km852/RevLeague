#include "LvProtocol.h"
#include "LvPlayer.h"
#include "LvMap.h"
#include "LvGame.h"
#include "LvObjectBase.h"
#include "LvObjectHero.h"

#include "Assets/CharData.h"

NvBinaryStreamWrite LvProtocol::CreatePeerRegistration(LvPlayer* player, unsigned long long encryptedKey)
{
    NvBinaryStreamWrite writer(36);

    writer.Write<int>(0); // header = 0
    writer.Write<int>(player->GetPlayerIndex()); // cid (client ID), we call it "player index"
    writer.Write<unsigned long long>(player->GetUserId()); // player ID
    writer.Write<int>(0);
    writer.Write<unsigned long long>(encryptedKey);
    writer.Write<int>(0); // this is likely unused in the game itself (TODO: verify with a data breakpoint)

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateSendGameNumber(LvPlayer* player)
{
    NvBinaryStreamWrite writer(150);

    writer.Write(PKT_World_SendGameNumber);
    writer.Write<int>(0);
    writer.Write(lvGame->GetGameId());
    writer.WritePaddedString(player->GetPlayerName(), 128);

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateQueryStatusAnswer()
{
    NvBinaryStreamWrite writer(10);

    writer.Write(PKT_S2C_QueryStatusAns);
    writer.Write<int>(0);
    writer.Write<unsigned char>(1); // isReady = 1

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateSynchVersionAnswer()
{
    NvBinaryStreamWrite writer(5000);

    writer.Write(PKT_SynchVersionS2C);
    writer.Write<int>(0);
    writer.Write<unsigned char>(1); // versionCorrect = 1
    writer.Write(lvMap->GetId());

    int iters = 0;
    for (LvPlayer* player : lvGame->GetPlayers())
    {
        // length of one block is 0x9f

        writer.Write(player->GetUserId());
        writer.Write<short>(30); // player level
        writer.Write<int>(0); // summoner spell 1
        writer.Write<int>(0); // summoner spell 2
        writer.Write<unsigned char>(0); // is bot
        writer.Write<unsigned int>(EnumToNetwork(player->GetTeam()));
        writer.WriteRepeatByte(0, 64); // bot name
        writer.WriteRepeatByte(0, 64); // bot skin name
        writer.Write<int>(0); // bot difficulty
        writer.Write<int>(player->GetProfileIconId());

        ++iters;
    }

    for (/**/; iters < 12; ++iters)
    {
        writer.Write<unsigned long long>(0xFFFFFFFFFFFFFFFFull);
        writer.WriteRepeatByte(0, 147);
        writer.Write<unsigned int>(0xFFFFFFFFu); // profile icon
    }

    writer.WritePaddedString(EXPECTED_GAME_VERSION_STRING, 256);
    writer.WritePaddedString("CLASSIC", 128);

    writer.WriteRepeatByte(0, 25); // orderRankedTeamName
    writer.WriteRepeatByte(0, 7); // orderRankedTeamTag
    writer.WriteRepeatByte(0, 25); // chaosRankedTeamName
    writer.WriteRepeatByte(0, 7); // chaosRankedTeamTag

    writer.WriteRepeatByte(0, 1024); // client metrics address?
    writer.WriteRepeatByte(0, 1024); // client metrics path?
    writer.Write<short>(0); // client metrics port?

    writer.Write<unsigned char>(0); // only two lowermost bits are referenced (bit 0: metrics log to file; bit 1: is matched game (show leaver warning when exiting))
    writer.WriteRepeatByte(0, 5); // something related to in-game tips

    LogAssert(writer.GetUnderlyingBuffer().size() == 0x1146); // sanity check

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateLoadingTeamRosterUpdate()
{
    NvBinaryStreamWrite writer(512);

    writer.Write(PKT_Loading_TeamRosterUpdate);
    writer.WriteRepeatByte(0, 3); // struct padding (useless)
    writer.Write<int>(6); // order team max size (TODO: experiment with bigger values someday)
    writer.Write<int>(6); // chaos team max size
    writer.WriteRepeatByte(0, 4); // struct padding (useless)

    int orderTeamSize = 0, chaosTeamSize = 0;
    for (LvPlayer* player : lvGame->GetPlayers())
    {
        if (player->GetTeam() == TT_BLUE)
        {
            writer.Write(player->GetUserId());
            ++orderTeamSize;
        }
    }

    for (int i = orderTeamSize; i < 24; ++i)
        writer.Write<unsigned long long>(0);

    for (LvPlayer* player : lvGame->GetPlayers())
    {
        if (player->GetTeam() == TT_RED)
        {
            writer.Write(player->GetUserId());
            ++chaosTeamSize;
        }
    }

    for (int i = chaosTeamSize; i < 24; ++i)
        writer.Write<unsigned long long>(0);

    writer.Write<int>(orderTeamSize);
    writer.Write<int>(chaosTeamSize);

    LogAssert(writer.GetUnderlyingBuffer().size() == 0x198); // sanity check

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateLoadingSetSkin(LvPlayer* player)
{
    NvBinaryStreamWrite writer(64);

    std::string charName = player->GetHero()->GetCharData()->GetInGameName();

    writer.Write(PKT_Loading_SetSkin);
    writer.WriteRepeatByte(0, 7); // struct padding (useless)
    writer.Write<unsigned long long>(player->GetUserId());
    writer.Write<int>(player->GetSkinIndex());
    writer.Write<int>((int)charName.size() + 1);
    writer.WritePaddedString(charName, charName.size() + 1);

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateLoadingSetName(LvPlayer* player)
{
    NvBinaryStreamWrite writer(64);

    writer.Write(PKT_Loading_SetName);
    writer.WriteRepeatByte(0, 7); // struct padding (useless)
    writer.Write(player->GetUserId());
    writer.Write<int>(0);
    writer.Write<int>((int)player->GetPlayerName().size() + 1);
    writer.WritePaddedString(player->GetPlayerName(), player->GetPlayerName().size() + 1);

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreatePingLoadInfo(LvPlayer* player)
{
    NvBinaryStreamWrite writer(32);

    writer.Write(PKT_S2C_Ping_Load_Info);
    writer.Write<int>(0);
    writer.Write<int>(player->GetPlayerIndex());
    writer.Write<unsigned long long>(player->GetUserId());

    LvClient* client = player->GetClient();
    if (client)
    {
        auto& progress = client->GetLoadingProgress();
        writer.Write<float>(progress.percentageLoaded);
        writer.Write<float>(progress.eta);
        writer.Write<unsigned int>(progress.MakeExtraData());
        writer.Write<unsigned char>(progress.isReady ? 1 : 0);
    }
    else
    {
        writer.WriteRepeatByte(0, 13);
    }

    LogAssert(writer.GetUnderlyingBuffer().size() == 30); // sanity check

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateStartSpawn()
{
    NvBinaryStreamWrite writer(10);

    writer.Write(PKT_S2C_StartSpawn);
    writer.Write<int>(0);
    writer.Write<unsigned char>(0); // numbBotsOrder
    writer.Write<unsigned char>(0); // numbBotsChaos

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateEndSpawn()
{
    NvBinaryStreamWrite writer(10);

    writer.Write(PKT_S2C_EndSpawn);
    writer.Write<int>(0);

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateStartGame()
{
    NvBinaryStreamWrite writer(10);

    writer.Write(PKT_S2C_StartGame);
    writer.Write<int>(0);
    writer.Write<unsigned char>(0); // lowermost bit: is pause enabled

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateCreateHero(LvPlayer* player)
{
    NvBinaryStreamWrite writer(256);

    LvObjectHero* hero = player->GetHero();

    writer.Write(PKT_S2C_CreateHero);
    writer.Write<int>(0);
    writer.Write<NetworkId>(hero->GetNetworkId());
    writer.Write<int>(player->GetPlayerIndex());
    writer.Write<unsigned char>(0x40); // netNodeId - always 0x40
    writer.Write<unsigned char>(0); // skillLevel ???
    writer.Write<unsigned char>(hero->GetTeam() == TT_BLUE ? 1 : 0);
    writer.Write<unsigned char>(0); // isBot
    writer.Write<unsigned char>(0); // botRank
    writer.Write<unsigned char>(player->GetTeamPlayerIndex()); // spawnPosIdx (TODO: figure out if it's zero- or one-indexed)
    writer.Write<int>(player->GetSkinIndex());
    writer.WritePaddedString(hero->GetName(), 128);
    writer.WritePaddedString(hero->GetCharData()->GetInGameName(), 40);
    writer.Write<float>(0.0f); // deathDurationRemaining
    writer.Write<float>(0.0f); // timeSinceDeath
    writer.Write<unsigned int>(0); // flags (related to current death state)

    return writer;
}

NvBinaryStreamWrite LvProtocol::CreateAvatarInfo(LvPlayer* player)
{
    NvBinaryStreamWrite writer(512);

    writer.Write(PKT_AvatarInfo);
    writer.Write(player->GetHero()->GetNetworkId());

    for (int i = 0; i < 30; ++i)
        writer.Write<int>(0); // rune hash ?

    writer.Write<int>(0); // summoner spell 1 hash
    writer.Write<int>(0); // summoner spell 2 hash

    for (int i = 0; i < 80; ++i)
    {
        writer.Write<int>(0); // mastery ID
        writer.Write<char>(0); // mastery level
    }

    writer.Write<char>(30); // summoner level

    return writer;
}
