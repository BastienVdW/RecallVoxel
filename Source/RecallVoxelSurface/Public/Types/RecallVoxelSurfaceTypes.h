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
 * Snapshot of UVoxelSurfaceSubsystem state captured at each recall keyframe.
 *
 * Snapshot saves populate FullChunks (full copy, infrequent).
 * Rollback saves leave FullChunks empty — the subsystem restores from its
 * locally-maintained undo queue instead, which is never serialized.
 */
USTRUCT()
struct RECALLVOXELSURFACE_API FVoxelSurfaceSnapshot
{
	GENERATED_BODY()

	/** Full chunk map — populated on snapshot saves only. */
	UPROPERTY()
	TMap<FIntVector2, FVoxelSurfaceChunk> FullChunks;
};
