// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "System/RecallVoxelViewSubsystem.h"

#include "Engine/World.h"
#include "Streaming/VoxelStreamingSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Types/RecallVoxelTypes.h"

void URecallVoxelViewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UVoxelStreamingSubsystem>();
	VoxelStreamingSystem = UWorld::GetSubsystem<UVoxelStreamingSubsystem>(GetWorld());
}

void URecallVoxelViewSubsystem::Deinitialize()
{
	VoxelStreamingSystem.Reset();
	Super::Deinitialize();
}

void URecallVoxelViewSubsystem::Reset()
{
	if (VoxelStreamingSystem.IsValid())
	{
		for (const auto& [Id, View] : ManagedViews)
		{
			VoxelStreamingSystem->UnregisterView(Id);
		}
	}
	ManagedViews.Reset();
}

void URecallVoxelViewSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	FVoxelViewSnapshot Snapshot;
	Snapshot.Views = ManagedViews;
	OutSnapshot = FInstancedStruct::Make<FVoxelViewSnapshot>(Snapshot);
}

void URecallVoxelViewSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
	const FVoxelViewSnapshot* Snapshot = InSnapshot.GetPtr<FVoxelViewSnapshot>();
	if (!Snapshot)
	{
		return;
	}

	if (VoxelStreamingSystem.IsValid())
	{
		for (const auto& [Id, View] : ManagedViews)
		{
			VoxelStreamingSystem->UnregisterView(Id);
		}
	}

	ManagedViews = Snapshot->Views;
	// Sync deferred to PostRestore
}

void URecallVoxelViewSubsystem::PostRestore()
{
	SyncToStreamingSubsystem();
}

void URecallVoxelViewSubsystem::SyncToStreamingSubsystem()
{
	if (!VoxelStreamingSystem.IsValid())
	{
		return;
	}
	for (const auto& [Id, View] : ManagedViews)
	{
		VoxelStreamingSystem->RegisterView(Id, View);
	}
}

void URecallVoxelViewSubsystem::RegisterView(uint32 ViewId, const FVoxelView& View)
{
	ManagedViews.Add(ViewId, View);
	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->RegisterView(ViewId, View);
	}
}

void URecallVoxelViewSubsystem::UnregisterView(uint32 ViewId)
{
	ManagedViews.Remove(ViewId);
	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->UnregisterView(ViewId);
	}
}

void URecallVoxelViewSubsystem::UpdateViewPosition(uint32 ViewId, const FVector& Position)
{
	FVoxelView* View = ManagedViews.Find(ViewId);
	if (!View)
	{
		return;
	}
	View->Position = Position;
	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->UpdateView(ViewId, *View);
	}
}

void URecallVoxelViewSubsystem::UpdateView(uint32 ViewId, const FVoxelView& View)
{
	ManagedViews[ViewId] = View;
	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->UpdateView(ViewId, View);
	}
}
