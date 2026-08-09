#pragma once

#include "LvTypes.h"
#include "NvLib.h"

class LvObjectBase : public NvNonCopyable {
private:
	std::string objName;
	NetworkId netId;
	LvTeam team;

	Vector3 position;
	Vector3 rotation;

	CharData* charData;
	std::unique_ptr<LvStatsBase> stats;

protected:
	LvObjectBase(const LvObjectFactory& builder, LvStatsBase* specifiedStats);

	virtual void RecalculateStats_AssignBaseStats(LvStatsBase* clonedStats);

public:
	virtual ~LvObjectBase();
	void PostCreateInit();

	NetworkId GetNetworkId() const { return netId; }
	LvTeam GetTeam() const { return team; }

	const std::string& GetName() const { return objName; }
	CharData* GetCharData() const { return charData; }

	virtual LvStatsBase* GetStats() { return stats.get(); }
	void RecalculateStats();

	// test whether this unit can directly see another unit (this is expensive, prefer CanSee instead)
	bool LineOfSightTest(LvObjectBase* target);
	// test whether this unit has vision on another unit (can be indirect, e.g. target revealed by yet another unit)
	virtual bool CanSee(LvObjectBase* target);
};

template <>
struct std::formatter<LvObjectBase*> : std::formatter<std::string> {
	auto format(LvObjectBase* obj, format_context& ctx) const { return formatter<string>::format(std::format("{:08x} {}", obj->GetNetworkId(), obj->GetName()), ctx); }
};
