// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "System/RecallVoxelLandscapeSubsystem.h"

#include "Engine/World.h"
#include "StructUtils/InstancedStruct.h"
#include "System/VoxelLandscapeSubsystem.h"
#include "System/VoxelSurfaceSubsystem.h"

void URecallVoxelLandscapeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Collection.InitializeDependency<UVoxelLandscapeSubsystem>();
    LandscapeSystem = UWorld::GetSubsystem<UVoxelLandscapeSubsystem>(GetWorld());

    Collection.InitializeDependency<UVoxelSurfaceSubsystem>();
    SurfaceSystem = UWorld::GetSubsystem<UVoxelSurfaceSubsystem>(GetWorld());
}

void URecallVoxelLandscapeSubsystem::Start(const FRecallSimulationStartParams& Params)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelLandscapeSubsystem::Start"));
    // Landscape regenerates from the modifier grid — no action needed on start.
}

void URecallVoxelLandscapeSubsystem::Reset()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelLandscapeSubsystem::Reset"));
	
    if (LandscapeSystem.IsValid())
    {
        LandscapeSystem->ClearAllCells();
    }
}

void URecallVoxelLandscapeSubsystem::Save(const FRecallSnapshotContext& Context,
                                           FInstancedStruct& OutSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelLandscapeSubsystem::Save"));
	
	checkf(PendingBrushes.IsEmpty() && PendingCapsuleBrushes.IsEmpty(), TEXT("Brush queue should be flushed before saving snapshot!"));

    OutSnapshot.InitializeAs<FVoxelLandscapeSnapshot>();
	
    if (LandscapeSystem.IsValid())
    {
    	checkf(LandscapeSystem->IsStateless(), TEXT("Landscape system should be stateless before saving snapshot!"));

    	FVoxelLandscapeSnapshot& Snap = OutSnapshot.GetMutable<FVoxelLandscapeSnapshot>();
    	Snap.GridData = LandscapeSystem->GetGridData();
    }
}

void URecallVoxelLandscapeSubsystem::Restore(const FRecallSnapshotContext& Context,
                                              const FInstancedStruct& InSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallVoxelLandscapeSubsystem::Restore"));
	
	const FVoxelLandscapeSnapshot* Snap = InSnapshot.GetPtr<FVoxelLandscapeSnapshot>();
	if (!Snap)
	{
		return;
	}
	
    if (LandscapeSystem.IsValid())
    {
		LandscapeSystem->SetGridData(Snap->GridData);
    }
}

void URecallVoxelLandscapeSubsystem::StartSimulation()
{
	if (LandscapeSystem.IsValid())
	{
		LandscapeSystem->StartSimulation();
	}
}

void URecallVoxelLandscapeSubsystem::ForceEndSimulation()
{
	if (LandscapeSystem.IsValid())
	{
		LandscapeSystem->ForceEndSimulation();
	}
}

void URecallVoxelLandscapeSubsystem::DrawSurfaceDebug()
{
	if (SurfaceSystem.IsValid())
	{
		SurfaceSystem->DrawDebug();
	}
}

void URecallVoxelLandscapeSubsystem::PushBrush(FName LayerName, FVector WorldPos, float Radius, float Value)
{
	FLandscapeBrushPaint Brush;
	Brush.LayerName = LayerName;
	Brush.WorldPos = WorldPos;
	Brush.Radius = Radius;
	Brush.Value = Value;
	PendingBrushes.Enqueue(Brush);
}

void URecallVoxelLandscapeSubsystem::PushCapsuleBrush(
	FName LayerName, FVector Start, FVector End, float Radius, float Value, FFloatInterval ValueRange)
{
	FLandscapeCapsuleBrushPaint Brush;
	Brush.LayerName = LayerName;
	Brush.Start = Start;
	Brush.End = End;
	Brush.Radius = Radius;
	Brush.Value = Value;
	Brush.ValueRange = ValueRange;
	PendingCapsuleBrushes.Enqueue(Brush);
}

void URecallVoxelLandscapeSubsystem::FlushBrushQueue()
{
	if (!LandscapeSystem.IsValid())
	{
		PendingBrushes.Empty();
		PendingCapsuleBrushes.Empty();
		return;
	}

	FLandscapeBrushPaint Brush;
	while (PendingBrushes.Dequeue(Brush))
	{
		LandscapeSystem->ApplyBrush(Brush.LayerName, Brush.WorldPos, Brush.Radius, Brush.Value);
	}

	FLandscapeCapsuleBrushPaint CapsuleBrush;
	while (PendingCapsuleBrushes.Dequeue(CapsuleBrush))
	{
		LandscapeSystem->ApplyCapsuleBrush(CapsuleBrush.LayerName, CapsuleBrush.Start, CapsuleBrush.End,
			CapsuleBrush.Radius, CapsuleBrush.Value, CapsuleBrush.ValueRange);
	}
}
