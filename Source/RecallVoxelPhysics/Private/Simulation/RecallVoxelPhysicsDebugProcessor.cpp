// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelPhysicsDebugProcessor.h"

#include "MassExecutionContext.h"
#include "Physics/JPRPhysicsBody.h"
#include "System/RecallVoxelPhysicsSubsystem.h"
#include "System/RecallVoxelViewSubsystem.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "Physics/RecallPhysicsObjects.h"

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
#include "DrawDebugHelpers.h"

static int32 GShowPhysicsBodies = 0;
static FAutoConsoleVariableRef CVarShowPhysicsBodies(
	TEXT("recall.voxel.ShowPhysicsBodies"),
	GShowPhysicsBodies,
	TEXT("Draw the shape of each active voxel physics body near a view. 0=off 1=on")
);

static float GShowPhysicsBodiesRange = 500.f;
static FAutoConsoleVariableRef CVarShowPhysicsBodiesRange(
	TEXT("recall.voxel.ShowPhysicsBodiesRange"),
	GShowPhysicsBodiesRange,
	TEXT("Range in cm within which voxel physics bodies are drawn (default 500)")
);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

URecallVoxelPhysicsDebugProcessor::URecallVoxelPhysicsDebugProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallVoxelPhysicsDebugProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelPhysicsSubsystem>(EMassFragmentAccess::ReadOnly);
	ProcessorRequirements.AddSubsystemRequirement<URecallPhysicsSubsystem>(EMassFragmentAccess::ReadOnly);
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelViewSubsystem>(EMassFragmentAccess::ReadOnly);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}

bool URecallVoxelPhysicsDebugProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	if (bRuntimeMode)
	{
		return false;
	}
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	return Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelPhysicsDebugProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	if (!GShowPhysicsBodies)
	{
		return;
	}

	const URecallVoxelPhysicsSubsystem& VoxelPhysicsSystem = Context.GetSubsystemChecked<URecallVoxelPhysicsSubsystem>();
	const URecallPhysicsSubsystem& PhysicsSystem = Context.GetSubsystemChecked<URecallPhysicsSubsystem>();
	const URecallVoxelViewSubsystem& ViewSubsystem = Context.GetSubsystemChecked<URecallVoxelViewSubsystem>();
	const UWorld* World = VoxelPhysicsSystem.GetWorld();

	const TMap<uint32, FVoxelView>& Views = ViewSubsystem.GetViews();
	const float RangeSq = FMath::Square(GShowPhysicsBodiesRange);

	for (const auto& [ChunkCoord, Handle] : VoxelPhysicsSystem.DebugGetChunkBodies())
	{
		const FConstRecallPhysicsBodyView Body = PhysicsSystem.GetBody(Handle);
		const TSharedPtr<const FJPRPhysicsBody> PinnedBody = Body.Pin();
		if (!PinnedBody.IsValid())
		{
			continue;
		}

		FVector BodyPosition;
		PinnedBody->GetPosition(BodyPosition);

		bool bInRange = false;
		for (const auto& [ViewId, View] : Views)
		{
			if (FVector::DistSquared(BodyPosition, View.Position) <= RangeSq)
			{
				bInRange = true;
				break;
			}
		}

		if (bInRange)
		{
			PinnedBody->DrawDebugShape(World, FColor::Cyan);
		}
	}
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}
