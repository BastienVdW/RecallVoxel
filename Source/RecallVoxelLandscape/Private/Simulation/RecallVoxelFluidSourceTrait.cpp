// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Simulation/RecallVoxelFluidSourceTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "Simulation/RecallVoxelFluidSourceFragments.h"
#include "Settings/VoxelDeveloperSettings.h"

void URecallVoxelFluidSourceTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	BuildContext.RequireFragment<FRecallTransformFragment>();
	BuildContext.AddFragment<FRecallVoxelFluidSourceFragment>();

	FRecallVoxelFluidSourceParams Params;
	Params.LayerName = LayerName;
	Params.EmissionRate = EmissionRate;
	Params.BurstAmount = BurstAmount;
	Params.MaxEmission = MaxEmission;
	Params.Radius = Radius;
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(Params));
}

TArray<FName> URecallVoxelFluidSourceTrait::GetFluidLayerOptions() const
{
	TArray<FName> Options;

	const UVoxelDeveloperSettings* Settings = GetDefault<UVoxelDeveloperSettings>();
	for (const FLandscapeLayerConfig& Layer : Settings->LandscapeLayers)
	{
		if (Layer.bIsFluid)
		{
			Options.Add(Layer.LayerName);
		}
	}

	return Options;
}
