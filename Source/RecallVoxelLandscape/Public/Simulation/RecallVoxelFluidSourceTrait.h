// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityTraitBase.h"
#include "RecallVoxelFluidSourceTrait.generated.h"

UCLASS(meta=(DisplayName="RE Fluid Source"))
class RECALLVOXELLANDSCAPE_API URecallVoxelFluidSourceTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Fluid Source", meta = (GetOptions = "GetFluidLayerOptions"))
	FName LayerName;

	UPROPERTY(EditAnywhere, Category = "Fluid Source", meta = (ClampMin = "0.0"))
	float EmissionRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Fluid Source", meta = (ClampMin = "0.0"))
	float BurstAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Fluid Source", meta = (ClampMin = "0.0"))
	float MaxEmission = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Fluid Source", meta = (ClampMin = "0.0"))
	float Radius = 100.0f;

	UFUNCTION()
	TArray<FName> GetFluidLayerOptions() const;
};
