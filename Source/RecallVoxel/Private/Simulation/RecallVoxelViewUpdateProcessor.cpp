// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelViewUpdateProcessor.h"

#include "MassExecutionContext.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "Streaming/VoxelView.h"
#include "System/RecallVoxelViewSubsystem.h"

//----------------------------------------------------------------------//
// URecallVoxelViewObserverProcessor
//----------------------------------------------------------------------//
URecallVoxelViewObserverProcessor::URecallVoxelViewObserverProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallControllerFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void URecallVoxelViewObserverProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallVoxelViewObserverProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallVoxelViewSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelViewObserverProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallVoxelViewSubsystem& ViewSystem = Context.GetMutableSubsystemChecked<URecallVoxelViewSubsystem>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const uint32 Serial = Context.GetEntity(EntityIndex).SerialNumber;
			ViewSystem.RegisterView(Serial, FVoxelView{});
		}
	});
}

//----------------------------------------------------------------------//
// URecallVoxelViewDeinitializer
//----------------------------------------------------------------------//
URecallVoxelViewDeinitializer::URecallVoxelViewDeinitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallControllerFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Remove;
}

void URecallVoxelViewDeinitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallVoxelViewDeinitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallVoxelViewSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelViewDeinitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallVoxelViewSubsystem& ViewSystem = Context.GetMutableSubsystemChecked<URecallVoxelViewSubsystem>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const uint32 Serial = Context.GetEntity(EntityIndex).SerialNumber;
			ViewSystem.UnregisterView(Serial);
		}
	});
}

//----------------------------------------------------------------------//
// URecallVoxelViewUpdateProcessor
//----------------------------------------------------------------------//
URecallVoxelViewUpdateProcessor::URecallVoxelViewUpdateProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void URecallVoxelViewUpdateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallVoxelViewSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelViewUpdateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallVoxelViewSubsystem& ViewSystem = Context.GetMutableSubsystemChecked<URecallVoxelViewSubsystem>();
		TConstArrayView<FRecallTransformFragment> Transforms = Context.GetFragmentView<FRecallTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const uint32 Serial = Context.GetEntity(EntityIndex).SerialNumber;
			ViewSystem.UpdateViewPosition(Serial, Transforms[EntityIndex].Position);
		}
	});
}
