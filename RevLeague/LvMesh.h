#pragma once

#include "NvLib.h"
#include "LvTypes.h"

struct NavGridCellLocator final {
	short x = -1;
	short z = -1;

	bool operator==(const NavGridCellLocator& rhs) const { return x == rhs.x && z == rhs.z; }
	bool operator!=(const NavGridCellLocator& rhs) const { return x != rhs.x || z != rhs.z; }
};

static constexpr int NavHintGridSize = 900;

struct NavHintGridNode
{
	float distances[NavHintGridSize];
	NavGridCellLocator locator;
};

struct NavHintGrid
{
	NavHintGridNode grid[NavHintGridSize];
	float GetCost(short from[2], short to[2], float weight1, float weight2);
};

class LvMeshCell final {
public:
	float mCenterHeight = 0.f;
	int mSessionID = -1;
	float mArrivalCost = 0.f;
	bool mIsOpen = false;
	short mGrassSectionId = -1;
	float mHeuristic = 0.0f;
	//ActorInterface* mActorList = nullptr;
	NavGridCellLocator mLocator;
	float mAdditionalCost = 0.0f;
	float mHintAsGoodCell = 0.0f;
	int mAdditionalCostRefCount = 0;
	int mGoodCellSessionID = -1;
	float mRefHintWeight = 0.5f;
	unsigned char mArrivalDirection = 9;
	LvMeshCellFlags mFlags = (LvMeshCellFlags)0;
	short mRefHintNode[2] = { 0 };

	short X() { return mLocator.x; }
	short Z() { return mLocator.z; }

	bool IsWallOfGrass() const { return (mFlags & MCF_GRASS) != 0; }

	void LoadFromFile(short x, short z, float centerHeight, LvMeshCellFlags flags, float refHintWeight, short refHint1, short refHint2)
	{
		mLocator.x = x;
		mLocator.z = z;
		mCenterHeight = centerHeight;
		mFlags = flags;
		mRefHintWeight = refHintWeight;
		mRefHintNode[0] = refHint1;
		mRefHintNode[1] = refHint2;
	}
};

class LvMesh final : public NvNonCopyable {
private:
	constexpr static inline float CellSize = 50.0f; // not really up for debate

	int cellCountX;
	int cellCountZ;
	int totalCellCount;

	Vector3 minBounds;
	Vector3 maxBounds;

	int heightSamplesX;
	int heightSamplesZ;
	int totalHeightSampleCount;

	float heightMapToGridTranslationX;
	float heightMapToGridTranslationZ;

	unsigned short compressionCoefficientX;
	unsigned short compressionCoefficientZ;

	std::unique_ptr<LvMeshCell[]> cells;
	std::unique_ptr<float[]> heightSamples;

	NavHintGrid hintGrid;

	void CalculateGrassSections();

public:
	explicit LvMesh(const std::vector<unsigned char>& meshData);

	LvMeshCell* GetCell(NavGridCellLocator locator);
};

inline LvMesh* lvMesh;
