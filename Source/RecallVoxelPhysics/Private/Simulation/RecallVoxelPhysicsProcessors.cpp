// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "RecallVoxelPhysicsProcessors.h"

#include "MassExecutionContext.h"
#include "Simulation/RecallVoxelProcessorGroupTypes.h"
#include "System/RecallVoxelSubsystem.h"
#include "System/RecallVoxelPhysicsSubsystem.h"

//----------------------------------------------------------------------//
// URecallVoxelPhysicsFlushProcessor
//----------------------------------------------------------------------//

URecallVoxelPhysicsFlushProcessor::URecallVoxelPhysicsFlushProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	ExecutionOrder.ExecuteInGroup = RecallVoxel::ProcessorGroupNames::VoxelPhysicsFlush;
	ExecutionOrder.ExecuteAfter.Add(RecallVoxel::ProcessorGroupNames::VoxelFlush);
	ExecutionOrder.ExecuteBefore.Add(RecallVoxel::ProcessorGroupNames::VoxelModifierFlush);
}

void URecallVoxelPhysicsFlushProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

bool URecallVoxelPhysicsFlushProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	if (bRuntimeMode)
	{
		return false;
	}
	
	return Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelPhysicsFlushProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelSubsystem>(EMassFragmentAccess::ReadOnly);
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelPhysicsSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelPhysicsFlushProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const URecallVoxelSubsystem&  VoxelSystem   = Context.GetSubsystemChecked<URecallVoxelSubsystem>();
	if (!VoxelSystem.GetGrid())
	{
		return;
	}

	URecallVoxelPhysicsSubsystem& VoxelPhysics = Context.GetMutableSubsystemChecked<URecallVoxelPhysicsSubsystem>();
	VoxelPhysics.OnChunksBaked(*VoxelSystem.GetGrid(), VoxelSystem.GetLastBakedCoords());
}
