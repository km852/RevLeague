#include "LvProtocol.h"
#include "LvPlayer.h"

NvBinaryStreamWrite LvProtocol::CreatePeerRegistration(LvPlayer* player, unsigned long long encryptedKey)
{
    NvBinaryStreamWrite writer;

    writer.Write<int>(0); // header = 0
    writer.Write<int>(player->GetPlayerIndex()); // cid (client ID), we call it "player index"
    writer.Write<unsigned long long>(player->GetUserId()); // player ID
    writer.Write<int>(0);
    writer.Write<unsigned long long>(encryptedKey);
    writer.Write<int>(0); // this is likely unused in the game itself (TODO: verify with a data breakpoint)

    return writer;
}
