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

// Reusable helper: builds a 100cm box mesh (local verts ±50) and bakes SDF with the given world transform.
static FVoxelModifierData MakeBoxAtTransform(const FTransform& WorldTransform, int32 Res = 32)
{
    TArray<FVector> Verts = {
        FVector(-50,-50,-50), FVector( 50,-50,-50), FVector( 50, 50,-50), FVector(-50, 50,-50),
        FVector(-50,-50, 50), FVector( 50,-50, 50), FVector( 50, 50, 50), FVector(-50, 50, 50),
    };
    TArray<int32> Indices = {
        0,2,1, 0,3,2,  4,5,6, 4,6,7,
        0,1,5, 0,5,4,  2,3,7, 2,7,6,
        0,4,7, 0,7,3,  1,2,6, 1,6,5,
    };
    FVoxelModifierData Mod;
    Mod.Params.Type      = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    Mod.Transform = WorldTransform;
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, Res);
    return Mod;
}

// ---------------------------------------------------------------------------
// Guard: SDFSamples must be populated after baking — if this fails, the
// in-game check (SDFResolution > 0 && SDFSamples.Num() == R^3) will silently
// skip the modifier and leave all voxels at -1.
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Edge_BakePopulatesSamples,
    "RecallVoxel.SDF.Edge.BakePopulatesSamples",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Edge_BakePopulatesSamples::RunTest(const FString&)
{
    FVoxelModifierData Mod = MakeBoxAtTransform(FTransform(FVector(200.f, 200.f, 200.f)));
    TestTrue("SDFResolution set to 32", Mod.SDF.Resolution == 32);
    TestTrue("SDFSamples count is 32^3", Mod.SDF.Samples.Num() == 32 * 32 * 32);
    TestTrue("SDFLocalBounds is valid", (bool)Mod.SDF.LocalBounds.IsValid);
    TestFalse("SDFSamples are not all zero", Mod.SDF.Samples.FindByPredicate([](float v){ return v != 0.f; }) == nullptr);
    return true;
}

// ---------------------------------------------------------------------------
// Box at negative chunk coordinates — reproduces user's actor at (-200,-200,300).
// Chunk (-1,-1,0) covers world X∈[-400,0], Y∈[-400,0], Z∈[0,400].
// The box centre (−200,−200,300) is inside it.
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Edge_NegativeChunkCoords,
    "RecallVoxel.SDF.Edge.NegativeChunkCoords",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Edge_NegativeChunkCoords::RunTest(const FString&)
{
    // Exactly the transform the user observed in-game
    FVoxelModifierData Mod = MakeBoxAtTransform(FTransform(FVector(-200.f, -200.f, 300.f)));

    FVoxelGrid Grid;
    Grid.AddModifier(Mod);

    TArray<FIntVector> Dirty = Grid.GetDirtyChunks();
    TestTrue("At least one dirty chunk", Dirty.Num() >= 1);

    // Chunk (-1,-1,0) should definitely be dirty
    bool bFoundExpected = Dirty.ContainsByPredicate([](const FIntVector& C){ return C == FIntVector(-1,-1,0); });
    TestTrue("Chunk (-1,-1,0) is dirty", bFoundExpected);

    FVoxelChunkGenerator Gen;
    Gen.StartGeneration(nullptr, Grid, Dirty);
    Gen.ForceEndGeneration(nullptr, Grid);

    const FVoxelChunk* Chunk = Grid.QueryChunk(FIntVector(-1, -1, 0));
    TestNotNull("Chunk (-1,-1,0) exists", Chunk);
    if (!Chunk) return false;

    TestTrue("Chunk reports bHasSolidVoxels", Chunk->bHasSolidVoxels);

    // Voxel at (8,8,12) in chunk (-1,-1,0):
    // WorldPos = (-400 + (8+0.5)*25, same, 0 + (12+0.5)*25) = (-187.5, -187.5, 312.5)
    // Local (relative to box at -200,-200,300) = (12.5, 12.5, 12.5) → inside box ✓
    const FVoxel& VInside = Chunk->At(8, 8, 12);
    TestTrue("Voxel (8,8,12) solid — inside box at (-200,-200,300)", VInside.Density > 0.f);

    // Voxel at (0,0,0): world (-387.5,-387.5,12.5) — far outside box
    const FVoxel& VOutside = Chunk->At(0, 0, 0);
    TestTrue("Voxel (0,0,0) empty — far from box", VOutside.Density <= 0.f);

    return true;
}

// ---------------------------------------------------------------------------
// Non-unit scale: box with scale (2,2,2) covers local verts ±50, but world
// extent is ±100 (100cm scaled to 200cm). Voxels up to 100cm from centre
// should be solid.
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Edge_NonUnitScale,
    "RecallVoxel.SDF.Edge.NonUnitScale",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Edge_NonUnitScale::RunTest(const FString&)
{
    FTransform T;
    T.SetLocation(FVector(200.f, 200.f, 200.f));
    T.SetScale3D(FVector(2.f, 2.f, 2.f)); // local ±50 → world ±100

    FVoxelModifierData Mod = MakeBoxAtTransform(T);

    TArray<FVoxelModifierData> Mods = { Mod };

    // Centre: local (0,0,0) → always inside
    float DCenter = FVoxelChunkGenerator::EvaluateDensity(FVector(200.f, 200.f, 200.f), Mods);
    TestTrue("Centre density > 0", DCenter > 0.f);

    // 90cm from centre: world (290,200,200) → local = InverseTransform((290,200,200)) with scale (2,2,2)
    // Local = (290-200, 0, 0) / 2 = (45, 0, 0) — inside box (±50)
    float D90 = FVoxelChunkGenerator::EvaluateDensity(FVector(290.f, 200.f, 200.f), Mods);
    TestTrue("90cm from centre (local 45cm) is solid", D90 > 0.f);

    // 110cm from centre: world (310,200,200) → local (55,0,0) — outside box
    float D110 = FVoxelChunkGenerator::EvaluateDensity(FVector(310.f, 200.f, 200.f), Mods);
    TestTrue("110cm from centre (local 55cm) is empty", D110 <= 0.f);

    return true;
}

// ---------------------------------------------------------------------------
// Box spanning two adjacent chunks: centre at (350,200,200), radius ±50 →
// world [300,400] in X spans chunk (0,0,0) and chunk (1,0,0) boundary.
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Edge_SpansTwoChunks,
    "RecallVoxel.SDF.Edge.SpansTwoChunks",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Edge_SpansTwoChunks::RunTest(const FString&)
{
    // Box centre at (350,200,200), local ±50 → world X ∈ [300,400]
    // Chunk (0,0,0) covers X∈[0,400], chunk (1,0,0) covers X∈[400,800].
    // The box just touches the boundary — at minimum chunk (0,0,0) must be dirty.
    FVoxelModifierData Mod = MakeBoxAtTransform(FTransform(FVector(350.f, 200.f, 200.f)));

    FVoxelGrid Grid;
    Grid.AddModifier(Mod);

    TArray<FIntVector> Dirty = Grid.GetDirtyChunks();
    TestTrue("At least one dirty chunk", Dirty.Num() >= 1);

    // Chunk (0,0,0) must be included since box occupies [300..400] in X
    bool bChunk0 = Dirty.ContainsByPredicate([](const FIntVector& C){ return C == FIntVector(0,0,0); });
    TestTrue("Chunk (0,0,0) is dirty", bChunk0);

    FVoxelChunkGenerator Gen;
    Gen.StartGeneration(nullptr, Grid, Dirty);
    Gen.ForceEndGeneration(nullptr, Grid);

    // Voxel at (14,8,8) in chunk (0,0,0):
    // World = (0 + (14+0.5)*25, (8+0.5)*25, (8+0.5)*25) = (362.5, 212.5, 212.5)
    // Local to box at (350,200,200) = (12.5, 12.5, 12.5) → inside ✓
    const FVoxelChunk* C0 = Grid.QueryChunk(FIntVector(0,0,0));
    if (TestNotNull("Chunk (0,0,0) exists", C0))
    {
        TestTrue("Voxel near box centre is solid", C0->At(14, 8, 8).Density > 0.f);
    }

    return true;
}

// ---------------------------------------------------------------------------
// Empty mesh: BakeTriangleMeshSDF with no vertices must not crash and must
// leave SDFResolution=0 so EvaluateDensity gracefully skips the modifier.
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Edge_EmptyMesh,
    "RecallVoxel.SDF.Edge.EmptyMesh",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Edge_EmptyMesh::RunTest(const FString&)
{
    FVoxelModifierData Mod;
    Mod.Params.Type      = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    TArray<FVector> Verts;
    TArray<int32>   Indices;
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    TestTrue("SDFResolution stays 0 for empty mesh", Mod.SDF.Resolution == 0);
    TestTrue("SDFSamples empty for empty mesh", Mod.SDF.Samples.IsEmpty());

    // EvaluateDensity should skip this modifier — result stays at default -1
    TArray<FVoxelModifierData> Mods = { Mod };
    float D = FVoxelChunkGenerator::EvaluateDensity(FVector::ZeroVector, Mods);
    TestTrue("EvaluateDensity returns default -1 for empty-mesh modifier", D <= 0.f);

    return true;
}
