// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"

#include "RecallVoxelPhysicsProcessors.generated.h"

/**
 * FrameEnd: runs after URecallVoxelFlushProcessor.
 * Reads freshly baked chunk coords from URecallVoxelSubsystem and
 * rebuilds Jolt convex bodies in URecallVoxelPhysicsSubsystem.
 */
UCLASS()
class URecallVoxelPhysicsFlushProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelPhysicsFlushProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
