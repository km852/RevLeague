#pragma once

#include "LvTypes.h"

class LvClientVisionInfo final : public NvNonCopyable {
public:
	bool isCurrentlyVisible = false;
	bool wasPreviouslyVisible = false; // status during previous vision update
	bool wasVisibleAtLeastOnce = false;
};
