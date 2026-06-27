// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "System/RecallVoxelPhysicsSubsystem.h"

#include "Async/ParallelFor.h"
#include "Engine/World.h"
#include "Geometry/VoxelMarchingCubes.h"
#include "Grid/VoxelGrid.h"
#include "Streaming/VoxelStreamingSubsystem.h"
#include "System/RecallVoxelSubsystem.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "Physics/Common/RecallPhysicsCommonShapeTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Types/RecallVoxelTypes.h"
#include "Voxel/VoxelTypes.h"

// ---------------------------------------------------------------------------
// Initialize / Deinitialize
// ---------------------------------------------------------------------------

void URecallVoxelPhysicsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Collection.InitializeDependency<URecallPhysicsSubsystem>();
    Collection.InitializeDependency<UVoxelStreamingSubsystem>();

    RecallPhysicsSystem  = UWorld::GetSubsystem<URecallPhysicsSubsystem>(GetWorld());
    VoxelStreamingSystem = UWorld::GetSubsystem<UVoxelStreamingSubsystem>(GetWorld());

    if (VoxelStreamingSystem.IsValid())
    {
        EvictedHandle = VoxelStreamingSystem->OnChunkEvicted.AddUObject(
            this, &URecallVoxelPhysicsSubsystem::OnChunkEvicted);
    }
}

void URecallVoxelPhysicsSubsystem::Deinitialize()
{
    if (VoxelStreamingSystem.IsValid())
    {
        VoxelStreamingSystem->OnChunkEvicted.Remove(EvictedHandle);
    }
    EvictedHandle.Reset();
    VoxelStreamingSystem.Reset();
    RecallPhysicsSystem.Reset();

    Super::Deinitialize();
}

void URecallVoxelPhysicsSubsystem::Start(const FRecallSimulationStartParams& Params)
{
    if (!VoxelStreamingSystem.IsValid())
    {
        return;
    }

	const FVoxelGrid& Grid = VoxelStreamingSystem->GetGrid();
    OnChunksBaked(Grid, Grid.GetAllChunkCoords());
}

void URecallVoxelPhysicsSubsystem::Reset()
{
    ChunkBodies.Reset();
    ChunkDescriptors.Reset();
}

void URecallVoxelPhysicsSubsystem::Save(const FRecallSnapshotContext& Context,
                                         FInstancedStruct& OutSnapshot)
{
    OutSnapshot = FInstancedStruct::Make<FVoxelPhysicsSnapshot>();
	
	FVoxelPhysicsSnapshot& Snapshot = OutSnapshot.GetMutable<FVoxelPhysicsSnapshot>();
	Snapshot.ChunkBodies = ChunkBodies;
	Snapshot.ChunkDescriptors = ChunkDescriptors;
}

void URecallVoxelPhysicsSubsystem::Restore(const FRecallSnapshotContext& Context,
                                             const FInstancedStruct& InSnapshot)
{
    const FVoxelPhysicsSnapshot* Snapshot = InSnapshot.GetPtr<FVoxelPhysicsSnapshot>();
    if (!Snapshot)
    {
        return;
    }

    ChunkBodies = Snapshot->ChunkBodies;
    ChunkDescriptors = Snapshot->ChunkDescriptors;
}

void URecallVoxelPhysicsSubsystem::OnChunksBaked(const FVoxelGrid& Grid,
                                                 const TArray<FIntVector>& BakedCoords)
{
	TArray<FVoxelPhysicsChunkDescriptor> Descriptors;
	Descriptors.SetNum(BakedCoords.Num());
	
	ParallelFor(BakedCoords.Num(), [&Grid, &BakedCoords, &Descriptors](int32 ChunkIndex)
	{
		const FIntVector& ChunkCoord = BakedCoords[ChunkIndex];
		const FVoxelChunk* Chunk = Grid.QueryChunk(ChunkCoord);
		if (Chunk && Chunk->IsValid())
		{
			GenerateChunkDescription(*Chunk, Descriptors[ChunkIndex]);
		}
	});
	
    for (int32 ChunkIndex = 0; ChunkIndex < BakedCoords.Num(); ++ChunkIndex)
    {
    	const FIntVector& ChunkCoord = BakedCoords[ChunkIndex];
		DestroyChunkPhysics(ChunkCoord);
    	
    	const FVoxelPhysicsChunkDescriptor& Desc = Descriptors[ChunkIndex];
    	CreateChunkPhysics(ChunkCoord, Desc);
    }
}

void URecallVoxelPhysicsSubsystem::OnChunkEvicted(FIntVector ChunkCoord)
{
	DestroyChunkPhysics(ChunkCoord);                                                                                                                                       
	ChunkDescriptors.Remove(ChunkCoord);    
}

void URecallVoxelPhysicsSubsystem::GenerateChunkDescription(const FVoxelChunk& Chunk, FVoxelPhysicsChunkDescriptor& OutDesc)
{
	const FVoxelMeshData Mesh = FVoxelMarchingCubes::ExtractSurface(Chunk, /*LODStep=*/2);
	if (Mesh.Vertices.Num() < 3)
	{
		return;
	}

	const FVector ChunkOrigin = Chunk.ChunkOriginWorld();
	OutDesc.ChunkCoord  = Chunk.ChunkCoord;
	OutDesc.WorldOrigin = ChunkOrigin;
	OutDesc.Vertices    = Mesh.Vertices;
	OutDesc.Triangles   = Mesh.Triangles;
}

void URecallVoxelPhysicsSubsystem::CreateChunkPhysics(const FIntVector& ChunkCoord, const FVoxelPhysicsChunkDescriptor& Desc)
{
	if (Desc.Vertices.Num() == 0)
	{
		return;
	}
	
	const FRecallPhysicsMeshShape MeshShape(Desc.Vertices, Desc.Triangles);
	const FRecallPhysicsBodyHandle Handle = RecallPhysicsSystem->CreateMutableStaticShape(MeshShape, Desc.WorldOrigin, FQuat::Identity, 0.6f);
	if (Handle.IsValid())
	{
		checkf(!ChunkBodies.Contains(ChunkCoord), TEXT("Chunk already has a physics body"));
		ChunkBodies.Add(ChunkCoord, Handle);
		ChunkDescriptors.Add(ChunkCoord, Desc);
	}
}

void URecallVoxelPhysicsSubsystem::DestroyChunkPhysics(FIntVector ChunkCoord)
{
    FRecallPhysicsBodyHandle Handle;
    if (!ChunkBodies.RemoveAndCopyValue(ChunkCoord, Handle))
    {
        return;
    }

    if (RecallPhysicsSystem.IsValid())
    {
        RecallPhysicsSystem->ReleaseBody(Handle);
    }
}

void URecallVoxelPhysicsSubsystem::DestroyAllPhysics()
{
    if (RecallPhysicsSystem.IsValid())
    {
        for (auto& [Coord, Handle] : ChunkBodies)
        {
            RecallPhysicsSystem->ReleaseBody(Handle);
        }
    }
    ChunkBodies.Reset();
}
