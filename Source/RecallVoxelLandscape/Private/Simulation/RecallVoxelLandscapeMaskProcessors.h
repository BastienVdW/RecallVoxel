// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#pragma once

#include "MassProcessor.h"
#include "RecallVoxelLandscapeMaskProcessors.generated.h"

UCLASS()
class URecallVoxelLandscapeMaskProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallVoxelLandscapeMaskProcessor();

protected:
	virtual bool ShouldAllowQueryBasedPruning(bool bRuntimeMode = true) const override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
