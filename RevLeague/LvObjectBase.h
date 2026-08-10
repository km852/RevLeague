#pragma once

#include "LvTypes.h"
#include "NvLib.h"

class LvObjectBase : public NvNonCopyable {
private:
	std::string objName;
	NetworkId netId;
	LvTeam team;
	LvObjectType objectType;

	Vector3 position;
	Vector3 rotation;

	float visionRange;

	CharData* charData;
	std::unique_ptr<LvStatsBase> stats;

	float gameplayCollisionRadius = 1.0f; // collision with missiles
	float selectionRadius = 1.0f; // grass acquisition radius
	float pathfindingRadius;

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

	double GetDistance(const Vector3& other) const { return std::sqrt(GetDistanceSqr(other)); }
	double GetDistance(LvObjectBase* other) const { return GetDistance(other->position); }
	double GetDistanceSqr(const Vector3& other) const { return (other - this->position).SqrLengthXZ(); }
	double GetDistanceSqr(LvObjectBase* other) const { return GetDistanceSqr(other->position); }
	bool IsWithinDistance(const Vector3& other, double distance) const { return GetDistanceSqr(other) <= (distance * distance); }
	bool IsWithinDistance(LvObjectBase* other, double distance) const { return IsWithinDistance(other->position, distance); }

	// test whether this unit can directly see another unit (this is expensive, prefer CanSee instead)
	bool LineOfSightTest(LvObjectBase* target);
	// test whether this unit has vision on another unit (can be indirect, e.g. target revealed by yet another unit)
	virtual bool CanSee(LvObjectBase* target);

	virtual void Update(float dt);
	virtual void UpdateMovement(float dt);
};

template <>
struct std::formatter<LvObjectBase*> : std::formatter<std::string> {
	auto format(LvObjectBase* obj, format_context& ctx) const { return formatter<string>::format(std::format("{:08x} {}", obj->GetNetworkId(), obj->GetName()), ctx); }
};
