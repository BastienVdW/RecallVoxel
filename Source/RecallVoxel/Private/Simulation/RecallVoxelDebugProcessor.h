// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"

#include "RecallVoxelDebugProcessor.generated.h"

/** Render phase: draws per-voxel density values for all non-empty chunks when recall.voxel.ShowVoxelDensity is enabled. */
UCLASS()
class URecallVoxelDebugProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelDebugProcessor();

public:
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
