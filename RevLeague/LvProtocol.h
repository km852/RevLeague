#pragma once

#include "NvLib.h"
#include "LvTypes.h"

namespace LvProtocol {
	NvBinaryStreamWrite CreatePeerRegistration(LvPlayer* player, unsigned long long encryptedKey);
}
