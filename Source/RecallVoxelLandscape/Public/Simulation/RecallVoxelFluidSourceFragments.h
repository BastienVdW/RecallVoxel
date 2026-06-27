// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Mass/EntityElementTypes.h"
#include "RecallVoxelFluidSourceFragments.generated.h"

USTRUCT()
struct FRecallVoxelFluidSourceFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	float EmittedAmount = 0.0f;

	UPROPERTY()
	bool bHasBurst = false;
};

USTRUCT()
struct FRecallVoxelFluidSourceParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FName LayerName;
	
	UPROPERTY()
	float EmissionRate = 1.0f;

	UPROPERTY()
	float BurstAmount = 0.0f;

	// Zero means that emission is unlimited.
	UPROPERTY()
	float MaxEmission = 0.0f;
	
	UPROPERTY()
	float Radius = 100.0f;
};
