// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Generation/VoxelChunkGenerator.h"
#include "Grid/VoxelGrid.h"
#include "Modifier/VoxelModifierTypes.h"
#include "SDF/VoxelSDFBaker.h"

static void MakeBoxMeshSDF(float HalfSize, const FVector& WorldCenter, FVoxelModifierData& OutMod)
{
    TArray<FVector> Verts = {
        FVector(-HalfSize,-HalfSize,-HalfSize),
        FVector( HalfSize,-HalfSize,-HalfSize),
        FVector( HalfSize, HalfSize,-HalfSize),
        FVector(-HalfSize, HalfSize,-HalfSize),
        FVector(-HalfSize,-HalfSize, HalfSize),
        FVector( HalfSize,-HalfSize, HalfSize),
        FVector( HalfSize, HalfSize, HalfSize),
        FVector(-HalfSize, HalfSize, HalfSize),
    };
    TArray<int32> Indices = {
        0,2,1, 0,3,2,
        4,5,6, 4,6,7,
        0,1,5, 0,5,4,
        2,3,7, 2,7,6,
        0,4,7, 0,7,3,
        1,2,6, 1,6,5,
    };
    OutMod.Params.Type      = EModifierType::MeshSDF;
    OutMod.Params.Operation = EModifierOp::Add;
    OutMod.Transform.SetLocation(WorldCenter);
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, OutMod, 32);
}

// ---------------------------------------------------------------------------
// Chunk voxels inside the box are solid, outside are empty
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Grid_VoxelsInsideBoxAreSolid,
    "RecallVoxel.SDF.Grid.VoxelsInsideBoxAreSolid",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Grid_VoxelsInsideBoxAreSolid::RunTest(const FString&)
{
    // 100cm box at world (200,200,200). Default chunk is 16 voxels * 25cm = 400cm.
    // Chunk (0,0,0) covers [0..400]^3 so the box is entirely inside it.
    FVoxelModifierData Mod;
    MakeBoxMeshSDF(50.f, FVector(200.f, 200.f, 200.f), Mod);

    FVoxelGrid Grid;
    Grid.AddModifier(Mod);

	TArray<FIntVector> Dirty = Grid.GetDirtyChunks();
	
    FVoxelChunkGenerator Gen;
    Gen.StartGeneration(nullptr, Grid, Dirty);
    Gen.ForceEndGeneration(nullptr, Grid);

    const FVoxelChunk* Chunk = Grid.QueryChunk(FIntVector(0, 0, 0));
    TestNotNull("Chunk (0,0,0) created", Chunk);
    if (!Chunk) return false;

    TestTrue("Chunk reports bHasSolidVoxels", Chunk->bHasSolidVoxels);

    // Voxel at (8,8,8): world centre ≈ (8+0.5)*25 = 212.5 cm in all axes — inside the box
    const FVoxel& VInside = Chunk->At(8, 8, 8);
    TestTrue("Voxel at (8,8,8) is solid — inside 100cm box at (200,200,200)", VInside.Density > 0.f);

    // Voxel at (0,0,0): world ≈ (12.5, 12.5, 12.5) — 187.5 cm from box centre, clearly outside
    const FVoxel& VOutside = Chunk->At(0, 0, 0);
    TestTrue("Voxel at (0,0,0) is empty — far from box", VOutside.Density <= 0.f);

    // Voxel at (15,15,15): world ≈ (387.5, 387.5, 387.5) — far outside
    const FVoxel& VCorner = Chunk->At(15, 15, 15);
    TestTrue("Voxel at (15,15,15) is empty — far corner", VCorner.Density <= 0.f);

    return true;
}

// ---------------------------------------------------------------------------
// World bounds for a MeshSDF modifier map to the correct chunks
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Grid_BoundsMarkCorrectChunks,
    "RecallVoxel.SDF.Grid.BoundsMarkCorrectChunks",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Grid_BoundsMarkCorrectChunks::RunTest(const FString&)
{
    // Box at (200,200,200) radius 50 → entirely in chunk (0,0,0)
    FVoxelModifierData Mod;
    MakeBoxMeshSDF(50.f, FVector(200.f, 200.f, 200.f), Mod);

    FVoxelGrid Grid;
    Grid.AddModifier(Mod);

    TArray<FIntVector> Dirty = Grid.GetDirtyChunks();
    TestTrue("Exactly one dirty chunk", Dirty.Num() == 1);
    if (Dirty.Num() > 0)
        TestTrue("Dirty chunk is (0,0,0)", Dirty[0] == FIntVector(0, 0, 0));

    return true;
}

// ---------------------------------------------------------------------------
// GetWorldBounds returns a box that intersects the expected chunk
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_WorldBounds_ContainsMeshRegion,
    "RecallVoxel.SDF.WorldBounds.ContainsMeshRegion",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_WorldBounds_ContainsMeshRegion::RunTest(const FString&)
{
    FVoxelModifierData Mod;
    MakeBoxMeshSDF(50.f, FVector(200.f, 200.f, 200.f), Mod);

    FBoxSphereBounds WB = Mod.GetWorldBounds();
    FBox Box = WB.GetBox();

    // The box [150..250]^3 at world (200,200,200) ± 50
    TestTrue("World bounds Min.X <= 150", Box.Min.X <= 150.f + 2.f); // +2 for 1cm padding
    TestTrue("World bounds Max.X >= 250", Box.Max.X >= 250.f - 2.f);
    TestTrue("World bounds contains centre", Box.IsInsideOrOn(FVector(200.f, 200.f, 200.f)));

    return true;
}
