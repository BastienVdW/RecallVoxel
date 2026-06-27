// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"

#include "RecallVoxelSurfaceProcessors.generated.h"

/**
 * PrePhysics (VoxelSurfaceDispatch): dispatches surface column tasks for dirty XY chunks.
 * Runs before URecallVoxelLandscapeProcessor (VoxelLandscapeDispatch) so landscape
 * simulation always reads freshly committed surface data.
 */
UCLASS()
class URecallVoxelSurfaceProcessor : public UMassProcessor
{
	GENERATED_BODY()
	URecallVoxelSurfaceProcessor();
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/**
 * FrameEnd (VoxelSurfaceFlush, after VoxelFlush): blocks on surface tasks and stitches chunk seams.
 */
UCLASS()
class URecallVoxelSurfaceFlushProcessor : public UMassProcessor
{
	GENERATED_BODY()
	URecallVoxelSurfaceFlushProcessor();
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
