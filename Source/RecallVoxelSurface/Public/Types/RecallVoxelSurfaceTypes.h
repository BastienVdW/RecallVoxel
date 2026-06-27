// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "VoxelSurfaceTypes.h"

#include "RecallVoxelSurfaceTypes.generated.h"

/**
 * Lightweight snapshot of UVoxelSurfaceSubsystem state.
 * Only stores FVoxelSurfaceChunk column data — no rendered meshes.
 * Captured at each recall keyframe; restored before the landscape processor
 * runs so URecallVoxelLandscapeSubsystem reads coherent surface heights.
 */
USTRUCT()
struct RECALLVOXELSURFACE_API FVoxelSurfaceSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FIntVector2, FVoxelSurfaceChunk> SurfaceChunks;
};
