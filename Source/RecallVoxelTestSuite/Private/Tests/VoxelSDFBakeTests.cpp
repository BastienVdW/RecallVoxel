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
#include "Voxel/VoxelTypes.h"

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

// Builds a watertight unit cube from (-HalfSize) to (+HalfSize) in local space.
// Winding: each face has consistent outward-pointing normal (right-hand rule).
static void MakeBoxMesh(float HalfSize, TArray<FVector>& OutVerts, TArray<int32>& OutIndices)
{
    const float H = HalfSize;
    OutVerts = {
        FVector(-H,-H,-H), // 0 bottom-left-front
        FVector( H,-H,-H), // 1 bottom-right-front
        FVector( H, H,-H), // 2 bottom-right-back
        FVector(-H, H,-H), // 3 bottom-left-back
        FVector(-H,-H, H), // 4 top-left-front
        FVector( H,-H, H), // 5 top-right-front
        FVector( H, H, H), // 6 top-right-back
        FVector(-H, H, H), // 7 top-left-back
    };
    OutIndices = {
        0,2,1, 0,3,2,   // bottom  (-Z)
        4,5,6, 4,6,7,   // top     (+Z)
        0,1,5, 0,5,4,   // front   (-Y)
        2,3,7, 2,7,6,   // back    (+Y)
        0,4,7, 0,7,3,   // left    (-X)
        1,2,6, 1,6,5,   // right   (+X)
    };
}

// ----------------------------------------------------------------------------
// RayTriangle
// ----------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_RayTriangle_Hit,
    "RecallVoxel.SDF.RayTriangle.Hit",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_RayTriangle_Hit::RunTest(const FString&)
{
    // Triangle in the XY plane at Z=10
    FVector A(0,0,10), B(10,0,10), C(0,10,10);
    FVector Orig(1,1,0);
    FVector Dir(0,0,1);
    float T = 0.f;
    bool bHit = VoxelSDF::RayTriangle(Orig, Dir, A, B, C, T);
    TestTrue("Ray hits triangle", bHit);
    TestTrue("Hit T ≈ 10", FMath::IsNearlyEqual(T, 10.f, 0.01f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_RayTriangle_Miss,
    "RecallVoxel.SDF.RayTriangle.Miss",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_RayTriangle_Miss::RunTest(const FString&)
{
    // Same triangle, ray offset so it misses
    FVector A(0,0,10), B(10,0,10), C(0,10,10);
    FVector Orig(20,20,0); // clearly outside triangle footprint
    FVector Dir(0,0,1);
    float T = 0.f;
    bool bHit = VoxelSDF::RayTriangle(Orig, Dir, A, B, C, T);
    TestFalse("Ray misses triangle", bHit);
    return true;
}

// ----------------------------------------------------------------------------
// PointTriangleSqDist
// ----------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_PointTriDist_Interior,
    "RecallVoxel.SDF.PointTriangleSqDist.Interior",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_PointTriDist_Interior::RunTest(const FString&)
{
    // Right-angle triangle in XY plane: (0,0,0),(10,0,0),(0,10,0)
    FVector A(0,0,0), B(10,0,0), C(0,10,0);
    // Point directly above centroid (projects inside triangle)
    FVector P(2, 2, 5);
    FVector Cl; float U, V;
    float D2 = VoxelSDF::PointTriangleSqDist(P, A, B, C, Cl, U, V);
    // Closest point should be (2,2,0), distance = 5
    TestTrue("Distance squared ≈ 25", FMath::IsNearlyEqual(D2, 25.f, 0.01f));
    TestTrue("Closest Z ≈ 0", FMath::IsNearlyZero(Cl.Z, 0.01f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_PointTriDist_Vertex,
    "RecallVoxel.SDF.PointTriangleSqDist.Vertex",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_PointTriDist_Vertex::RunTest(const FString&)
{
    FVector A(0,0,0), B(10,0,0), C(0,10,0);
    // Point closest to vertex A
    FVector P(-3, -4, 0);
    FVector Cl; float U, V;
    float D2 = VoxelSDF::PointTriangleSqDist(P, A, B, C, Cl, U, V);
    // Closest = A, distance = sqrt(9+16) = 5
    TestTrue("Distance squared ≈ 25", FMath::IsNearlyEqual(D2, 25.f, 0.01f));
    TestTrue("Closest = A", Cl.Equals(A, 0.01f));
    return true;
}

// ----------------------------------------------------------------------------
// BakeTriangleMeshSDF — sign convention
// ----------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Bake_InsideNegative,
    "RecallVoxel.SDF.Bake.InsideNegative",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Bake_InsideNegative::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    MakeBoxMesh(50.f, Verts, Indices); // 100cm cube centred at origin

    FVoxelModifierData Mod;
    Mod.Params.Type = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    // Identity transform: world == local
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    TestTrue("SDFSamples populated", Mod.SDF.Samples.Num() == 32*32*32);
    TestTrue("SDFLocalBounds valid", (bool)Mod.SDF.LocalBounds.IsValid);

    // Centre of the cube — must be inside (SDF negative)
    float SdfCenter = FVoxelChunkGenerator::SampleSDFTrilinear(Mod, FVector(0.f, 0.f, 0.f));
    TestTrue("SDF at centre < 0 (inside)", SdfCenter < 0.f);

    // Far outside point
    float SdfOutside = FVoxelChunkGenerator::SampleSDFTrilinear(Mod, FVector(200.f, 0.f, 0.f));
    TestTrue("SDF at far point > 0 (outside)", SdfOutside > 0.f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Bake_DistanceMagnitude,
    "RecallVoxel.SDF.Bake.DistanceMagnitude",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Bake_DistanceMagnitude::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    MakeBoxMesh(50.f, Verts, Indices);

    FVoxelModifierData Mod;
    Mod.Params.Type = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    // Point at (40, 0, 0) — 10 cm from the right face (X=50)
    // SDF should be ≈ -10 (negative = inside, magnitude = distance to surface)
    float Sdf = FVoxelChunkGenerator::SampleSDFTrilinear(Mod, FVector(40.f, 0.f, 0.f));
    TestTrue("SDF at (40,0,0) < 0", Sdf < 0.f);
    // Allow generous tolerance (32³ grid, 100cm cube → ~3cm per cell)
    TestTrue("SDF at (40,0,0) ≈ -10 (within 5cm)", FMath::Abs(Sdf + 10.f) < 5.f);

    // Point at (70, 0, 0) — 20 cm outside the right face
    float SdfOut = FVoxelChunkGenerator::SampleSDFTrilinear(Mod, FVector(70.f, 0.f, 0.f));
    TestTrue("SDF at (70,0,0) > 0", SdfOut > 0.f);
    TestTrue("SDF at (70,0,0) ≈ 20 (within 5cm)", FMath::Abs(SdfOut - 20.f) < 5.f);

    return true;
}

// ----------------------------------------------------------------------------
// EvaluateDensity via MeshSDF — density is negated SDF (positive inside)
// ----------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Eval_InsidePositive,
    "RecallVoxel.SDF.EvaluateDensity.InsidePositive",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Eval_InsidePositive::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    MakeBoxMesh(50.f, Verts, Indices);

    FVoxelModifierData Mod;
    Mod.Params.Type = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    // Box centred at world (200, 200, 200) — clearly in chunk (0,0,0) [world 0..400]
    Mod.Transform.SetLocation(FVector(200.f, 200.f, 200.f));
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    TArray<FVoxelModifierData> Mods = { Mod };

    // World pos (200,200,200) = box centre → local (0,0,0) → density positive
    float D = FVoxelChunkGenerator::EvaluateDensity(FVector(200.f, 200.f, 200.f), Mods);
    TestTrue("Density at box centre > 0 (solid)", D > 0.f);

    // World pos far outside → density negative
    float DOut = FVoxelChunkGenerator::EvaluateDensity(FVector(400.f, 400.f, 400.f), Mods);
    TestTrue("Density far outside < 0 (empty)", DOut < 0.f);

    return true;
}

// ----------------------------------------------------------------------------
// EvaluateDensity with a NON-CENTRED mesh (vertices 0..100 instead of -50..50).
// This reproduces real SM_Cube assets whose pivot is at a corner, not the centre.
// When Transform.Location = (200,-390,250), the world centre of the geometry is
// at (250,-340,300) (or offset similarly depending on which corner).
// EvaluateDensity must still return positive at the geometry centre.
// ----------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Eval_NonCentredMesh,
    "RecallVoxel.SDF.EvaluateDensity.NonCentredMesh",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Eval_NonCentredMesh::RunTest(const FString&)
{
    // Vertices 0..100 in all axes — pivot at min corner, NOT centred
    TArray<FVector> Verts = {
        FVector(  0,  0,  0), FVector(100,  0,  0), FVector(100,100,  0), FVector(  0,100,  0),
        FVector(  0,  0,100), FVector(100,  0,100), FVector(100,100,100), FVector(  0,100,100),
    };
    TArray<int32> Indices = {
        0,2,1, 0,3,2,  4,5,6, 4,6,7,
        0,1,5, 0,5,4,  2,3,7, 2,7,6,
        0,4,7, 0,7,3,  1,2,6, 1,6,5,
    };

    FVoxelModifierData Mod;
    Mod.Params.Type      = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    Mod.Transform.SetLocation(FVector(200.f, -390.f, 250.f));
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    TestTrue("SDFSamples populated", Mod.SDF.Samples.Num() == 32 * 32 * 32);
    TestTrue("SDFLocalBounds valid", (bool)Mod.SDF.LocalBounds.IsValid);

    // Local centre is at (50,50,50). World centre = Transform.Location + (50,50,50)
    // = (250,-340,300)
    const FVector WorldGeomCentre(250.f, -340.f, 300.f);

    // Verify InverseTransform maps world centre → local centre
    FVector LocalCentre = Mod.Transform.InverseTransformPosition(WorldGeomCentre);
    TestTrue("InverseTransform gives local (50,50,50)",
        LocalCentre.Equals(FVector(50.f, 50.f, 50.f), 0.1f));

    TArray<FVoxelModifierData> Mods = { Mod };

    // Density at world geometry centre must be positive (solid)
    float DCenter = FVoxelChunkGenerator::EvaluateDensity(WorldGeomCentre, Mods);
    TestTrue("Density at geometry world-centre > 0", DCenter > 0.f);

    // Density at the pivot location (200,-390,250) = local (0,0,0) = mesh corner
    // SDF at a corner: the point IS on the surface so density ≈ 0. Allow slight
    // variation due to grid resolution — just verify it's not deeply negative.
    float DPivot = FVoxelChunkGenerator::EvaluateDensity(
        FVector(200.f, -390.f, 250.f), Mods);
    TestTrue("Density at pivot (mesh corner) >= -5 (near surface, not deep interior)",
        DPivot >= -5.f);

    // Far outside
    float DFar = FVoxelChunkGenerator::EvaluateDensity(FVector(0.f, 0.f, 0.f), Mods);
    TestTrue("Density far outside < 0", DFar < 0.f);

    return true;
}

// ----------------------------------------------------------------------------
// EvaluateDensity with a centred mesh placed at (200,-390,250) and identity rotation.
// Mirrors the in-game VoxelObjectComponent scenario. Verifies that:
//   (a) EvaluateDensity returns positive at the box world-centre
//   (b) FVoxelGrid + FVoxelChunkGenerator end-to-end produces solid voxels in the
//       chunk that contains the box
// ----------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Eval_InGamePosition,
    "RecallVoxel.SDF.EvaluateDensity.InGamePosition",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Eval_InGamePosition::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    MakeBoxMesh(50.f, Verts, Indices); // centred box, local verts ±50

    FVoxelModifierData Mod;
    Mod.Params.Type      = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    Mod.Transform.SetLocation(FVector(200.f, -390.f, 250.f));
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    // --- Part 1: direct EvaluateDensity calls ---

    TArray<FVoxelModifierData> Mods = { Mod };

    // Box world-centre = (200,-390,250), local (0,0,0) → inside → positive
    float DCenter = FVoxelChunkGenerator::EvaluateDensity(FVector(200.f, -390.f, 250.f), Mods);
    TestTrue("EvaluateDensity at box world-centre > 0", DCenter > 0.f);

    // 10 cm inside the +X face: world (240,-390,250) → local (40,0,0) → inside
    float DInside = FVoxelChunkGenerator::EvaluateDensity(FVector(240.f, -390.f, 250.f), Mods);
    TestTrue("EvaluateDensity 10cm inside +X face > 0", DInside > 0.f);

    // 200 cm outside along +X: world (450,-390,250) → clearly outside
    float DOutside = FVoxelChunkGenerator::EvaluateDensity(FVector(450.f, -390.f, 250.f), Mods);
    TestTrue("EvaluateDensity far outside < 0", DOutside < 0.f);

    // --- Part 2: full grid + generator ---

    FVoxelGrid Grid;
    Grid.AddModifier(Mod);

    TArray<FIntVector> Dirty = Grid.GetDirtyChunks();
    TestTrue("At least one dirty chunk after AddModifier", Dirty.Num() > 0);

    // Chunk containing the box: WorldToChunkCoord(200,-390,250) with 16*25=400 chunk size
    // = (0,-1,0)
    FVoxelGrid TmpGrid;
    FIntVector BoxChunk = TmpGrid.WorldToChunkCoord(FVector(200.f, -390.f, 250.f));
    TestTrue("Box lives in chunk (0,-1,0)", BoxChunk == FIntVector(0, -1, 0));

    bool bBoxChunkDirty = Dirty.ContainsByPredicate(
        [&](const FIntVector& C){ return C == BoxChunk; });
    TestTrue("Box chunk is dirty", bBoxChunkDirty);

    FVoxelChunkGenerator Gen;
    Gen.StartGeneration(nullptr, Grid, Dirty);
    Gen.ForceEndGeneration(nullptr, Grid);

    const FVoxelChunk* Chunk = Grid.QueryChunk(BoxChunk);
    TestNotNull("Box chunk exists after generation", Chunk);
    if (!Chunk) return false;

    TestTrue("Box chunk has solid voxels", Chunk->bHasSolidVoxels);

    // Voxel closest to box world-centre (200,-390,250) in chunk (0,-1,0):
    // ChunkOrigin = (0,-400,0); local voxel = (200/25, (-390+400)/25, 250/25) ≈ (8,0,10)
    // Centre of voxel (8,0,10): (8*25+12.5, -400+0*25+12.5, 10*25+12.5) = (212.5,-387.5,262.5)
    // Local to box: (12.5,2.5,12.5) — inside ±50 bounds → should be solid
    const FVoxel& VCenter = Chunk->At(8, 0, 10);
    TestTrue("Voxel (8,0,10) near box centre is solid", VCenter.Density > 0.f);

    return true;
}
