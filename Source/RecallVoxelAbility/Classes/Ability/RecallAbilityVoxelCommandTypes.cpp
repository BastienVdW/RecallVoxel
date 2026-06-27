// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "RecallAbilityVoxelCommandTypes.h"

#include "Ability/RecallAbilityExecutionTypes.h"
#include "Engine/World.h"
#include "MassEntityView.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/RecallVoxelSubsystem.h"

void FRecallAbilityVoxelSphereCommand::Execute(const FRecallAbilityExecutionContext& Context,
    ERecallAbilityExecutionEvent Event) const
{
    if (Event != ERecallAbilityExecutionEvent::OnTick)
    {
        return;
    }

    const bool bInRange = FrameRange.Contains(Context.AbilityFragment.ElapsedFrames);

    // Not in range — nothing to do
    if (!bInRange)
    {
        return;
    }

    // Entering frame range — build modifier and add
    const FMassEntityView EntityView = Context.GetEntityView();
    const FRecallTransformFragment& TransformFragment = EntityView.GetFragmentData<FRecallTransformFragment>();
    const FTransform& EntityTransform = TransformFragment.GetTransform();

    const FVector WorldOffset  = EntityTransform.TransformVector(Offset);
    const FVector SphereOrigin = EntityTransform.GetLocation() + WorldOffset;

    FVoxelModifierData ModifierData;
    ModifierData.Params.Type        = EModifierType::PrimitiveSphere;
    ModifierData.Params.Operation   = Operation;
    ModifierData.Params.SurfaceType = SurfaceType;
    // Sphere radius stored in Scale.X per FVoxelModifierData convention
    ModifierData.Transform = FTransform(FQuat::Identity, SphereOrigin, FVector(Radius));

    URecallVoxelSubsystem* VoxelSystem = UWorld::GetSubsystem<URecallVoxelSubsystem>(Context.GetWorld());
    if (!ensureAlwaysMsgf(VoxelSystem, TEXT("FRecallAbilityVoxelSphereCommand: URecallVoxelSubsystem not found")))
    {
        return;
    }

    VoxelSystem->AddDynamicModifier(ModifierData);
}
