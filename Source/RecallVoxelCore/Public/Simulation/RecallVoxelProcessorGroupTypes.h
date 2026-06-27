// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

namespace RecallVoxel::ProcessorGroupNames
{
	// PrePhysics -- voxel generation tasks dispatched
	const FName VoxelGenerationDispatch = FName(TEXT("RecallVoxelGenerationDispatch"));
	
	// PrePhysics — surface column tasks dispatched; landscape must run after so it reads committed surface
	const FName VoxelSurfaceDispatch = FName(TEXT("RecallVoxelSurfaceDispatch"));

	// PrePhysics — landscape sim tasks dispatched (after surface dispatch)
	const FName VoxelLandscapeDispatch = FName(TEXT("RecallVoxelLandscapeDispatch"));

	// FrameEnd — voxel generation complete, baked coords available for downstream consumers
	const FName VoxelFlush = FName(TEXT("RecallVoxelFlush"));
	const FName VoxelPhysicsFlush = FName(TEXT("RecallVoxelPhysicsFlush"));
	const FName VoxelModifierFlush = FName(TEXT("RecallVoxelModifierFlush"));

	// FrameEnd — surface generation complete; committed surface ready for landscape
	const FName VoxelSurfaceFlush = FName(TEXT("RecallVoxelSurfaceFlush"));

	// FrameEnd — landscape simulation and mesh gen complete
	const FName VoxelLandscapeFlush = FName(TEXT("RecallVoxelLandscapeFlush"));

	// FrameEnd — landscape sensor colliders updated
	const FName VoxelLandscapePhysicsFlush = FName(TEXT("RecallVoxelLandscapePhysicsFlush"));

} // namespace RecallVoxel::ProcessorGroupNames
