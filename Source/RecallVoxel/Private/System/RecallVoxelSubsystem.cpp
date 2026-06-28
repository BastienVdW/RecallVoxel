// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "System/RecallVoxelSubsystem.h"

#include "Engine/World.h"
#include "Grid/VoxelGrid.h"
#include "Settings/RecallVoxelDeveloperSettings.h"
#include "Streaming/VoxelStreamingSubsystem.h"
#include "StructUtils/InstancedStruct.h"

void URecallVoxelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UVoxelStreamingSubsystem>();
	VoxelStreamingSystem = UWorld::GetSubsystem<UVoxelStreamingSubsystem>(GetWorld());
}

void URecallVoxelSubsystem::Deinitialize()
{
	VoxelStreamingSystem.Reset();
	Super::Deinitialize();
}

void URecallVoxelSubsystem::Start(const FRecallSimulationStartParams& Params)
{
	if (!VoxelStreamingSystem.IsValid())
	{
		return;
	}
	
	// Initial bake — we need to force the generation of all dirty chunks so that we can save the initial state of the voxel grid.
	VoxelStreamingSystem->ForceEndGeneration();
	
	if (VoxelStreamingSystem.IsValid())
	{
		DefaultGridNextModifierId = VoxelStreamingSystem->GetGrid().GetNextModifierId();
	}
}

void URecallVoxelSubsystem::Reset()
{
	RemoveDynamicModifiers();
	NextModifierId = 0;

	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->GetMutableGrid().SetNextModifierId(DefaultGridNextModifierId);
		VoxelStreamingSystem->StartDirtyChunkGeneration();
	}
}

void URecallVoxelSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	checkf(PendingModifierCommands.IsEmpty(),
		TEXT("%hs called with unflushed modifier commands — FlushModifierCommands must be called before saving"), __FUNCTION__);
	
	OutSnapshot.InitializeAs<FVoxelGridSnapshot>();
	
	FVoxelGridSnapshot& Snapshot = OutSnapshot.GetMutable<FVoxelGridSnapshot>();
	Snapshot.DynamicModifiers = DynamicModifiers;
	Snapshot.RecallNextModifierId = NextModifierId;
	
	if (VoxelStreamingSystem.IsValid())
	{
		const FVoxelGrid& Grid = VoxelStreamingSystem->GetGrid();
		Snapshot.GridNextModifierId = Grid.GetNextModifierId();
		Snapshot.DirtyChunkCoords = Grid.GetDirtyChunkCoords();
	}
}

void URecallVoxelSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
	const FVoxelGridSnapshot* Snapshot = InSnapshot.GetPtr<FVoxelGridSnapshot>();
	if (!Snapshot || !VoxelStreamingSystem.IsValid())
	{
		return;
	}

	NextModifierId = Snapshot->RecallNextModifierId;

	FVoxelGrid& Grid = VoxelStreamingSystem->GetMutableGrid();
	const TSet<FRecallModifierHandle> ModifiersToAdd = RemoveDeprecatedModifiers(Grid, Snapshot->DynamicModifiers);

	for (const FRecallModifierHandle RecallHandle : ModifiersToAdd)
	{
		const FVoxelModifierRecord& Record = Snapshot->DynamicModifiers.FindChecked(RecallHandle);
		Grid.AddModifier(Record.Data, Record.GridHandle.Id);
		DynamicModifiers.Add(RecallHandle, Record);
		GridToRecallHandle.Add(Record.GridHandle, RecallHandle);
	}

	Grid.SetNextModifierId(Snapshot->GridNextModifierId);
	Grid.SetDirtyChunkCoords(Snapshot->DirtyChunkCoords);
	VoxelStreamingSystem->StartDirtyChunkGeneration();
}

void URecallVoxelSubsystem::PostRestore()
{
	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->ForceEndGeneration();
	}
}

TSet<FRecallModifierHandle> URecallVoxelSubsystem::RemoveDeprecatedModifiers(
	FVoxelGrid& Grid, const TMap<FRecallModifierHandle, FVoxelModifierRecord>& NewDynamicModifiers)
{
	TSet<FRecallModifierHandle> ModifiersToAdd;
	NewDynamicModifiers.GetKeys(ModifiersToAdd);

	for (auto It = DynamicModifiers.CreateIterator(); It; ++It)
	{
		const FRecallModifierHandle& OldHandle = It.Key();
		FVoxelModifierRecord& CurrentRecord = It.Value();
		
		const FVoxelModifierRecord* NewRecord = NewDynamicModifiers.Find(OldHandle);
		const bool bIsPersistent = NewRecord != nullptr;

		if (bIsPersistent && CurrentRecord == *NewRecord)
		{
			ModifiersToAdd.Remove(OldHandle);
			continue;
		}

		Grid.RemoveModifier(CurrentRecord.GridHandle);
		It.RemoveCurrent();
		GridToRecallHandle.Remove(CurrentRecord.GridHandle);
	}

	return ModifiersToAdd;
}

const FVoxelGrid* URecallVoxelSubsystem::GetGrid() const
{
	return VoxelStreamingSystem.IsValid() ? &VoxelStreamingSystem->GetGrid() : nullptr;
}

const TArray<FIntVector>& URecallVoxelSubsystem::GetLastBakedCoords() const
{
	static const TArray<FIntVector> Empty;
	return VoxelStreamingSystem.IsValid() ? VoxelStreamingSystem->GetLastBakedCoords() : Empty;
}

void URecallVoxelSubsystem::StartVoxelGeneration(float DeltaTime)
{
	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->Tick(DeltaTime);
		VoxelStreamingSystem->StartDirtyChunkGeneration();
	}
}

void URecallVoxelSubsystem::ForceEndVoxelGeneration()
{
	if (VoxelStreamingSystem.IsValid())
	{
		VoxelStreamingSystem->ForceEndGeneration();
	}
}

void URecallVoxelSubsystem::RemoveDynamicModifiers()
{
	if (!VoxelStreamingSystem.IsValid())
	{
		return;
	}
	
	FVoxelGrid& Grid = VoxelStreamingSystem->GetMutableGrid();
	
	for (auto& [RecallHandle, Rec] : DynamicModifiers)
	{
		if (Rec.GridHandle.IsValid())
		{
			Grid.RemoveModifier(Rec.GridHandle);
		}
	}
	
	DynamicModifiers.Reset();
	GridToRecallHandle.Reset();
}

FRecallModifierHandle URecallVoxelSubsystem::AddDynamicModifier(const FVoxelModifierData& Data)
{
	if (!VoxelStreamingSystem.IsValid())
	{
		return FRecallModifierHandle::Invalid();
	}

	const uint32 RecallId = ++NextModifierId;
	const FRecallModifierHandle Handle{ RecallId };

	FVoxelModifierRecord& Record = DynamicModifiers.Add(Handle);
	Record.Data           = Data;
	Record.RecallHandle	  = Handle;
	Record.bStatic        = false;

	PendingModifierCommands.Enqueue(FVoxelModifierCommand::MakeAdd(RecallId));
	return Handle;
}

void URecallVoxelSubsystem::RemoveDynamicModifier(FRecallModifierHandle Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	// Queue the remove; DynamicModifiers entry stays until FlushModifierCommands so the grid HandleId is available
	PendingModifierCommands.Enqueue(FVoxelModifierCommand::MakeRemove(Handle.Id));
}

void URecallVoxelSubsystem::FlushModifierCommands()
{
	if (PendingModifierCommands.IsEmpty())
	{
		return;
	}

	if (!VoxelStreamingSystem.IsValid())
	{
		PendingModifierCommands.Empty();
		return;
	}
	
	FVoxelGrid& Grid = VoxelStreamingSystem->GetMutableGrid();

	FVoxelModifierCommand Cmd;
	while (PendingModifierCommands.Dequeue(Cmd))
	{
		FVoxelModifierRecord* Rec = DynamicModifiers.Find(Cmd.RecallHandle);
		if (!Rec) continue;

		if (Cmd.bIsAdd)
		{
			PruneOverlappingDynamicModifiers(Grid, Rec->Data);
			Rec->GridHandle = Grid.AddModifier(Rec->Data);
			GridToRecallHandle.Add(Rec->GridHandle, Cmd.RecallHandle);
		}
		else
		{
			if (Rec->GridHandle.IsValid())
			{
				GridToRecallHandle.Remove(Rec->GridHandle);
				Grid.RemoveModifier(Rec->GridHandle);
			}
			DynamicModifiers.Remove(Cmd.RecallHandle);
		}
	}
}

void URecallVoxelSubsystem::PruneOverlappingDynamicModifiers(FVoxelGrid& Grid, const FVoxelModifierData& NewData)
{
	const TArray<FModifierHandle> CandidateHandles = Grid.GetModifierHandlesInBounds(NewData.GetWorldBounds().GetBox());
	if (CandidateHandles.IsEmpty())
	{
		return;
	}

	PruneToRemoveCache.Reset(CandidateHandles.Num());
	
	for (const FModifierHandle& GridHandle : CandidateHandles)
	{
		if (const FRecallModifierHandle* RecallHandle = GridToRecallHandle.Find(GridHandle))
		{
			FVoxelModifierRecord& Rec = DynamicModifiers[*RecallHandle];
			if (!Rec.bStatic)
			{
				PruneToRemoveCache.Add(*RecallHandle);
			}
		}
	}
	
	const int32 MaxOverlapping = GetDefault<URecallVoxelDeveloperSettings>()->MaxOverlappingDynamicModifiers;
	const int32 RemoveCount = PruneToRemoveCache.Num() - MaxOverlapping;

	for (int32 RmvIdx = 0; RmvIdx < RemoveCount; ++RmvIdx)
	{
		const FRecallModifierHandle& RecallHandle = PruneToRemoveCache[RmvIdx];
		FVoxelModifierRecord& Rec = DynamicModifiers[RecallHandle];
		GridToRecallHandle.Remove(Rec.GridHandle);
		Grid.RemoveModifier(Rec.GridHandle);
		DynamicModifiers.Remove(RecallHandle);
	}
}
