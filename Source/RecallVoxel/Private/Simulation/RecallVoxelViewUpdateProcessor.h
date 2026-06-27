// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassObserverProcessor.h"
#include "MassProcessor.h"

#include "RecallVoxelViewUpdateProcessor.generated.h"

/** Watches FRecallControllerFragment additions. On add: registers a voxel view for the controller entity. */
UCLASS()
class URecallVoxelViewObserverProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

	URecallVoxelViewObserverProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

/** Watches FRecallControllerFragment removals. On remove: unregisters the voxel view. */
UCLASS()
class URecallVoxelViewDeinitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URecallVoxelViewDeinitializer();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

/** PrePhysics: updates voxel view positions from controller transforms each frame. */
UCLASS()
class URecallVoxelViewUpdateProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelViewUpdateProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
