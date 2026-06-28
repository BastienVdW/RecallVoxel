// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Types/RecallVoxelTypes.h"
#include "Containers/Queue.h"

#include "RecallVoxelSubsystem.generated.h"

class FVoxelGrid;

struct FVoxelModifierCommand
{
	bool   bIsAdd         = true;
	FRecallModifierHandle RecallHandle;  // stable subsystem-level ID; used by both Add and Remove to find the record

	static FVoxelModifierCommand MakeAdd(uint32 RecallId)    { return { true,  RecallId }; }
	static FVoxelModifierCommand MakeRemove(uint32 RecallId) { return { false, RecallId }; }
};

UCLASS()
class RECALLVOXEL_API URecallVoxelSubsystem : public UWorldSubsystem, public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ Begin IRecallSimulationReactSystemInterface Interface
	virtual void Start(const FRecallSimulationStartParams& Params) override;
	virtual int32 GetStartOrderPriority() const override { return Recall::SimReactSystem::StartOrder::MediumPriority; }
	virtual void Reset() override;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override;
	virtual void PostRestore() override;
	//~ End IRecallSimulationReactSystemInterface Interface

	// Pass-throughs to UVoxelStreamingSubsystem — use that subsystem directly when possible
	const FVoxelGrid*         GetGrid() const;
	const TArray<FIntVector>& GetLastBakedCoords() const;
	
	void StartVoxelGeneration(float DeltaTime);
	void ForceEndVoxelGeneration();

	FRecallModifierHandle AddDynamicModifier(const FVoxelModifierData& Data);
	void RemoveDynamicModifier(FRecallModifierHandle Handle);

	// Called by URecallVoxelFlushProcessor after ForceEndGeneration — safe to mutate the grid here
	void FlushModifierCommands();

private:
	TSet<FRecallModifierHandle> RemoveDeprecatedModifiers(const TMap<FRecallModifierHandle, FVoxelModifierRecord>& NewDynamicModifiers);
	void RemoveDynamicModifiers();

	// Removes the oldest dynamic modifiers that overlap NewData's bounds, keeping at most
	// URecallVoxelDeveloperSettings::MaxOverlappingDynamicModifiers in any region. Called from FlushModifierCommands.
	void PruneOverlappingDynamicModifiers(FVoxelGrid& Grid, const FVoxelModifierData& NewData);

	TWeakObjectPtr<class UVoxelStreamingSubsystem> VoxelStreamingSystem;
	
	uint32 NextModifierId = 0;
	uint32 DefaultGridNextModifierId = 1;

	TMap<FRecallModifierHandle, FVoxelModifierRecord> DynamicModifiers;
	TMap<FModifierHandle, FRecallModifierHandle>      GridToRecallHandle; // reverse lookup for prune
	
	TQueue<FVoxelModifierCommand>                     PendingModifierCommands;
	TArray<FRecallModifierHandle>                     PruneToRemoveCache;
};

template<>
struct TMassExternalSubsystemTraits<URecallVoxelSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
