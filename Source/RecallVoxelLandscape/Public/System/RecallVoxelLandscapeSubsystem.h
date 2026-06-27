// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Types/RecallVoxelLandscapeTypes.h"
#include "Containers/Queue.h"
#include "RecallVoxelLandscapeSubsystem.generated.h"

/**
 * Manages landscape cell snapshot/restore for the recall simulation.
 * Also exposes simulation entry points used by Mass processors so they
 * never reach into VoxelLandscape subsystems directly.
 *
 * Save/Restore serializes FVoxelLandscapeSnapshot (all non-zero cell records).
 */
UCLASS()
class RECALLVOXELLANDSCAPE_API URecallVoxelLandscapeSubsystem : public UWorldSubsystem,
    public IRecallSimulationReactSystemInterface
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // IRecallSimulationReactSystemInterface
    virtual void Start(const FRecallSimulationStartParams& Params) override;
    virtual void Reset() override;
    virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override;
    virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override;

    // Processor entry points
    void StartSimulation();
    void ForceEndSimulation();
    void DrawSurfaceDebug();

    // Fluid brush queuing (called by FluidSourceProcessor)
    void PushBrush(FName LayerName, FVector WorldPos, float Radius, float Value);
	void PushCapsuleBrush(FName LayerName, FVector Start, FVector End, float Radius, float Value, FFloatInterval ValueRange);
    void FlushBrushQueue();

private:
    TWeakObjectPtr<class UVoxelLandscapeSubsystem> LandscapeSystem;
    TWeakObjectPtr<class UVoxelSurfaceSubsystem>   SurfaceSystem;
    TQueue<FLandscapeBrushPaint>                   PendingBrushes;
	TQueue<FLandscapeCapsuleBrushPaint>              PendingCapsuleBrushes;
};

template<>
struct TMassExternalSubsystemTraits<URecallVoxelLandscapeSubsystem> final
{
    enum
    {
        GameThreadOnly = false
    };
};
