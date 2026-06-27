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
	if (StreamingSystem.IsValid() && SurfaceSystem.IsValid())
	{
		const FVoxelGrid& Grid = StreamingSystem->GetGrid();
		SurfaceSystem->StartSurfaceGeneration(Grid, Grid.GetAllChunkCoords());
		SurfaceSystem->ForceEndSurfaceGeneration();
	}
}

void URecallVoxelSurfaceSubsystem::Reset()
{
	if (SurfaceSystem.IsValid())
	{
		SurfaceSystem->SetAllSurfaceChunks({});
	}
}

void URecallVoxelSurfaceSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	FVoxelSurfaceSnapshot Snap;

	if (SurfaceSystem.IsValid())
	{
		Snap.SurfaceChunks = SurfaceSystem->GetAllSurfaceChunks();
	}

	OutSnapshot = FInstancedStruct::Make<FVoxelSurfaceSnapshot>(MoveTemp(Snap));
}

void URecallVoxelSurfaceSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
	const FVoxelSurfaceSnapshot* Snap = InSnapshot.GetPtr<FVoxelSurfaceSnapshot>();
	if (!Snap || !SurfaceSystem.IsValid())
	{
		return;
	}

	// Directly overwrite the subsystem's surface chunk map so landscape reads coherent heights.
	// Rendered mesh is left intentionally stale for one frame — normal dirty-chunk generation
	// will rebuild it asynchronously without blocking the restore path.
	SurfaceSystem->SetAllSurfaceChunks(Snap->SurfaceChunks);
}

void URecallVoxelSurfaceSubsystem::StartSurfaceGeneration()
{
	if (StreamingSystem.IsValid() && SurfaceSystem.IsValid())
	{
		const FVoxelGrid& Grid = StreamingSystem->GetGrid();
		SurfaceSystem->StartSurfaceGeneration(Grid, Grid.GetDirtyChunks());
	}
}

void URecallVoxelSurfaceSubsystem::ForceEndSurfaceGeneration()
{
	if (SurfaceSystem.IsValid())
	{
		SurfaceSystem->ForceEndSurfaceGeneration();
	}
}
