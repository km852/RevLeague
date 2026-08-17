#pragma once

#include "LvTypes.h"
#include "NvLib.h"

#include "LvClientVisionInfo.h"

class LvObjectBase : public NvNonCopyable {
	friend LvDebugInterface;
private:
	std::string objName;
	NetworkId netId;
	LvTeam team;

	Vector3 position;
	Vector3 rotation;

	CharData* charData;
	std::unique_ptr<LvStatsBase> stats;

	float visionRange;

	float gameplayCollisionRadius = 1.0f; // collision with missiles
	float selectionRadius = 1.0f; // grass acquisition radius
	float pathfindingRadius;

	LvClientVisionInfo clientVisionInfo[MaxGameClients];

	bool orderVisionGranted = false;
	bool chaosVisionGranted = false;

protected:
	LvObjectType objectType;

	LvObjectBase(const LvObjectFactory& builder, LvStatsBase* specifiedStats);

	virtual void RecalculateStats_AssignBaseStats(LvStatsBase* clonedStats);

public:
	virtual ~LvObjectBase();
	void PostCreateInit();

	Vector3 GetPosition() const { return position; }
	Vector3 GetRotation() const { return rotation; }
	NetworkId GetNetworkId() const { return netId; }
	LvTeam GetTeam() const { return team; }
	LvObjectType GetType() const { return objectType; }

	const std::string& GetName() const { return objName; }
	CharData* GetCharData() const { return charData; }

	float GetVisionRadius() const { return visionRange; }
	float GetPathfindingRadius() const { return pathfindingRadius; }
	float GetGameplayCollisionRadius() const { return gameplayCollisionRadius; }
	float GetSelectionRadius() const { return selectionRadius; }

	virtual LvStatsBase* GetStats() { return stats.get(); }
	void RecalculateStats();

	double GetDistance(const Vector3& other) const { return std::sqrt(GetDistanceSqr(other)); }
	double GetDistance(LvObjectBase* other) const { return GetDistance(other->position); }
	double GetDistanceSqr(const Vector3& other) const { return (other - this->position).SqrLengthXZ(); }
	double GetDistanceSqr(LvObjectBase* other) const { return GetDistanceSqr(other->position); }
	bool IsWithinDistance(const Vector3& other, double distance) const { return GetDistanceSqr(other) <= (distance * distance); }
	bool IsWithinDistance(LvObjectBase* other, double distance) const { return IsWithinDistance(other->position, distance); }

	// can team X see this unit? (use CanSee instead of this function to check visibility)
	bool IsTeamVisionGranted(LvTeam team) const { return team == TT_BLUE ? orderVisionGranted : (team == TT_RED ? chaosVisionGranted : true); }
	void SetTeamVisionGranted(LvTeam team, bool isVisible) { if (team == TT_BLUE) orderVisionGranted = isVisible; else if (team == TT_RED) chaosVisionGranted = isVisible; }

	LvClientVisionInfo* GetVisionInfoForPlayer(int playerIndex) { return &this->clientVisionInfo[playerIndex]; }

	virtual std::vector<NvBinaryStreamWrite> CreateEnterVisibilityPackets(LvClient* visionClient);
	virtual void WriteEnterVisibilityPacketSuffix(NvBinaryStreamWrite& visPacket, LvClient* visionClient);

	virtual void SendOnEnterVisionPackets(LvClient* visionClient) = 0;
	virtual void SendOnLeaveVisionPackets(LvClient* visionClient) = 0;

	// test whether this unit can directly see another unit (this is expensive, prefer CanSee instead)
	bool LineOfSightTest(LvObjectBase* target);
	// test whether this unit has vision on another unit (can be indirect, e.g. target revealed by yet another unit)
	virtual bool CanSee(LvObjectBase* target);

	virtual void Update(double dt);
	virtual void UpdateMovement(double dt);
};

template <>
struct std::formatter<LvObjectBase*> : std::formatter<std::string> {
	auto format(LvObjectBase* obj, format_context& ctx) const { return formatter<string>::format(std::format("{:08x} {}", obj->GetNetworkId(), obj->GetName()), ctx); }
};
