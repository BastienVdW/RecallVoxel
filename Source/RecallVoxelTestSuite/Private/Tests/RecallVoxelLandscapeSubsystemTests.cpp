// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Grid/VoxelLandscapeGrid.h"
#include "Types/RecallVoxelLandscapeTypes.h"

// Pure-data round-trip test: simulates the Save/Restore logic on raw structs
// without requiring a World or subsystem instance.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecallVoxelLandscape_SnapshotRoundtrip,
    "RecallVoxel.Landscape.SnapshotRoundtrip",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRecallVoxelLandscape_SnapshotRoundtrip::RunTest(const FString& Parameters)
{
    const FName  Water   = FName("Water");
	const FLandscapeChunkKey Key{FIntVector2(2, 3), 0};
	FVoxelLandscapeChunk Chunk;
	Chunk.ChunkCoord        = FIntVector2(2, 3);
	Chunk.SurfaceLayerIndex = 0;
	Chunk.bDirty = true;
	Chunk.bSleeping = true;
	Chunk.StableSimulationSteps = 4;

	TArray<FLandscapeCell> Cells;
	Cells.SetNumZeroed(18 * 18);
	Cells[5].Depth = 3.f;
	Cells[5].Velocity = FVector2f(1.f, 2.f);
	Cells[9].Depth = 7.f;
    FVoxelLandscapeLayer WaterLayer;
    WaterLayer.Cells = Cells;
    Chunk.Layers.Add(Water, WaterLayer);

	FVoxelLandscapeSnapshot Snap;
	Snap.GridData.Chunks.Add(Key, Chunk);
	Snap.GridData.DirtyKeys.Add(Key);

	FVoxelLandscapeGrid RestoredGrid;
	RestoredGrid.SetData(Snap.GridData);
	const FVoxelLandscapeChunk* Restored = RestoredGrid.Find(Key);
	TestNotNull("Chunk restored", Restored);
	if (!Restored) return false;

	TestEqual("Cell depth restored", Restored->Layers[Water].Cells[5].Depth, 3.f);
	TestEqual("Cell velocity restored", Restored->Layers[Water].Cells[5].Velocity, FVector2f(1.f, 2.f));
	TestTrue("Sleeping state restored", Restored->bSleeping);
	TestEqual("Stable steps restored", Restored->StableSimulationSteps, 4);
	TestTrue("Dirty key restored", RestoredGrid.GetDirtyKeySet().Contains(Key));
	const FLandscapeChunkKeyList* IndexedKeys = RestoredGrid.FindKeys(Key.ChunkCoord);
	TestTrue("Coordinate index rebuilt", IndexedKeys && IndexedKeys->Values.Contains(Key));

    return true;
}
