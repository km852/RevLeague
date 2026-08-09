#pragma once

#include <unordered_map>

constexpr inline const char* g_KnownNames[] = {
"*Pos1",
"*Pos2",
"*Pos3",
"*Pos4",
"*Pos5",
"*Pos6",
"Order5*Pos1",
"Order5*Pos2",
"Order5*Pos3",
"Order5*Pos4",
"Order5*Pos5",
"Order5*Pos6",
"Chaos5*Pos1",
"Chaos5*Pos2",
"Chaos5*Pos3",
"Chaos5*Pos4",
"Chaos5*Pos5",
"Chaos5*Pos6",
"Order5*Facing1",
"Order5*Facing2",
"Order5*Facing3",
"Order5*Facing4",
"Order5*Facing5",
"Order5*Facing6",
"Chaos5*Facing1",
"Chaos5*Facing2",
"Chaos5*Facing3",
"Chaos5*Facing4",
"Chaos5*Facing5",
"Chaos5*Facing6",
};

inline std::unordered_map<unsigned int, const char*> g_NameHashMap;
