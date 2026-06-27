// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Performance baselines for SDF bake and chunk generation.
// These tests fail if processing time exceeds a generous threshold so regressions
// are caught automatically. They also log the actual elapsed time for profiling.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"
#include "Generation/VoxelChunkGenerator.h"
#include "Grid/VoxelGrid.h"
#include "Modifier/VoxelModifierTypes.h"
#include "SDF/VoxelSDFBaker.h"

static void BuildBoxMesh(TArray<FVector>& OutVerts, TArray<int32>& OutIndices, float Half = 50.f)
{
    OutVerts = {
        FVector(-Half,-Half,-Half), FVector( Half,-Half,-Half), FVector( Half, Half,-Half), FVector(-Half, Half,-Half),
        FVector(-Half,-Half, Half), FVector( Half,-Half, Half), FVector( Half, Half, Half), FVector(-Half, Half, Half),
    };
    OutIndices = {
        0,2,1, 0,3,2,  4,5,6, 4,6,7,
        0,1,5, 0,5,4,  2,3,7, 2,7,6,
        0,4,7, 0,7,3,  1,2,6, 1,6,5,
    };
}

// ---------------------------------------------------------------------------
// SDF bake time: 32³ grid, simple 12-triangle box — should finish well under 1s
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Perf_BakeTime32,
    "RecallVoxel.SDF.Perf.BakeTime32",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Perf_BakeTime32::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    BuildBoxMesh(Verts, Indices);

    FVoxelModifierData Mod;
    Mod.Params.Type = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;

    double Start = FPlatformTime::Seconds();
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);
    double Elapsed = FPlatformTime::Seconds() - Start;

    UE_LOG(LogTemp, Log, TEXT("SDF bake 32³ × 12 triangles: %.1f ms"), Elapsed * 1000.0);
    TestTrue(FString::Printf(TEXT("Bake 32³ < 500ms (was %.1fms)"), Elapsed * 1000.0), Elapsed < 0.5);

    return true;
}

// ---------------------------------------------------------------------------
// SDF bake time: 64³ grid — the cost scales as O(R^3 * T) so 64³ is 8x slower
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Perf_BakeTime64,
    "RecallVoxel.SDF.Perf.BakeTime64",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Perf_BakeTime64::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    BuildBoxMesh(Verts, Indices);

    FVoxelModifierData Mod;
    Mod.Params.Type = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;

    double Start = FPlatformTime::Seconds();
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 64);
    double Elapsed = FPlatformTime::Seconds() - Start;

    UE_LOG(LogTemp, Log, TEXT("SDF bake 64³ × 12 triangles: %.1f ms"), Elapsed * 1000.0);
    TestTrue(FString::Printf(TEXT("Bake 64³ < 5000ms (was %.1fms)"), Elapsed * 1000.0), Elapsed < 5.0);

    return true;
}

// ---------------------------------------------------------------------------
// Chunk generation time: bake a single 16³ chunk with one MeshSDF modifier
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Perf_ChunkGenTime,
    "RecallVoxel.SDF.Perf.ChunkGenTime",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Perf_ChunkGenTime::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    BuildBoxMesh(Verts, Indices);

    FVoxelModifierData Mod;
    Mod.Params.Type      = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    Mod.Transform.SetLocation(FVector(200.f, 200.f, 200.f));
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    FVoxelGrid Grid;
    Grid.AddModifier(Mod);
    TArray<FIntVector> Dirty = Grid.GetDirtyChunks();

    FVoxelChunkGenerator Gen;

    double Start = FPlatformTime::Seconds();
    Gen.StartGeneration(nullptr, Grid, Dirty);
    Gen.ForceEndGeneration(nullptr, Grid);
    double Elapsed = FPlatformTime::Seconds() - Start;

    UE_LOG(LogTemp, Log, TEXT("Chunk gen (16³ voxels, MeshSDF): %.2f ms for %d chunk(s)"),
        Elapsed * 1000.0, Dirty.Num());
    TestTrue(FString::Printf(TEXT("Chunk gen < 500ms (was %.1fms)"), Elapsed * 1000.0), Elapsed < 0.5);

    return true;
}

// ---------------------------------------------------------------------------
// Sample throughput: 10000 SampleSDFTrilinear calls in < 10ms
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSDF_Perf_SampleThroughput,
    "RecallVoxel.SDF.Perf.SampleThroughput",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVoxelSDF_Perf_SampleThroughput::RunTest(const FString&)
{
    TArray<FVector> Verts; TArray<int32> Indices;
    BuildBoxMesh(Verts, Indices);

    FVoxelModifierData Mod;
    Mod.Params.Type = EModifierType::MeshSDF;
    Mod.Params.Operation = EModifierOp::Add;
    VoxelSDF::BakeTriangleMeshSDF(Verts, Indices, {}, Mod, 32);

    const int32 N = 10000;
    float Accumulator = 0.f; // prevent dead-code elimination

    double Start = FPlatformTime::Seconds();
    for (int32 i = 0; i < N; ++i)
    {
        FVector P(FMath::FRandRange(-60.f, 60.f), FMath::FRandRange(-60.f, 60.f), FMath::FRandRange(-60.f, 60.f));
        Accumulator += FVoxelChunkGenerator::SampleSDFTrilinear(Mod, P);
    }
    double Elapsed = FPlatformTime::Seconds() - Start;

    UE_LOG(LogTemp, Log, TEXT("SDF sample throughput: %d samples in %.2f ms (acc=%.1f)"),
        N, Elapsed * 1000.0, Accumulator);
    TestTrue(FString::Printf(TEXT("%d samples < 10ms (was %.2fms)"), N, Elapsed * 1000.0), Elapsed < 0.01);

    return true;
}
