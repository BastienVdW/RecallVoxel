// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
#pragma once

#include "CoreMinimal.h"
#include "VoxelLandscapeTypes.h"
#include "RecallVoxelLandscapeTypes.generated.h"

USTRUCT()
struct RECALLVOXELLANDSCAPE_API FVoxelLandscapeSnapshot
{
    GENERATED_BODY()

	UPROPERTY()
	FVoxelLandscapeGridData GridData;
};

USTRUCT()
struct RECALLVOXELLANDSCAPE_API FLandscapeBrushPaint
{
    GENERATED_BODY()

    UPROPERTY() FName LayerName;
    UPROPERTY() FVector WorldPos = FVector::ZeroVector;
    UPROPERTY() float Radius = 0.f;
    UPROPERTY() float Value = 0.f;
};

USTRUCT()
struct RECALLVOXELLANDSCAPE_API FLandscapeCapsuleBrushPaint
{
	GENERATED_BODY()

	UPROPERTY() FName LayerName;
	UPROPERTY() FVector Start = FVector::ZeroVector;
	UPROPERTY() FVector End = FVector::ZeroVector;
	UPROPERTY() float Radius = 0.f;
	UPROPERTY() float Value = 0.f;
	UPROPERTY() FFloatInterval ValueRange = FFloatInterval(0.f, TNumericLimits<float>::Max());
};
