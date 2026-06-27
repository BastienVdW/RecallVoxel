// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelFluidSourceProcessors.h"

#include "MassExecutionContext.h"
#include "MassEntityQuery.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "Simulation/RecallVoxelFluidSourceFragments.h"
#include "Simulation/RecallVoxelProcessorGroupTypes.h"
#include "System/RecallVoxelLandscapeSubsystem.h"

//----------------------------------------------------------------------//
// URecallFluidSourceProcessor
//----------------------------------------------------------------------//
URecallFluidSourceProcessor::URecallFluidSourceProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void URecallFluidSourceProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& IE)
{
	Super::InitializeInternal(Owner, IE);
}

bool URecallFluidSourceProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallFluidSourceProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallVoxelFluidSourceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRecallVoxelFluidSourceParams>();
	EntityQuery.AddSubsystemRequirement<URecallVoxelLandscapeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallFluidSourceProcessor::Execute(FMassEntityManager&, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& ExecutionContext)
	{
		const TConstArrayView<FRecallTransformFragment> TransformFragments = ExecutionContext.GetFragmentView<FRecallTransformFragment>();
		const TArrayView<FRecallVoxelFluidSourceFragment> FluidSourceFragments = ExecutionContext.GetMutableFragmentView<FRecallVoxelFluidSourceFragment>();
		const FRecallVoxelFluidSourceParams& FluidSourceParams = ExecutionContext.GetConstSharedFragment<FRecallVoxelFluidSourceParams>();

		URecallVoxelLandscapeSubsystem& RecallLandscapeSystem = ExecutionContext.GetMutableSubsystemChecked<URecallVoxelLandscapeSubsystem>();

		for (int32 EntityIndex = 0; EntityIndex < ExecutionContext.GetNumEntities(); ++EntityIndex)
		{
			const FRecallTransformFragment& Transform = TransformFragments[EntityIndex];
			FRecallVoxelFluidSourceFragment& FluidSource = FluidSourceFragments[EntityIndex];

			const float DeltaTime = ExecutionContext.GetDeltaTimeSeconds();
			float Value = FluidSourceParams.EmissionRate * DeltaTime;
			if (!FluidSource.bHasBurst)
			{
				Value += FluidSourceParams.BurstAmount;
				FluidSource.bHasBurst = true;
			}

			if (FluidSourceParams.MaxEmission > 0.0f)
			{
				Value = FMath::Min(Value, FMath::Max(FluidSourceParams.MaxEmission - FluidSource.EmittedAmount, 0.0f));
			}

			if (Value <= 0.0f)
			{
				continue;
			}

			RecallLandscapeSystem.PushBrush(FluidSourceParams.LayerName, Transform.Position, FluidSourceParams.Radius, Value);
			FluidSource.EmittedAmount += Value;
		}
	});
}
