// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"

#include "RecallVoxelPhysicsDebugProcessor.generated.h"

/** Render phase: draws a box around each active voxel physics body when recall.voxel.ShowPhysicsBodies is enabled. */
UCLASS()
class URecallVoxelPhysicsDebugProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelPhysicsDebugProcessor();

public:
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
