#include <print>
#include <filesystem>
#include <fstream>
#include <format>
#include <vector>
#include <unordered_set>

#include "NvLib.h"
#include "NameDB.h"
#include "json.hpp"

using json = nlohmann::json;

enum TypeGroupIndex {
	TYPE_INDEX_Int32List = 0,
	TYPE_INDEX_Float32List = 1,
	TYPE_INDEX_FixedPointFloatList = 2,
	TYPE_INDEX_Int16List = 3,
	TYPE_INDEX_Int8List = 4,
	TYPE_INDEX_BitList = 5,
	TYPE_INDEX_FixedPointFloatListVec3 = 6,
	TYPE_INDEX_Float32ListVec3 = 7,
	TYPE_INDEX_FixedPointFloatListVec2 = 8,
	TYPE_INDEX_Float32ListVec2 = 9,
	TYPE_INDEX_FixedPointFloatListVec4 = 10,
	TYPE_INDEX_Float32ListVec4 = 11,
	TYPE_INDEX_StringList = 12,
};


NLOHMANN_JSON_SERIALIZE_ENUM(TypeGroupIndex,
{
	{ TYPE_INDEX_Int32List, "Int32" }, { TYPE_INDEX_Float32List, "Float32" }, { TYPE_INDEX_FixedPointFloatList, "FloatFixed" }, { TYPE_INDEX_Int16List, "Int16" },
	{ TYPE_INDEX_Int8List, "Int8" }, { TYPE_INDEX_BitList, "Bit" }, { TYPE_INDEX_FixedPointFloatListVec3, "FixedVec3" }, { TYPE_INDEX_Float32ListVec3, "FloatVec3" },
	{ TYPE_INDEX_FixedPointFloatListVec2, "FixedVec2" }, { TYPE_INDEX_Float32ListVec2, "FloatVec2" }, { TYPE_INDEX_FixedPointFloatListVec4, "FixedVec4" },
	{ TYPE_INDEX_Float32ListVec3, "FloatVec4" }, { TYPE_INDEX_StringList, "String" },
});

template <int N>
struct VectorB {
	unsigned char v[N] = { 0 };
};

template <int N>
struct VectorF {
	float v[N] = { 0 };

	VectorF() {}
	VectorF(VectorB<N> bv) {
		for (int i = 0; i < N; ++i)
			v[i] = bv.v[i] / 10.0f;
	}
};

template<> VectorF<2> NvBinaryStreamRead::Read() { VectorF<2> v; v.v[0] = Read<float>(); v.v[1] = Read<float>(); return v; }
template<> VectorF<3> NvBinaryStreamRead::Read() { VectorF<3> v; v.v[0] = Read<float>(); v.v[1] = Read<float>(); v.v[2] = Read<float>(); return v; }
template<> VectorF<4> NvBinaryStreamRead::Read() { VectorF<4> v; v.v[0] = Read<float>(); v.v[1] = Read<float>(); v.v[2] = Read<float>(); v.v[3] = Read<float>(); return v; }
template<> VectorB<2> NvBinaryStreamRead::Read() { VectorB<2> v; v.v[0] = Read<unsigned char>(); v.v[1] = Read<unsigned char>(); return v; }
template<> VectorB<3> NvBinaryStreamRead::Read() { VectorB<3> v; v.v[0] = Read<unsigned char>(); v.v[1] = Read<unsigned char>(); v.v[2] = Read<unsigned char>(); return v; }
template<> VectorB<4> NvBinaryStreamRead::Read() { VectorB<4> v; v.v[0] = Read<unsigned char>(); v.v[1] = Read<unsigned char>(); v.v[2] = Read<unsigned char>(); v.v[3] = Read<unsigned char>(); return v; }

static std::optional<std::vector<unsigned char>> LoadFileRaw(const std::string& filePath)
{
	std::ifstream f(filePath, std::ios::binary);
	if (!f)
	{
		std::print("Could not open file: {}", filePath);
		return std::nullopt;
	}

	auto size = std::filesystem::file_size(filePath);

	std::vector<unsigned char> result;
	result.resize(size);
	if (!f.read((char*)&result[0], size))
	{
		std::print("Could not open file: {}", filePath);
		return std::nullopt;
	}

	return result;
}

struct InibinKey {
	unsigned int key;
	TypeGroupIndex typeId;
};

union InibinValue {
	long long valInteger;
	VectorF<4> valVector;
	float valFloat;

	explicit InibinValue(unsigned char c) { valInteger = c; }
	explicit InibinValue(short c) { valInteger = c; }
	explicit InibinValue(int c) { valInteger = c; }
	explicit InibinValue(float c) { valFloat = c; }
	explicit InibinValue(VectorF<2> c) { valVector.v[0] = c.v[0]; valVector.v[1] = c.v[1]; }
	explicit InibinValue(VectorF<3> c) { valVector.v[0] = c.v[0]; valVector.v[1] = c.v[1]; valVector.v[2] = c.v[2]; }
	explicit InibinValue(VectorF<4> c) { valVector.v[0] = c.v[0]; valVector.v[1] = c.v[1]; valVector.v[2] = c.v[2]; valVector.v[3] = c.v[3]; }
	explicit InibinValue(VectorB<2> c) { VectorF<2> nv = c; valVector.v[0] = nv.v[0]; valVector.v[1] = nv.v[1]; }
	explicit InibinValue(VectorB<3> c) { VectorF<3> nv = c; valVector.v[0] = nv.v[0]; valVector.v[1] = nv.v[1]; valVector.v[2] = nv.v[2]; }
	explicit InibinValue(VectorB<4> c) { VectorF<4> nv = c; valVector.v[0] = nv.v[0]; valVector.v[1] = nv.v[1]; valVector.v[2] = nv.v[2]; valVector.v[3] = nv.v[3]; }
};

static inline char hashToLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + 32;
	return c;
}

static unsigned int SDBMHash(const char* s)
{
	unsigned int h = 0;

	while (*s)
	{
		h = ((unsigned char)hashToLower(*s)) + (h << 6) + (h << 16) - h;
		++s;
	}

	return h;
}

static std::vector<InibinKey> allKeys;
static std::vector<InibinValue> allValues;

static unsigned short ReadKeys(NvBinaryStreamRead& stream, TypeGroupIndex tt)
{
	unsigned short entryCount = stream.Read<unsigned short>();

	for (int i = 0; i < entryCount; ++i)
		allKeys.push_back({ .key = stream.Read<unsigned int>(), .typeId = tt });

	return entryCount;
}

void ReadFloatFixedList(NvBinaryStreamRead& stream)
{
	abort();
}

void ReadBitList(NvBinaryStreamRead& stream)
{
	abort();
}

void ReadFloatFixedVec3List(NvBinaryStreamRead& stream)
{
	abort();
}

template <typename T>
void ReadGeneric(NvBinaryStreamRead& stream, TypeGroupIndex tt)
{
	unsigned short entryCount = ReadKeys(stream, tt);
	for (int i = 0; i < entryCount; ++i)
		allValues.emplace_back(stream.Read<T>());
}

void ReadInibin(NvBinaryStreamRead& stream)
{
	int version = stream.Read<char>();
	if (version != 2)
		throw NvSystemException(std::format("Only .inibin v2 files are supported (saw version {})", version));

	stream.SkipBytes(2);

	int presentFields = stream.Read<unsigned short>();
	if (presentFields & (1 << 0)) ReadGeneric<int>(stream, TYPE_INDEX_Int32List);
	if (presentFields & (1 << 1)) ReadGeneric<float>(stream, TYPE_INDEX_Float32List);
	if (presentFields & (1 << 2)) ReadFloatFixedList(stream);
	if (presentFields & (1 << 3)) ReadGeneric<short>(stream, TYPE_INDEX_Int16List);
	if (presentFields & (1 << 4)) ReadGeneric<unsigned char>(stream, TYPE_INDEX_Int8List);
	if (presentFields & (1 << 5)) ReadBitList(stream);
	if (presentFields & (1 << 6)) ReadGeneric<VectorB<3>>(stream, TYPE_INDEX_FixedPointFloatListVec3);
	if (presentFields & (1 << 7)) ReadGeneric<VectorF<3>>(stream, TYPE_INDEX_Float32ListVec3);
	if (presentFields & (1 << 8)) ReadGeneric<VectorB<2>>(stream, TYPE_INDEX_FixedPointFloatListVec2);
	if (presentFields & (1 << 9)) ReadGeneric<VectorF<2>>(stream, TYPE_INDEX_Float32ListVec2);
	if (presentFields & (1 << 10)) ReadGeneric<VectorB<4>>(stream, TYPE_INDEX_FixedPointFloatListVec4);
	if (presentFields & (1 << 11)) ReadGeneric<VectorF<4>>(stream, TYPE_INDEX_Float32ListVec4);
	if (presentFields & (1 << 12)) ReadGeneric<short>(stream, TYPE_INDEX_StringList);
	if (presentFields & (1 << 13)) abort();
	if (presentFields & (1 << 14)) abort();
	if (presentFields & (1 << 15)) abort();

	std::vector<unsigned char> remainingBytes = stream.ReadRemainingBytes();

	std::unordered_set<unsigned int> hashesSeen;

	json j;

	for (size_t i = 0; i < allKeys.size(); ++i)
	{
		const InibinKey& k = allKeys[i];
		const InibinValue& v = allValues[i];

		auto it = g_NameHashMap.find(k.key);
		std::string resolvedKey = it == g_NameHashMap.end() ? std::format("__unk_{:08x}", k.key) : it->second;

		if (hashesSeen.contains(k.key))
			std::print("Warning: Duplicate key {} ({})\n", k.key, resolvedKey);

		hashesSeen.insert(k.key);

		json entryJson = json{
			{ "type", k.typeId }
		};

		if (k.typeId == TYPE_INDEX_Int32List || k.typeId == TYPE_INDEX_Int16List || k.typeId == TYPE_INDEX_Int8List)
			entryJson["value"] = v.valInteger;
		else if (k.typeId == TYPE_INDEX_Float32List)
			entryJson["value"] = v.valFloat;
		else if (k.typeId == TYPE_INDEX_FixedPointFloatListVec2 || k.typeId == TYPE_INDEX_Float32ListVec2)
			entryJson["value"] = json{ {"x", v.valVector.v[0]}, {"y", v.valVector.v[0]} };
		else if (k.typeId == TYPE_INDEX_FixedPointFloatListVec3 || k.typeId == TYPE_INDEX_Float32ListVec3)
			entryJson["value"] = json{ {"x", v.valVector.v[0]}, {"y", v.valVector.v[1]}, {"z", v.valVector.v[2]} };
		else if (k.typeId == TYPE_INDEX_FixedPointFloatListVec4 || k.typeId == TYPE_INDEX_Float32ListVec4)
			entryJson["value"] = json{ {"x", v.valVector.v[0]}, {"y", v.valVector.v[1]}, {"z", v.valVector.v[2]}, {"w", v.valVector.v[3]} };
		else if (k.typeId == TYPE_INDEX_StringList)
			entryJson["value"] = std::string((char*)remainingBytes.data() + v.valInteger);

		j[resolvedKey] = entryJson;
	}

	std::print("{}", j.dump(2));
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::print("Usage: InibinReader <inibin file path>\n");
		return 1;
	}

	for (const char* name : g_KnownNames)
	{
		unsigned int h = SDBMHash(name);

		if (g_NameHashMap.contains(h))
		{
			std::print("DB warning: hash collision between \"{}\" and \"{}\"\n", name, g_NameHashMap[h]);
			continue;
		}

		g_NameHashMap[h] = name;
	}

	auto fileData = LoadFileRaw(argv[1]);
	if (fileData.value_or({}).size() == 0)
		return 1;

	const auto& vec = fileData.value();
	NvBinaryStreamRead stream(vec.data(), vec.size());

	ReadInibin(stream);

	return 0;
}
