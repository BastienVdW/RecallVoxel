// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Modifier/VoxelModifierTypes.h"
#include "Streaming/VoxelView.h"
#include "Physics/RecallPhysicsBodyHandle.h"
#include "RecallVoxelTypes.generated.h"

// Stable caller-facing handle returned by URecallVoxelSubsystem::AddDynamicModifier
USTRUCT(BlueprintType)
struct RECALLVOXELCORE_API FRecallModifierHandle
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 Id = 0;

	static FRecallModifierHandle Invalid() { return {}; }
	bool IsValid() const { return Id != 0; }
	bool operator==(const FRecallModifierHandle& O) const { return Id == O.Id; }
	bool operator!=(const FRecallModifierHandle& O) const { return Id != O.Id; }
	friend uint32 GetTypeHash(const FRecallModifierHandle& H) { return H.Id; }
};

// Wraps a modifier with its handle ID and static/dynamic classification
USTRUCT()
struct RECALLVOXELCORE_API FVoxelModifierRecord
{
	GENERATED_BODY()

	bool operator==(const FVoxelModifierRecord& Other) const
	{
		return RecallHandle == Other.RecallHandle &&
			bStatic == Other.bStatic &&
			Data == Other.Data;
	}

	bool operator!=(const FVoxelModifierRecord& Other) const { return !(*this == Other); }

	UPROPERTY()
	FVoxelModifierData Data;

	// Stable subsystem-level ID (FRecallModifierHandle) — never changes
	UPROPERTY()
	FRecallModifierHandle RecallHandle;

	// Grid-level ID (FModifierHandle::Id) — filled by FlushModifierCommands after Add
	UPROPERTY()
	FModifierHandle GridHandle;

	UPROPERTY()
	bool bStatic = false;
};

// Full snapshot of the dynamic modifier list
USTRUCT()
struct RECALLVOXELCORE_API FVoxelGridSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FRecallModifierHandle, FVoxelModifierRecord> DynamicModifiers;
	
	UPROPERTY()
	uint32 RecallNextModifierId = 1;
	
	UPROPERTY()
	uint32 GridNextModifierId = 1;

	UPROPERTY()
	TSet<FIntVector> DirtyChunkCoords;
};

// Physics shape descriptor for one chunk — stored for Restore
USTRUCT()
struct RECALLVOXELCORE_API FVoxelPhysicsChunkDescriptor
{
	GENERATED_BODY()

	UPROPERTY()
	FIntVector        ChunkCoord  = FIntVector::ZeroValue;
	
	UPROPERTY()
	FVector           WorldOrigin = FVector::ZeroVector;
	
	// chunk-local space
	UPROPERTY()
	TArray<FVector3f> Vertices;
	
	UPROPERTY()
	TArray<int32>     Triangles;
};

// Snapshot of all voxel physics shapes
USTRUCT()
struct RECALLVOXELCORE_API FVoxelPhysicsSnapshot
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<FIntVector, FJPRPhysicsBodyHandle> ChunkBodies;

	UPROPERTY()
	TMap<FIntVector, FVoxelPhysicsChunkDescriptor> ChunkDescriptors;
};

// Snapshot of all registered views
USTRUCT()
struct RECALLVOXELCORE_API FVoxelViewSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<uint32, FVoxelView> Views;
};
