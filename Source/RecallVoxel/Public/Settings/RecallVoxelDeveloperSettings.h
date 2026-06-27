// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RecallVoxelDeveloperSettings.generated.h"

// Project Settings → Plugins → RecallVoxel
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Recall Voxel"))
class RECALLVOXEL_API URecallVoxelDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URecallVoxelDeveloperSettings();

	// Maximum number of dynamic modifiers allowed to overlap in any single region.
	// Oldest overlapping modifiers are pruned when this limit is exceeded.
	// Default: 100.
	UPROPERTY(Config, EditAnywhere, Category="Modifiers", meta=(ClampMin="1", ClampMax="1000"))
	int32 MaxOverlappingDynamicModifiers = 100;
};
