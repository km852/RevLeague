#include "LvMesh.h"

template<> inline Vector3 NvBinaryStreamRead::Read() { float x = Read<float>(); float y = Read<float>(); float z = Read<float>(); return Vector3(x, y, z); }

void LvMesh::CalculateGrassSections()
{
	std::function<void(LvMeshCell*, short)> dfs = [this, &dfs](LvMeshCell* cell, short currSectionId) -> void {
		if (!cell || !cell->IsWallOfGrass())
			return;

		if (cell->mGrassSectionId != -1)
			return;

		cell->mGrassSectionId = currSectionId;

		for (int dx = -1; dx <= 1; ++dx) {
			for (int dz = -1; dz <= 1; ++dz) {
				NavGridCellLocator newLocator = { (short)(cell->X() + dx), (short)(cell->Z() + dz)};
				dfs(this->GetCell(newLocator), currSectionId);
			}
		}
	};

	for (int i = 0; i < this->totalCellCount; ++i)
		this->cells[i].mGrassSectionId = -1;

	short currSectionId = 0;

	for (int i = 0; i < this->totalCellCount; ++i)
	{
		if (this->cells[i].mGrassSectionId == -1 && this->cells[i].IsWallOfGrass())
		{
			dfs(&this->cells[i], currSectionId++);
		}
	}

	LogDebug("Found {} disjoint grass sections", currSectionId);
}

LvMesh::LvMesh(const std::vector<unsigned char>& meshData)
{
	NvBinaryStreamRead meshReader(meshData.data(), meshData.size());

	int majorVersion = meshReader.Read<unsigned char>();
	int minorVersion = meshReader.Read<unsigned short>();

	LogDebug("Mesh version {}.{}", majorVersion, minorVersion);
	LogAssert(majorVersion == 3);
	LogAssert(minorVersion == 1);

	this->minBounds = meshReader.Read<Vector3>();
	this->maxBounds = meshReader.Read<Vector3>();

	LogDebug("Minimum bounds vector: {}", minBounds);
	LogDebug("Maximum bounds vector: {}", maxBounds);

	this->compressionCoefficientX = (unsigned short)((this->minBounds.X + this->maxBounds.X) / 2.f);
	this->compressionCoefficientZ = (unsigned short)((this->minBounds.Z + this->maxBounds.Z) / 2.f);

	float cellSize = meshReader.Read<float>();
	LogAssert(cellSize == 50.f);

	this->cellCountX = meshReader.Read<int>();
	this->cellCountZ = meshReader.Read<int>();
	this->totalCellCount = this->cellCountX * this->cellCountZ;

	LogDebug("Cell count X: {}, Z: {} [total {}]", this->cellCountX, cellCountZ, this->totalCellCount);

	this->cells = std::make_unique_for_overwrite<LvMeshCell[]>(this->totalCellCount); // i don't think make_unique_for_overwrite saves us the default constructor call here

	for (int i = 0; i < this->totalCellCount; ++i)
	{
		float centerHeight = meshReader.Read<float>();

		LogAssert(meshReader.Read<int>() == -1); // sessionId - always -1

		LogAssert(!meshReader.Read<float>()); // arrivalCost - always 0
		LogAssert(!meshReader.Read<int>()); // isOpen - always 0
		LogAssert(!meshReader.Read<float>()); // heuristic - always 0
		LogAssert(!meshReader.Read<int>()); // actorList - always 0

		short x = meshReader.Read<short>();
		short z = meshReader.Read<short>();

		LogAssert(x >= 0 && x < cellCountX);
		LogAssert(z >= 0 && z < cellCountZ);

		LogAssert(!meshReader.Read<float>()); // additionalCost - always 0
		LogAssert(!meshReader.Read<float>()); // hintAsGoodCell - always 0

		LogAssert(!meshReader.Read<int>()); // additionalCostRefCount - always 0
		LogAssert(meshReader.Read<int>() == -1); // goodCellSessionId - always -1

		float refHintWeight = meshReader.Read<float>(); // refHintWeight
		LogAssert(meshReader.Read<unsigned short>() == 9); // arrivalDirection - always 9

		LvMeshCellFlags flags = (LvMeshCellFlags)meshReader.Read<unsigned short>();

		short refHint1 = meshReader.Read<short>(); // refHint1
		short refHint2 = meshReader.Read<short>(); // refHint2

		if (!std::isfinite(refHintWeight)) // for some reason certain cells have refHintWeight = nan
			refHintWeight = 0.5f;

		this->cells[z * cellCountX + x].LoadFromFile(x, z, centerHeight, flags, refHintWeight, refHint1, refHint2);
	}

	this->heightSamplesX = meshReader.Read<int>();
	this->heightSamplesZ = meshReader.Read<int>();
	this->heightMapToGridTranslationX = meshReader.Read<float>();
	this->heightMapToGridTranslationZ = meshReader.Read<float>();

	this->totalHeightSampleCount = this->heightSamplesX * this->heightSamplesZ;
	this->heightSamples = std::make_unique_for_overwrite<float[]>(this->totalHeightSampleCount);

	for (int i = 0; i < this->totalHeightSampleCount; ++i)
		this->heightSamples[i] = meshReader.Read<float>();

	LogDebug("Height samples X: {}, Z: {} [total {}]", this->heightSamplesX, this->heightSamplesZ, this->totalHeightSampleCount);

	for (int i = 0; i < NavHintGridSize; ++i)
	{
		for (int j = 0; j < NavHintGridSize; ++j)
		{
			hintGrid.grid[i].distances[j] = meshReader.Read<float>();
		}

		hintGrid.grid[i].locator.x = meshReader.Read<short>();
		hintGrid.grid[i].locator.z = meshReader.Read<short>();
	}

	if (meshReader.GetCurrentPos() < meshData.size())
		LogWarning("only {}/{} bytes read from NavGrid file", meshReader.GetCurrentPos(), meshData.size());

	this->CalculateGrassSections();
}

LvMeshCell* LvMesh::GetCell(NavGridCellLocator locator)
{
	if (locator.x < 0 || locator.z < 0 || locator.x >= cellCountX || locator.z >= cellCountZ)
		return nullptr;

	return &this->cells[locator.z * cellCountX + locator.x];
}
