// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#include "Simulation/RecallVoxelLandscapeMaskProcessors.h"

#include "MassExecutionContext.h"
#include "MassEntityQuery.h"
#include "Simulation/RecallVoxelLandscapeMaskFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/RecallVoxelLandscapeSubsystem.h"

URecallVoxelLandscapeMaskProcessor::URecallVoxelLandscapeMaskProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

bool URecallVoxelLandscapeMaskProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return bRuntimeMode ? false : Super::ShouldAllowQueryBasedPruning(bRuntimeMode);
}

void URecallVoxelLandscapeMaskProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallVoxelLandscapeMaskFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRecallVoxelLandscapeMaskParams>();
	EntityQuery.AddSubsystemRequirement<URecallVoxelLandscapeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallVoxelLandscapeMaskProcessor::Execute(FMassEntityManager&, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& ExecutionContext)
	{
		const TConstArrayView<FRecallTransformFragment> Transforms = ExecutionContext.GetFragmentView<FRecallTransformFragment>();
		const TArrayView<FRecallVoxelLandscapeMaskFragment> Masks =
			ExecutionContext.GetMutableFragmentView<FRecallVoxelLandscapeMaskFragment>();
		const FRecallVoxelLandscapeMaskParams& Params =
			ExecutionContext.GetConstSharedFragment<FRecallVoxelLandscapeMaskParams>();
		URecallVoxelLandscapeSubsystem& Landscape =
			ExecutionContext.GetMutableSubsystemChecked<URecallVoxelLandscapeSubsystem>();

		for (int32 EntityIndex = 0; EntityIndex < ExecutionContext.GetNumEntities(); ++EntityIndex)
		{
			const FRecallTransformFragment& Transform = Transforms[EntityIndex];
			FRecallVoxelLandscapeMaskFragment& Mask = Masks[EntityIndex];
			Mask.TimeSinceLastApply += ExecutionContext.GetDeltaTimeSeconds();

			const bool bMoved = !Transform.Position.Equals(Mask.LastPosition, Params.MovementTolerance);
			const bool bIdleUpdateDue = Params.IdleUpdateInterval > 0.f && Mask.TimeSinceLastApply >= Params.IdleUpdateInterval;
			if (Mask.bHasApplied && !(Params.bApplyOnMovement && bMoved) && !bIdleUpdateDue)
			{
				continue;
			}

			const FVector Center = Transform.Position + Transform.Rotation.RotateVector(Params.Shape.Offset);
			const FVector Axis = Transform.Rotation.RotateVector(FVector::ForwardVector) * Params.Shape.HalfLength;
			Landscape.PushCapsuleBrush(Params.LayerName, Center - Axis, Center + Axis, Params.Shape.Radius,
				Params.Intensity, Params.ValueRange);
			Mask.LastPosition = Transform.Position;
			Mask.TimeSinceLastApply = 0.f;
			Mask.bHasApplied = true;
		}
	});
}
