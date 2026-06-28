// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelProcessors.h"

#include "MassExecutionContext.h"
#include "Simulation/RecallVoxelProcessorGroupTypes.h"
#include "System/RecallVoxelSubsystem.h"

//----------------------------------------------------------------------//
// URecallVoxelStreamingProcessor
//----------------------------------------------------------------------//
URecallVoxelStreamingProcessor::URecallVoxelStreamingProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = RecallVoxel::ProcessorGroupNames::VoxelGenerationDispatch;
}

void URecallVoxelStreamingProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

bool URecallVoxelStreamingProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	if (bRuntimeMode)
	{
		return false;
	}
	
	return Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelStreamingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelStreamingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	URecallVoxelSubsystem& VoxelSystem = Context.GetMutableSubsystemChecked<URecallVoxelSubsystem>();
	VoxelSystem.StartVoxelGeneration(Context.GetDeltaTimeSeconds());
}

//----------------------------------------------------------------------//
// URecallVoxelFlushProcessor
//----------------------------------------------------------------------//
URecallVoxelFlushProcessor::URecallVoxelFlushProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	ExecutionOrder.ExecuteInGroup = RecallVoxel::ProcessorGroupNames::VoxelFlush;
}

void URecallVoxelFlushProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

bool URecallVoxelFlushProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	if (bRuntimeMode)
	{
		return false;
	}
	
	return Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelFlushProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelFlushProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	URecallVoxelSubsystem& VoxelSystem = Context.GetMutableSubsystemChecked<URecallVoxelSubsystem>();
	VoxelSystem.ForceEndVoxelGeneration();
}

//----------------------------------------------------------------------//
// URecallVoxelModifierFlushProcessor
//----------------------------------------------------------------------//
URecallVoxelModifierFlushProcessor::URecallVoxelModifierFlushProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	ExecutionOrder.ExecuteInGroup = RecallVoxel::ProcessorGroupNames::VoxelModifierFlush;
}

void URecallVoxelModifierFlushProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

bool URecallVoxelModifierFlushProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	if (bRuntimeMode)
	{
		return false;
	}
	
	return Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelModifierFlushProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelModifierFlushProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Apply queued modifier commands now that async generation has finished
	URecallVoxelSubsystem& VoxelSystem = Context.GetMutableSubsystemChecked<URecallVoxelSubsystem>();
	VoxelSystem.FlushModifierCommands();
}
