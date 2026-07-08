// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "System/RecallVoxelSurfaceSubsystem.h"

#include "Engine/World.h"
#include "Streaming/VoxelStreamingSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "System/VoxelSurfaceSubsystem.h"
#include "Types/RecallVoxelSurfaceTypes.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

void URecallVoxelSurfaceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UVoxelStreamingSubsystem>();
	StreamingSystem = UWorld::GetSubsystem<UVoxelStreamingSubsystem>(GetWorld());
	
	Collection.InitializeDependency<UVoxelSurfaceSubsystem>();
	SurfaceSystem = UWorld::GetSubsystem<UVoxelSurfaceSubsystem>(GetWorld());
}

void URecallVoxelSurfaceSubsystem::Start(const FRecallSimulationStartParams& Params)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelSurfaceSubsystem::Start"));
	
	if (StreamingSystem.IsValid() && SurfaceSystem.IsValid())
	{
		const FVoxelGrid& Grid = StreamingSystem->GetGrid();
		SurfaceSystem->StartSurfaceGeneration(Grid, Grid.GetAllChunkCoords());
		SurfaceSystem->ForceEndSurfaceGeneration();
	}
}

void URecallVoxelSurfaceSubsystem::Reset()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelSurfaceSubsystem::Reset"));

	if (SurfaceSystem.IsValid())
	{
		SurfaceSystem->SetAllSurfaceChunks({});
	}

	CachedUndoQueue.Reset();
}

void URecallVoxelSurfaceSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelSurfaceSubsystem::Save"));

	if (Context.IsRollback())
	{
		return;
	}

	FVoxelSurfaceSnapshot Snap;

	if (SurfaceSystem.IsValid())
	{
		Snap.FullChunks = SurfaceSystem->GetAllSurfaceChunks();
	}

	OutSnapshot = FInstancedStruct::Make<FVoxelSurfaceSnapshot>(MoveTemp(Snap));
}

void URecallVoxelSurfaceSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelSurfaceSubsystem::Restore"));

	const FVoxelSurfaceSnapshot* Snap = InSnapshot.GetPtr<FVoxelSurfaceSnapshot>();
	if (!Snap || !SurfaceSystem.IsValid())
	{
		return;
	}

	// Rendered mesh is left intentionally stale for one frame — normal dirty-chunk generation
	// rebuilds it asynchronously without blocking the restore path.
	if (Context.IsSnapshot())
	{
		SurfaceSystem->SetAllSurfaceChunks(Snap->FullChunks);
		CachedUndoQueue.Reset();
	}
	else
	{
		TMap<FIntVector2, FVoxelSurfaceChunk> State = SurfaceSystem->GetAllSurfaceChunks();
		CachedUndoQueue.RestoreTo(Context.Frame, State);
		SurfaceSystem->SetAllSurfaceChunks(State);
	}
}

void URecallVoxelSurfaceSubsystem::StartSurfaceGeneration()
{
	if (!StreamingSystem.IsValid() || !SurfaceSystem.IsValid())
	{
		return;
	}

	const FVoxelGrid& Grid = StreamingSystem->GetGrid();
	const TArray<FIntVector> DirtyCoords = Grid.GetDirtyChunks();

	// Snapshot pre-generation state of the surface columns about to be regenerated.
	PendingPreState.Reset();
	PendingDirtyCoords.Reset();
	
	const TMap<FIntVector2, FVoxelSurfaceChunk>& Current = SurfaceSystem->GetAllSurfaceChunks();
	for (const FIntVector& Coord : DirtyCoords)
	{
		const FIntVector2 Coord2D(Coord.X, Coord.Y);
		PendingDirtyCoords.Add(Coord2D);
		if (const FVoxelSurfaceChunk* Chunk = Current.Find(Coord2D))
		{
			PendingPreState.Add(Coord2D, *Chunk);
		}
		// No entry = chunk did not exist before; PushDiff will record it as new (undo = remove).
	}

	SurfaceSystem->StartSurfaceGeneration(Grid, DirtyCoords);
}

void URecallVoxelSurfaceSubsystem::ForceEndSurfaceGeneration()
{
	if (!SurfaceSystem.IsValid())
	{
		return;
	}

	SurfaceSystem->ForceEndSurfaceGeneration();

	if (!PendingDirtyCoords.IsEmpty())
	{
		// Build post-state restricted to dirty coords so PushDiff doesn't scan the entire chunk map.
		TMap<FIntVector2, FVoxelSurfaceChunk> PostState;
		const TMap<FIntVector2, FVoxelSurfaceChunk>& AllChunks = SurfaceSystem->GetAllSurfaceChunks();
		for (const FIntVector2& Coord : PendingDirtyCoords)
		{
			if (const FVoxelSurfaceChunk* Chunk = AllChunks.Find(Coord))
			{
				PostState.Add(Coord, *Chunk);
			}
		}

		const uint32 Frame = Recall::Simulation::Utils::GetFrame(this);
		CachedUndoQueue.PushDiff(Frame, PendingPreState, PostState);
		PendingPreState.Reset();
		PendingDirtyCoords.Reset();
	}
}
