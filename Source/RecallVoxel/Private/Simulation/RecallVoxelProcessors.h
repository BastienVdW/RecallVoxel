// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"

#include "RecallVoxelProcessors.generated.h"

/** PrePhysics: ticks VoxelStreamingSubsystem and kicks off generation for dirty chunks. */
UCLASS()
class URecallVoxelStreamingProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelStreamingProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/** FrameEnd: forces generation completion and notifies downstream subsystems. */
UCLASS()
class URecallVoxelFlushProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelFlushProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

/** FrameEnd: forces modifier generation completion and notifies downstream subsystems. */
UCLASS()
class URecallVoxelModifierFlushProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelModifierFlushProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

