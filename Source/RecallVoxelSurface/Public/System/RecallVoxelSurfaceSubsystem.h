// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Util/RecallUndoQueue.h"
#include "VoxelSurfaceTypes.h"

#include "RecallVoxelSurfaceSubsystem.generated.h"

/**
 * Snapshots and restores UVoxelSurfaceSubsystem's column data (FVoxelSurfaceChunk map)
 * at each recall keyframe so that URecallVoxelLandscapeSubsystem reads coherent surface
 * heights after a rollback.
 *
 * The rendered mesh is left intentionally stale for one frame after restore — the
 * existing async surface render path will regenerate it as normal dirty-chunk updates.
 */
UCLASS()
class RECALLVOXELSURFACE_API URecallVoxelSurfaceSubsystem : public UWorldSubsystem,
                                                             public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	void StartSurfaceGeneration();
	void ForceEndSurfaceGeneration();
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// IRecallSimulationReactSystemInterface
	virtual void Start(const FRecallSimulationStartParams& Params) override;
	virtual void Reset() override;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override;

private:
	TWeakObjectPtr<class UVoxelStreamingSubsystem> StreamingSystem;
	TWeakObjectPtr<class UVoxelSurfaceSubsystem> SurfaceSystem;

	TRecallUndoQueue<FIntVector2, FVoxelSurfaceChunk> CachedUndoQueue;

	// Populated in StartSurfaceGeneration; consumed in ForceEndSurfaceGeneration to push a diff.
	TMap<FIntVector2, FVoxelSurfaceChunk> PendingPreState;
	TArray<FIntVector2> PendingDirtyCoords;
};

template<>
struct TMassExternalSubsystemTraits<URecallVoxelSurfaceSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
