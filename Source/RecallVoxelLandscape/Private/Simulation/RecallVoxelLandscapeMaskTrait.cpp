// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#include "Simulation/RecallVoxelLandscapeMaskTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "Settings/VoxelDeveloperSettings.h"
#include "Simulation/Transform/RecallTransformFragments.h"

void URecallVoxelLandscapeMaskTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	BuildContext.RequireFragment<FRecallTransformFragment>();
	BuildContext.AddFragment<FRecallVoxelLandscapeMaskFragment>();

	FRecallVoxelLandscapeMaskParams Params;
	Params.LayerName = LayerName;
	Params.Shape = Shape;
	Params.Shape.Radius = FMath::Max(Shape.Radius, 0.f);
	Params.Shape.HalfLength = FMath::Max(Shape.HalfLength, 0.f);
	Params.bApplyOnMovement = bApplyOnMovement;
	Params.MovementTolerance = FMath::Max(MovementTolerance, 0.f);
	Params.IdleUpdateInterval = IdleUpdateInterval;
	Params.Intensity = Intensity;
	Params.ValueRange = ValueRange;
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(Params));
}

TArray<FName> URecallVoxelLandscapeMaskTrait::GetLandscapeLayerOptions() const
{
	TArray<FName> Options;
	for (const FLandscapeLayerConfig& Layer : GetDefault<UVoxelDeveloperSettings>()->LandscapeLayers)
	{
		Options.Add(Layer.LayerName);
	}
	return Options;
}
