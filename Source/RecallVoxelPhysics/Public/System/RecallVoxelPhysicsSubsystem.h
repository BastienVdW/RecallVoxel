// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Physics/RecallPhysicsBodyHandle.h"
#include "Types/RecallVoxelTypes.h"
#include "RecallVoxelPhysicsSubsystem.generated.h"

class FVoxelGrid;
struct FVoxelChunk;

/**
 * Manages Jolt physics bodies for all voxel terrain chunks.
 *
 * Lifecycle:
 *  - OnChunksBaked: called by URecallVoxelFlushProcessor after generation completes
 *  - OnChunkEvicted: subscribed to UVoxelStreamingSubsystem::OnChunkEvicted
 *
 * Save/Restore serializes FVoxelPhysicsSnapshot (all chunk mesh descriptors).
 * Per-chunk body handles are tracked in ChunkBodies and released on evict.
 */
UCLASS()
class RECALLVOXELPHYSICS_API URecallVoxelPhysicsSubsystem : public UWorldSubsystem,
    public IRecallSimulationReactSystemInterface
{
    GENERATED_BODY()

public:
    // UWorldSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Called by URecallVoxelFlushProcessor after ForceEndGeneration
    void OnChunksBaked(const FVoxelGrid& Grid, const TArray<FIntVector>& BakedCoords);

    // Bound to UVoxelStreamingSubsystem::OnChunkEvicted
    void OnChunkEvicted(FIntVector ChunkCoord);

    // IRecallSimulationReactSystemInterface
	virtual void Start(const FRecallSimulationStartParams& Params) override;
    virtual void Reset() override;
    virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override;
    virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT                                                                                                                                   
	const TMap<FIntVector, FJPRPhysicsBodyHandle>& DebugGetChunkBodies() const { return ChunkBodies; }                                                                     
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT                                                                                                                                                                        
     
private:
	static void GenerateChunkDescription(const FVoxelChunk& Chunk, FVoxelPhysicsChunkDescriptor& OutDesc);
	void CreateChunkPhysics(const FIntVector& ChunkCoord, const FVoxelPhysicsChunkDescriptor& Desc);
    void DestroyChunkPhysics(FIntVector ChunkCoord);
    void DestroyAllPhysics();

    // Per-chunk Jolt body handles, populated when chunks are baked and cleared on eviction or reset.
    TMap<FIntVector, FJPRPhysicsBodyHandle> ChunkBodies;

    // Mesh descriptors — populated on each bake, used for Save and PostRestore
    TMap<FIntVector, FVoxelPhysicsChunkDescriptor> ChunkDescriptors;

    TWeakObjectPtr<class URecallPhysicsSubsystem>   RecallPhysicsSystem;
    TWeakObjectPtr<class UVoxelStreamingSubsystem>  VoxelStreamingSystem;

    FDelegateHandle EvictedHandle;
};

template<>
struct TMassExternalSubsystemTraits<URecallVoxelPhysicsSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
