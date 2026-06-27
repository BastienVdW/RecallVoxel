// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"
#include "RecallVoxelLandscapeProcessors.generated.h"

/** PrePhysics (VoxelLandscapeDispatch, after VoxelSurfaceDispatch): dispatches landscape sim tasks after surface columns are cached. */
UCLASS()
class URecallVoxelLandscapeProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
	URecallVoxelLandscapeProcessor();
	
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/** FrameEnd (VoxelLandscapeFlush, after VoxelSurfaceFlush): ForceEnd sim + mesh gen. */
UCLASS()
class URecallVoxelLandscapeFlushProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelLandscapeFlushProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/** FrameEnd (VoxelLandscapeFlush, after VoxelLandscapeFlushProcessor): drains brush queue and applies to landscape. */
UCLASS()
class URecallVoxelLandscapeBrushFlushProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelLandscapeBrushFlushProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/** FrameEnd (VoxelLandscapeFlush): draws surface debug visualization when enabled. */
UCLASS()
class URecallVoxelLandscapeDebugRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()
	URecallVoxelLandscapeDebugRepresentationProcessor();
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
