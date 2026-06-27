// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelSurfaceProcessors.h"

#include "MassExecutionContext.h"
#include "Simulation/RecallVoxelProcessorGroupTypes.h"
#include "System/RecallVoxelSurfaceSubsystem.h"

//----------------------------------------------------------------------//
// URecallVoxelSurfaceProcessor
//----------------------------------------------------------------------//
URecallVoxelSurfaceProcessor::URecallVoxelSurfaceProcessor()
{
	ExecutionFlags  = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = RecallVoxel::ProcessorGroupNames::VoxelSurfaceDispatch;
	ExecutionOrder.ExecuteBefore.Add(RecallVoxel::ProcessorGroupNames::VoxelGenerationDispatch);
}

void URecallVoxelSurfaceProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

bool URecallVoxelSurfaceProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelSurfaceProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelSurfaceSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelSurfaceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	URecallVoxelSurfaceSubsystem& VoxelSurfaceSystem = Context.GetMutableSubsystemChecked<URecallVoxelSurfaceSubsystem>();
	VoxelSurfaceSystem.StartSurfaceGeneration();
}

//----------------------------------------------------------------------//
// URecallVoxelSurfaceFlushProcessor
//----------------------------------------------------------------------//
URecallVoxelSurfaceFlushProcessor::URecallVoxelSurfaceFlushProcessor()
{
	ExecutionFlags  = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	ExecutionOrder.ExecuteInGroup = RecallVoxel::ProcessorGroupNames::VoxelSurfaceFlush;
	ExecutionOrder.ExecuteAfter.Add(RecallVoxel::ProcessorGroupNames::VoxelFlush);
}

void URecallVoxelSurfaceFlushProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

bool URecallVoxelSurfaceFlushProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelSurfaceFlushProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelSurfaceSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelSurfaceFlushProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	URecallVoxelSurfaceSubsystem& VoxelSurfaceSystem = Context.GetMutableSubsystemChecked<URecallVoxelSurfaceSubsystem>();
	VoxelSurfaceSystem.ForceEndSurfaceGeneration();
}
