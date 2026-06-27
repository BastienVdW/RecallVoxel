# RecallVoxel

RecallVoxel is an experimental Unreal Engine plugin that adds voxel support to the
[Recall](../Recall) simulation library using [VoxelPlugin](../VoxelPlugin).

> [!WARNING]
> RecallVoxel is not production ready. Rollback support is a work in progress, and
> the memory, serialization, and resimulation costs of voxel data have not been
> solved for production-scale games. Recall itself is also experimental and should
> currently be considered alpha-quality software.

## Current scope

RecallVoxel connects Recall's fixed-step Mass simulation and snapshot lifecycle to
the voxel runtime. The current implementation includes:

- fixed-step dispatch and flush ordering for voxel generation, surface generation,
  landscape simulation, modifiers, and physics;
- snapshot, restore, and reset hooks for dynamic voxel modifiers;
- snapshot and restore of voxel streaming views managed by Mass entities;
- snapshot and restore of landscape cells and surface-column data;
- Jolt physics bodies generated from baked voxel chunks, with physics mesh
  descriptors included in snapshots;
- a Recall ability command that applies spherical voxel modifiers;
- Mass traits and processors for landscape masks and fluid sources; and
- development debug processors and menu integration.

The plugin is divided into the following runtime modules:

- `RecallVoxelCore`: shared handles, snapshot types, and processor group names.
- `RecallVoxel`: streaming views, dynamic modifiers, generation orchestration, and
  developer settings.
- `RecallVoxelSurface`: surface generation and surface snapshot integration.
- `RecallVoxelLandscape`: landscape simulation, masks, fluid sources, and landscape
  snapshots.
- `RecallVoxelPhysics`: voxel chunk collision through Recall's Jolt physics layer.
- `RecallVoxelAbility`: voxel commands for Recall abilities.

`RecallVoxelTestSuite` contains the plugin's Unreal Automation tests.

## Rollback status and limitations

Rollback integration exists, but it should be treated as a prototype. In
particular:

- Voxel state is inherently data-heavy. Surface columns, landscape cells, dynamic
  modifiers, streaming views, and physics triangle meshes may all contribute to a
  snapshot. Snapshot size and restore cost can grow quickly with world size,
  resolution, activity, and rollback history.
- Physics snapshots retain per-chunk mesh descriptors so collision can be recreated.
  This is useful for correctness experiments but is expensive in both memory and
  processing time.
- Surface rendering is asynchronous. After a restore, the rendered mesh may remain
  stale for a frame while dirty chunks regenerate.
- The current tests cover selected landscape and signed-distance-field behavior;
  they do not establish production-scale determinism, performance, networking, or
  long-session stability.
- APIs, snapshot formats, processor ordering, and module boundaries may change
  without compatibility guarantees.

Any game integration will need an explicit strategy for snapshot budgets, voxel
resolution, active-world bounds, compression or deltas, physics reconstruction,
and rollback-window length. Do not assume full voxel snapshots will scale to a
shipping game without substantial additional work.

## Requirements and setup

RecallVoxel is developed as part of the CCR workspace and expects compatible source
versions of these plugins:

- `Recall`
- `RecallAbility`
- `VoxelPlugin`

The plugin descriptor enables these dependencies. Enable `RecallVoxel` for the game
project, regenerate project files if required, and build the editor target from the
`CCR` directory:

```powershell
..\Engine\Build\BatchFiles\Build.bat CCREditor Win64 Development -Project="D:\Projects\CCR\CCR\CCR.uproject" -WaitMutex
```

Before starting a build, close the Unreal Editor and disable Live Coding, or wait
for any active UnrealBuildTool process to finish.

## Configuration

RecallVoxel settings are available under **Project Settings > Plugins > Recall
Voxel**. `Max Overlapping Dynamic Modifiers` limits the number of dynamic modifiers
that may overlap one region; when the limit is exceeded, the oldest overlapping
modifiers are pruned. The default is `100`.

## Tests

From the `CCR` directory, run:

```powershell
.\Plugins\RecallVoxel\Scripts\RunUnitTests.bat
```

The script builds `CCREditor`, runs the `RecallVoxel` Unreal Automation test group
with `NullRHI`, and writes reports under `CCR/Reports/RecallVoxel` relative to the
engine workspace root.

## License

RecallVoxel is licensed under the [Apache License 2.0](LICENSE).
