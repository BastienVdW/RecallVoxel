// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityElementTypes.h"
#include "RecallVoxelLandscapeMaskFragments.generated.h"

USTRUCT(BlueprintType)
struct FRecallVoxelLandscapeMaskCapsule
{
	GENERATED_BODY()

	/** Local-space center offset. */
	UPROPERTY(EditAnywhere, Category = "Landscape Mask")
	FVector Offset = FVector::ZeroVector;

	/** Radius of the capsule footprint. */
	UPROPERTY(EditAnywhere, Category = "Landscape Mask", meta = (ClampMin = "0.0"))
	float Radius = 50.f;

	/** Half-length of the capsule's local X axis, excluding its end caps. */
	UPROPERTY(EditAnywhere, Category = "Landscape Mask", meta = (ClampMin = "0.0"))
	float HalfLength = 50.f;
};

USTRUCT()
struct FRecallVoxelLandscapeMaskFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FVector LastPosition = FVector::ZeroVector;

	UPROPERTY()
	float TimeSinceLastApply = 0.f;

	UPROPERTY()
	bool bHasApplied = false;
};

USTRUCT()
struct FRecallVoxelLandscapeMaskParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FName LayerName;

	UPROPERTY()
	FRecallVoxelLandscapeMaskCapsule Shape;

	UPROPERTY()
	bool bApplyOnMovement = true;

	UPROPERTY()
	float MovementTolerance = 1.f;

	/** Reapply interval while stationary. Zero disables idle reapplication. */
	UPROPERTY()
	float IdleUpdateInterval = 1.f;

	UPROPERTY()
	float Intensity = -100.f;

	UPROPERTY()
	FFloatInterval ValueRange = FFloatInterval(5.f, TNumericLimits<float>::Max());
};
