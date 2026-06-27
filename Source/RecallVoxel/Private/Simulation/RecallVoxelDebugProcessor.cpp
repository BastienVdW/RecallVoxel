// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelDebugProcessor.h"

#include "MassExecutionContext.h"
#include "Grid/VoxelGrid.h"
#include "System/RecallVoxelSubsystem.h"
#include "Voxel/VoxelTypes.h"

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
#include "DrawDebugHelpers.h"

static int32 GShowVoxelDensity = 0;
static FAutoConsoleVariableRef CVarShowVoxelDensity(
	TEXT("recall.voxel.ShowVoxelDensity"),
	GShowVoxelDensity,
	TEXT("Draw per-voxel density values in world (non-empty chunks only). 0=off 1=on")
);

static int32 GShowChunkBounds = 0;
static FAutoConsoleVariableRef CVarShowChunkBounds(
	TEXT("recall.voxel.ShowChunkBounds"),
	GShowChunkBounds,
	TEXT("Draw a solid box around each chunk to visualise its bounds. 0=off 1=non-empty only 2=all chunks")
);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

URecallVoxelDebugProcessor::URecallVoxelDebugProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallVoxelDebugProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelSubsystem>(EMassFragmentAccess::ReadOnly);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}

bool URecallVoxelDebugProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	if (bRuntimeMode)
	{
		return false;
	}
	
	return Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelDebugProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	if (!GShowVoxelDensity && !GShowChunkBounds)
	{
		return;
	}

	const URecallVoxelSubsystem& VoxelSystem = Context.GetSubsystemChecked<URecallVoxelSubsystem>();
	const FVoxelGrid* Grid = VoxelSystem.GetGrid();
	if (!Grid)
	{
		return;
	}

	const UWorld* World = VoxelSystem.GetWorld();

	for (const FIntVector& Coord : Grid->GetAllChunkCoords())
	{
		const FVoxelChunk* Chunk = Grid->QueryChunk(Coord);
		if (!Chunk || !Chunk->IsValid())
		{
			continue;
		}

		const FVector CornerOrigin = Chunk->ChunkOriginWorld();
		const float VS = Chunk->VoxelSize;
		const int32 R  = Chunk->Resolution;
		const float ChunkWorldSize = R * VS;
		const FVector ChunkCenter = CornerOrigin + FVector(ChunkWorldSize * 0.5f);

		if (GShowChunkBounds)
		{
			const bool bShowThisChunk = (GShowChunkBounds == 2) || Chunk->bHasSolidVoxels;
			if (bShowThisChunk)
			{
				const FColor BoxColor = Chunk->bHasSolidVoxels ? FColor(255, 140, 0) : FColor(80, 80, 80);
				DrawDebugBox(World, ChunkCenter, FVector(ChunkWorldSize * 0.5f), BoxColor, false, 0.f, 0, 3.f);
			}
		}

		if (GShowVoxelDensity && Chunk->bHasSolidVoxels)
		{
			for (int32 Z = 0; Z < R; ++Z)
			for (int32 Y = 0; Y < R; ++Y)
			for (int32 X = 0; X < R; ++X)
			{
				const FVoxel& V = Chunk->At(X, Y, Z);
				if (V.Density <= 0.f) continue; // only draw solid voxels to reduce visual noise

				const FVector Pos = CornerOrigin + FVector(X + 0.5f, Y + 0.5f, Z + 0.5f) * VS;
				DrawDebugString(World, Pos,
					FString::Printf(TEXT("%.2f"), V.Density), nullptr, FColor::Green, 0.f);
			}
		}
	}
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}
