// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
using UnrealBuildTool;

public class RecallVoxelAbility : ModuleRules
{
    public RecallVoxelAbility(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "RecallAbility",
            "RecallVoxelCore",
            "VoxelCore",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "MassCore",
            "MassEntity",
            "RecallCore",
            "RecallSimulation",
            "RecallVoxel",
        });
    }
}
