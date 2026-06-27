// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelLandscapeProcessors.h"

#include "MassExecutionContext.h"
#include "Simulation/RecallVoxelProcessorGroupTypes.h"
#include "System/RecallVoxelLandscapeSubsystem.h"

//----------------------------------------------------------------------//
// URecallVoxelLandscapeProcessor
//----------------------------------------------------------------------//
URecallVoxelLandscapeProcessor::URecallVoxelLandscapeProcessor()
{
	ExecutionFlags  = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	// Surface must have dispatched its column tasks first so landscape reads correct heights.
	ExecutionOrder.ExecuteInGroup = RecallVoxel::ProcessorGroupNames::VoxelLandscapeDispatch;
	ExecutionOrder.ExecuteBefore.Add(RecallVoxel::ProcessorGroupNames::VoxelSurfaceDispatch);
}

void URecallVoxelLandscapeProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& IE)
{
	Super::InitializeInternal(Owner, IE);
}

bool URecallVoxelLandscapeProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelLandscapeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelLandscapeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelLandscapeProcessor::Execute(FMassEntityManager&, FMassExecutionContext& Context)
{
	URecallVoxelLandscapeSubsystem& VoxelLandscapeSystem = Context.GetMutableSubsystemChecked<URecallVoxelLandscapeSubsystem>();
	VoxelLandscapeSystem.StartSimulation();
}

//----------------------------------------------------------------------//
// URecallVoxelLandscapeFlushProcessor
//----------------------------------------------------------------------//
URecallVoxelLandscapeFlushProcessor::URecallVoxelLandscapeFlushProcessor()
{
	ExecutionFlags  = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	ExecutionOrder.ExecuteInGroup    = RecallVoxel::ProcessorGroupNames::VoxelLandscapeFlush;
	ExecutionOrder.ExecuteBefore.Add(RecallVoxel::ProcessorGroupNames::VoxelSurfaceFlush);
}

void URecallVoxelLandscapeFlushProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& IE)
{
	Super::InitializeInternal(Owner, IE);
}

bool URecallVoxelLandscapeFlushProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelLandscapeFlushProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelLandscapeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelLandscapeFlushProcessor::Execute(FMassEntityManager&, FMassExecutionContext& Context)
{
	URecallVoxelLandscapeSubsystem& VoxelLandscapeSystem = Context.GetMutableSubsystemChecked<URecallVoxelLandscapeSubsystem>();
	VoxelLandscapeSystem.ForceEndSimulation();
}

//----------------------------------------------------------------------//
// URecallVoxelLandscapeBrushFlushProcessor
//----------------------------------------------------------------------//
URecallVoxelLandscapeBrushFlushProcessor::URecallVoxelLandscapeBrushFlushProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	ExecutionOrder.ExecuteAfter.Add(RecallVoxel::ProcessorGroupNames::VoxelSurfaceFlush);
	ExecutionOrder.ExecuteAfter.Add(RecallVoxel::ProcessorGroupNames::VoxelLandscapeFlush);
}

void URecallVoxelLandscapeBrushFlushProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& IE)
{
	Super::InitializeInternal(Owner, IE);
}

bool URecallVoxelLandscapeBrushFlushProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelLandscapeBrushFlushProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelLandscapeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelLandscapeBrushFlushProcessor::Execute(FMassEntityManager&, FMassExecutionContext& Context)
{
	URecallVoxelLandscapeSubsystem& RecallLandscapeSystem = Context.GetMutableSubsystemChecked<URecallVoxelLandscapeSubsystem>();
	RecallLandscapeSystem.FlushBrushQueue();
}

//----------------------------------------------------------------------//
// URecallVoxelLandscapeDebugRepresentationProcessor
//----------------------------------------------------------------------//
URecallVoxelLandscapeDebugRepresentationProcessor::URecallVoxelLandscapeDebugRepresentationProcessor()
{
	ExecutionFlags  = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallVoxelLandscapeDebugRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& IE)
{
	Super::InitializeInternal(Owner, IE);
}

bool URecallVoxelLandscapeDebugRepresentationProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelLandscapeDebugRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallVoxelLandscapeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelLandscapeDebugRepresentationProcessor::Execute(FMassEntityManager&, FMassExecutionContext& Context)
{
	URecallVoxelLandscapeSubsystem& VoxelLandscapeSystem = Context.GetMutableSubsystemChecked<URecallVoxelLandscapeSubsystem>();
	VoxelLandscapeSystem.DrawSurfaceDebug();
}
