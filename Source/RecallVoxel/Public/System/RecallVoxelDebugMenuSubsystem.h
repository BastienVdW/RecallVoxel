// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "RecallVoxelDebugMenuSubsystem.generated.h"

class IDebugMenu;

UCLASS()
class RECALLVOXEL_API URecallVoxelDebugMenuSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override {}
	virtual TStatId GetStatId() const override;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<class UDebugMenuSubsystem> DebugMenuSubsystem;

	void CreateDebugMenuItems(IDebugMenu& DebugMenu);
};
