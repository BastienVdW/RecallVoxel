// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#pragma once

#include "MassEntityTraitBase.h"
#include "Simulation/RecallVoxelLandscapeMaskFragments.h"
#include "RecallVoxelLandscapeMaskTrait.generated.h"

UCLASS(meta = (DisplayName = "RE Landscape Mask"))
class RECALLVOXELLANDSCAPE_API URecallVoxelLandscapeMaskTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Landscape Mask", meta = (GetOptions = "GetLandscapeLayerOptions"))
	FName LayerName;

	UPROPERTY(EditAnywhere, Category = "Landscape Mask")
	FRecallVoxelLandscapeMaskCapsule Shape;

	UPROPERTY(EditAnywhere, Category = "Landscape Mask")
	bool bApplyOnMovement = true;

	UPROPERTY(EditAnywhere, Category = "Landscape Mask", meta = (Units = Centimeters, ClampMin = "0.0", EditCondition = "bApplyOnMovement"))
	float MovementTolerance = 1.f;

	UPROPERTY(EditAnywhere, Category = "Landscape Mask", meta = (ClampMin = "0.0"))
	float IdleUpdateInterval = 1.f;

	UPROPERTY(EditAnywhere, Category = "Landscape Mask", meta = (Units = Centimeters))
	float Intensity = -100.f;

	/** Limits brush writes without forcing values already outside the interval back into it. */
	UPROPERTY(EditAnywhere, Category = "Landscape Mask")
	FFloatInterval ValueRange = FFloatInterval(5.f, 10.f);

	UFUNCTION()
	TArray<FName> GetLandscapeLayerOptions() const;
};
