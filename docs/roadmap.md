# Roadmap

Each milestone is a small, reviewable step. After each one we stop and review before starting the next.
Order and scope can change as we learn.

## Phase 1: Foundation (current)

- [x] CMake project, presets, warnings, pinned dependencies
- [x] `Application` lifecycle and fixed-timestep loop
- [x] SDL3 window and event handling, clean shutdown
- [x] OpenGL context, GLAD loading, clear color, GL error reporting
- [x] Logging with sinks
- [x] Unit tests for `FrameTimer` and `Log`
- [ ] Build and run verified on Windows 11 with MSVC (by the project owner; see README)

## Phase 2: Drawing something

Problem: the engine cannot show anything yet, so there is nothing to observe.
- 2D orthographic camera and coordinate system
- Immediate-mode drawing of rectangles and lines (one small shader, one dynamic vertex buffer)
- Sandbox draws a few rectangles

## Phase 3: Scene model

- Entity IDs, a Transform component, simple component storage
- Scene that owns entities and updates in `OnFixedUpdate`
- Sandbox moves a few entities by velocity

## Phase 4: Input and pause/step

- Minimal keyboard state (pressed / held / released per fixed step)
- Pause and single-step the simulation (first real debugging control)

## Phase 5: Collision and its debug view

- AABB and circle overlap tests, a collision system that stores its results
- Debug renderer draws collision shapes and contact data from that stored data
- Velocity and acceleration vector overlays

## Phase 6: Inspection and profiling

- Dear ImGui integration (small inspector and stats panel only)
- Entity and component inspector reading real state
- Scope-based profiler around real engine operations

## Phase 7: Event tracing

- Structured engine events (collision, spawn, state change) recorded with frame numbers
- Cause-and-effect timeline view

## Later / optional

- State recording and replay (builds on the fixed step)
- Box2D integration behind a clean boundary
- Textures and sprites, audio
- Continuous integration with Windows builds
