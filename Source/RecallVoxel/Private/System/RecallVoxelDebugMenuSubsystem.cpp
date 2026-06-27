// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "System/RecallVoxelDebugMenuSubsystem.h"

#include "Debug/DebugMenuInterface.h"
#include "Engine/GameInstance.h"
#include "System/Debug/DebugMenuSubsystem.h"

void URecallVoxelDebugMenuSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UDebugMenuSubsystem>();

#if WITH_DEBUG_MENU
	DebugMenuSubsystem = UGameInstance::GetSubsystem<UDebugMenuSubsystem>(GetGameInstance());
	if (DebugMenuSubsystem.IsValid())
	{
		CreateDebugMenuItems(DebugMenuSubsystem->GetMutableDebugMenu());
	}
#endif // WITH_DEBUG_MENU
}

void URecallVoxelDebugMenuSubsystem::Deinitialize()
{
	Super::Deinitialize();

#if WITH_DEBUG_MENU
	DebugMenuSubsystem.Reset();
#endif // WITH_DEBUG_MENU
}

TStatId URecallVoxelDebugMenuSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URecallVoxelDebugMenuSubsystem, STATGROUP_Tickables);
}

void URecallVoxelDebugMenuSubsystem::CreateDebugMenuItems(IDebugMenu& DebugMenu)
{
#if WITH_DEBUG_MENU
	DebugMenu.AddItem_Bool(TEXT("Voxel"), "Show Voxel Density", false, TEXT("recall.voxel.ShowVoxelDensity"));
	DebugMenu.AddItem_Int(TEXT("Voxel"), "Show Chunk Bounds", 0, 0, 2, TEXT("recall.voxel.ShowChunkBounds"));
	DebugMenu.AddItem_Bool(TEXT("Voxel"), "Show Physics Bodies", false, TEXT("recall.voxel.ShowPhysicsBodies"));
	DebugMenu.AddItem_Bool(TEXT("Voxel"), "Show Surface Debug", false, TEXT("VoxelSurface.Debug"));
	DebugMenu.AddItem_Int(TEXT("Voxel"), "Surface Debug Layer", -1, -1, 8, TEXT("VoxelSurface.Debug.Layer"));
#endif // WITH_DEBUG_MENU
}
