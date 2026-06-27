// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Ability/RecallAbilityCommandTypes.h"
#include "Modifier/VoxelModifierTypes.h"

#include "RecallAbilityVoxelCommandTypes.generated.h"

USTRUCT(DisplayName="Voxel Sphere")
struct RECALLVOXELABILITY_API FRecallAbilityVoxelSphereCommand : public FRecallAbilityCommand
{
    GENERATED_BODY()

    virtual void Execute(const FRecallAbilityExecutionContext& Context, ERecallAbilityExecutionEvent Event) const override;

protected:
    UPROPERTY(EditAnywhere)
    FInt32Range FrameRange{ 0, 1 };

    UPROPERTY(EditAnywhere, meta=(Units=Centimeters, ClampMin="1.0"))
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere)
    FVector Offset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    EModifierOp Operation = EModifierOp::Remove;

    UPROPERTY(EditAnywhere)
    uint8 SurfaceType = 0;
};
