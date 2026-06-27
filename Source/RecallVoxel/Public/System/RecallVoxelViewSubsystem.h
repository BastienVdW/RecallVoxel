// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Streaming/VoxelView.h"

#include "RecallVoxelViewSubsystem.generated.h"

UCLASS()
class RECALLVOXEL_API URecallVoxelViewSubsystem : public UWorldSubsystem, public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	// Called by observer processor with entity serial as ViewId
	void RegisterView(uint32 ViewId, const FVoxelView& View);
	void UnregisterView(uint32 ViewId);
	void UpdateView(uint32 ViewId, const FVoxelView& View);

	// Updates only the Position field of an existing view, preserving radii and other settings.
	void UpdateViewPosition(uint32 ViewId, const FVector& Position);

	const TMap<uint32, FVoxelView>& GetViews() const { return ManagedViews; }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// IRecallSimulationReactSystemInterface
	virtual void Start(const FRecallSimulationStartParams& Params) override {}
	virtual void Reset() override;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override;
	virtual void PostRestore() override;

private:
	void SyncToStreamingSubsystem();

	TWeakObjectPtr<class UVoxelStreamingSubsystem> VoxelStreamingSystem;

	TMap<uint32, FVoxelView> ManagedViews;
};

template<>
struct TMassExternalSubsystemTraits<URecallVoxelViewSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
