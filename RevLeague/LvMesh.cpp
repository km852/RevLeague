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

Vector3 LvMesh::GetPositionByCell(LvMeshCell* cell) const
{
	float x = (cell->X() * CellSize) + this->minBounds.X + (CellSize / 2.0f);
	float z = (cell->Z() * CellSize) + this->minBounds.Z + (CellSize / 2.0f);

	return Vector3(x, 0.0f, z);
}

void LvMesh::SetFlagInRadius(float xPos, float zPos, float radius, LvMeshCellFlags newFlags)
{
	LogAssert(radius >= 0.0f);

	int x1 = (int)((xPos - this->minBounds.X - radius) / CellSize - 0.5f);
	int x2 = (int)((xPos - this->minBounds.X + radius) / CellSize - 0.5f);
	int z1 = (int)((zPos - this->minBounds.Z - radius) / CellSize + 0.5f);
	int z2 = (int)((zPos - this->minBounds.Z + radius) / CellSize + 0.5f);

	LogAssert(x2 - x1 < 100);
	LogAssert(z2 - z1 < 100);

	if (z1 > z2 || x1 > x2)
		return;

	float radiusSqr = radius * radius;
	int itersRemaining = 1000;

	for (int currZ = z1; currZ <= z2; ++currZ)
	{
		for (int currX = x1; currX <= x2; ++currX)
		{
			LvMeshCell* cell = this->GetCell((short)currX, (short)currZ);
			if (cell)
			{
				Vector3 cellPos = this->GetPositionByCell(cell);
				float dx = cellPos.X - xPos;
				float dz = cellPos.Z - zPos;
				if (dx * dx + dz * dz <= radiusSqr)
					cell->mFlags = newFlags;
			}

			if (--itersRemaining < 0)
			{
				LogError("breaking out of an infinite loop");
				LogError("invoked with pos=({:.4f}; {:.4f}) and radius={:.4f}", xPos, zPos, radius);

				return;
			}
		}
	}
}

short LvMesh::IsWallOfGrass(float xPos, float zPos, float radius)
{
	// this is mostly a copy of LvMesh::SetFlagInRadius
	LogAssert(radius >= 0.0f);

	if (radius < 35.0f)
	{
		LvMeshCell* cell = this->GetCellFromMapPosition(Vector3(xPos, 0.f, zPos));
		return (cell && cell->IsWallOfGrass()) ? cell->mGrassSectionId : -1;
	}

	radius = std::min(500.f, radius);

	int x1 = (int)((xPos - this->minBounds.X - radius) / CellSize - 0.5f);
	int x2 = (int)((xPos - this->minBounds.X + radius) / CellSize - 0.5f);
	int z1 = (int)((zPos - this->minBounds.Z - radius) / CellSize + 0.5f);
	int z2 = (int)((zPos - this->minBounds.Z + radius) / CellSize + 0.5f);

	LogAssert(x2 - x1 < 100);
	LogAssert(z2 - z1 < 100);

	if (z1 > z2 || x1 > x2)
		return -1;

	float radiusSqr = radius * radius;
	int itersRemaining = 1000;

	int totalCells = 0, grassCells = 0;
	short lastGrassSectionId = -1;

	for (int currZ = z1; currZ <= z2; ++currZ)
	{
		for (int currX = x1; currX <= x2; ++currX)
		{
			LvMeshCell* cell = this->GetCell((short)currX, (short)currZ);
			if (cell)
			{
				Vector3 cellPos = this->GetPositionByCell(cell);
				float dx = cellPos.X - xPos;
				float dz = cellPos.Z - zPos;
				if (dx * dx + dz * dz <= radiusSqr)
				{
					if (!cell->IsImpassable())
						++totalCells;

					if (cell->IsWallOfGrass())
					{
						++grassCells;
						lastGrassSectionId = cell->mGrassSectionId;
					}
				}
			}

			if (--itersRemaining < 0)
			{
				LogError("breaking out of an infinite loop");
				LogError("invoked with pos=({:.4f}; {:.4f}) and radius={:.4f}", xPos, zPos, radius);

				return -1;
			}
		}
	}

	return (grassCells > 0 && grassCells >= (int)(totalCells * 0.4f)) ? lastGrassSectionId : -1;
}

bool LvMesh::LineOfSightTestInner(const Vector3& startPos, const Vector3& endPos, float maxRayLength, short sourceGrassSectionId, short targetGrassSectionId)
{
	if (Vector3::AlmostEqualXZ(startPos, endPos, 0.1f))
		return true;

	float distanceRemaining = (float)((endPos - startPos).LengthXZ() + 1.0);
	if (distanceRemaining > maxRayLength)
		return false;

	// source and target are in different grass sections
	if (targetGrassSectionId != -1 && sourceGrassSectionId != targetGrassSectionId)
		return false;

	constexpr float Accuracy = 8.0f;
	constexpr float MovementAmount = CellSize / Accuracy;

	Vector3 currPos = startPos;
	Vector3 moveInterval = (endPos - startPos).NormalizedXZ() * MovementAmount;

	int maxIterations = 5000;

	for (;;)
	{
		if (--maxIterations <= 0)
		{
			LogError("breaking out of an infinite loop");
			LogError("invoked with startPos=({}) endPos=({})", startPos, endPos);

			return false;
		}

		LvMeshCell* cell = this->GetCellFromMapPosition(currPos);
		if (!cell)
			return false;

		if (cell->IsImpassable() && !cell->IsFOWSeeThrough())
			return false;

		if (cell->IsWallOfGrass() && sourceGrassSectionId != cell->mGrassSectionId)
			return false;

		currPos += moveInterval;
		distanceRemaining -= MovementAmount;

		if (distanceRemaining < 0.0f)
			return true;
	}

	return true;
}

bool LvMesh::LineOfSightTest(const Vector3& startPos, const Vector3& endPos, float maxRayLength, short sourceGrassSectionId, short targetGrassSectionId)
{
	// TODO: generate equidistant points around EndPos and fire multiple rays towards there
	return this->LineOfSightTestInner(startPos, endPos, maxRayLength, sourceGrassSectionId, targetGrassSectionId);
}

LvMeshCell* LvMesh::GetCell(NavGridCellLocator locator) const
{
	if (locator.x < 0 || locator.z < 0 || locator.x >= cellCountX || locator.z >= cellCountZ)
		return nullptr;

	return &this->cells[locator.z * cellCountX + locator.x];
}

LvMeshCell* LvMesh::GetCellFromMapPosition(const Vector3& mapPos)
{
	float x = mapPos.X - this->minBounds.X;
	float z = mapPos.Z - this->minBounds.Z;

	if (x < 0.0f || z < 0.0f)
		return nullptr;

	int cellIndexX = static_cast<int>(x / CellSize);
	int cellIndexZ = static_cast<int>(z / CellSize);

	if (cellIndexX < 0 || cellIndexZ < 0 || cellIndexX >= cellCountX || cellIndexZ >= cellCountZ)
		return nullptr;

	return &this->cells[cellIndexZ * cellCountX + cellIndexX];
}
