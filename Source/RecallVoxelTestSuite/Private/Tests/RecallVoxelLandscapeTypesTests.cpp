// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Types/RecallVoxelLandscapeTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecallVoxelLandscape_SnapshotDefaults,
    "RecallVoxel.Landscape.SnapshotDefaults",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRecallVoxelLandscape_SnapshotDefaults::RunTest(const FString& Parameters)
{
	FVoxelLandscapeSnapshot Snap;
	TestTrue("Empty snapshot has no chunks", Snap.GridData.Chunks.IsEmpty());
	TestTrue("Empty snapshot has no dirty keys", Snap.GridData.DirtyKeys.IsEmpty());

	const FLandscapeChunkKey Key{FIntVector2(1, 2), 1};
	FVoxelLandscapeChunk& Chunk = Snap.GridData.Chunks.Add(Key);
	Chunk.ChunkCoord = Key.ChunkCoord;
	Chunk.SurfaceLayerIndex = Key.SurfaceLayerIndex;
	Chunk.Layers.Add(FName("Water")).Cells.Add({7.5f, FVector2f(2.f, 3.f)});
	Snap.GridData.DirtyKeys.Add(Key);

	TestEqual("Snapshot has one chunk", Snap.GridData.Chunks.Num(), 1);
	TestTrue("Snapshot preserves dirty key", Snap.GridData.DirtyKeys.Contains(Key));
	TestEqual("Snapshot preserves depth", Snap.GridData.Chunks[Key].Layers[FName("Water")].Cells[0].Depth, 7.5f);
	TestEqual("Snapshot preserves velocity", Snap.GridData.Chunks[Key].Layers[FName("Water")].Cells[0].Velocity, FVector2f(2.f, 3.f));

    return true;
}
