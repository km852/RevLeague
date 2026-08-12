#pragma once

#include "LvTypes.h"
#include "LvMap.h"
#include "LvObjectBase.h"

class LvObjectIterator final : public NvNonCopyable {
	std::optional<LvTeam> team;
	std::optional<LvTeam> notInTeam;
	std::optional<std::pair<Vector3, double>> withinRange; // within radius of a particular center point

	auto GetInitialView() const
	{
		switch (team.value_or(TT_NONE))
		{
		case TT_BLUE:
			return lvMap->blueObjects | std::views::all;
		case TT_RED:
			return lvMap->redObjects | std::views::all;
		case TT_NEUTRAL:
			return lvMap->neutralObjects | std::views::all;
		default:
			return lvMap->objects | std::views::all;
		}
	}

public:
	LvObjectIterator& Team(LvTeam team_) { this->team = team_; return *this; }
	LvObjectIterator& NotInTeam(LvTeam team_) { this->notInTeam = team_; return *this; }
	LvObjectIterator& WithinRange(const Vector3& pos, double radius) { this->withinRange = std::make_pair(pos, radius); return *this; }

	auto Iterate() const
	{
		return GetInitialView() |
			std::views::filter([this](LvObjectBase* obj) { return this->notInTeam.has_value() ? obj->GetTeam() != notInTeam.value() : true; }) |
			std::views::filter([this](LvObjectBase* obj) { return this->withinRange.has_value() ? obj->IsWithinDistance(this->withinRange->first, this->withinRange->second) : true; });
	}

	auto IterateAll() const { return lvMap->objects | std::views::all; }
};
