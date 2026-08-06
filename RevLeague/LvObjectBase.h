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

protected:
	LvObjectBase(const LvObjectFactory& builder, LvStatsBase* specifiedStats);

public:
	virtual ~LvObjectBase();
	void PostCreateInit();

	NetworkId GetNetworkId() const { return netId; }
	LvTeam GetTeam() const { return team; }

	const std::string& GetName() const { return objName; }
	CharData* GetCharData() const { return charData; }
};

template <>
struct std::formatter<LvObjectBase*> : std::formatter<std::string> {
	auto format(LvObjectBase* obj, format_context& ctx) const { return formatter<string>::format(std::format("{:08x} {}", obj->GetNetworkId(), obj->GetName()), ctx); }
};
