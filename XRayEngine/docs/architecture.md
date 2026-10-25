# XRay Engine architecture (Phase 1)

This document describes what exists today and where future features will plug in.
Anything described as "future" is **not implemented**.

## 1. Modules

| Module | Folder | Purpose | Depends on |
|---|---|---|---|
| core | `xray/core` | Things every system needs: `Log`, `FrameTimer`, `EngineConfig`, `Color`. No knowledge of windows or graphics. | standard library only |
| platform | `xray/platform` | `Window`: SDL3 window, OpenGL context, event queue. | core, SDL3 |
| renderer | `xray/renderer` | `Renderer`: loads OpenGL, clears and presents each frame. | core, platform, GLAD |
| (top level) | `xray/Application.h` | `Application`: owns the window and renderer, runs the loop, exposes hooks to the game. | everything above |
| scene *(future)* | | Entities, transforms, components. | core |
| debug *(future)* | | Event tracing, overlays, inspector, profiler. | core (and read access to scene) |

### Dependency direction

```
sandbox ──► Application ──► renderer ──► platform ──► core
                │                                       ▲
                └───────────────────────────────────────┘
```

Arrows point from user to dependency. Lower layers never include headers of higher layers:
`core` knows nothing about `platform`, and the engine knows nothing about the sandbox.

**Why `Application` is not in `core/`.** It must create a `Window` and a `Renderer`. If it lived in
`core`, `core` would depend on `platform`, reversing the arrow above. So it sits one level above.

### What is XRay's code and what is a library's

- **SDL3** (fetched, built statically): creates the OS window, delivers events, creates the OpenGL
  context, swaps buffers. XRay's `Window` is the only code that includes SDL headers.
- **GLAD** (generated, vendored): looks up OpenGL function addresses. Only `Renderer.cpp` includes it.
- **OpenGL** (your GPU driver): the actual drawing. Phase 1 uses only viewport, clear color, clear.
- **doctest** (fetched, tests only): test framework.
- **Everything else** is XRay: loop, timing, logging, ownership, lifecycle, configuration.

SDL and GLAD are linked `PRIVATE` to the engine library, so the sandbox and the tests cannot
accidentally depend on them.

## 2. Application lifecycle and game loop

`Application::Run()`:

```
Run()
 ├─ Initialize()
 │    ├─ ValidateConfig()              reject nonsensical settings with a clear log message
 │    ├─ create FrameTimer from config
 │    ├─ Window::Create()              SDL video init -> SDL window -> GL 3.3 core context -> vsync
 │    └─ Renderer::Create()            gladLoadGL, log GL version/vendor/renderer
 ├─ RunLoop()
 │    └─ while not quit requested and window not closing:
 │         1. timer.Tick(now)                     measure real time since last frame
 │         2. window.PollEvents()                 drain SDL's event queue (close, Esc, resize)
 │         3. while timer.ConsumeFixedStep():     catch the simulation up
 │               OnFixedUpdate(1/60 s)             <-- game hook
 │         4. renderer.BeginFrame()               viewport + clear
 │            OnRender()                           <-- game hook
 │            renderer.EndFrame()                  report GL errors, swap buffers
 │         5. if a new FPS sample exists: update the window title
 └─ Shutdown()                        renderer, then window
```

### Fixed timestep, in short

Real frames vary in length; simulation steps must not. `FrameTimer` keeps an accumulator of
owed time. Each frame adds the real elapsed time (clamped to `maxFrameSeconds`) and the loop
runs as many 1/60 s steps as fit. A 40 ms frame runs two steps and keeps 6.7 ms for next frame.
This makes behavior independent of frame rate and is a prerequisite for deterministic replay.
The long explanation is in the comment at the top of `FrameTimer.h`.

## 3. Resource ownership and shutdown

| Resource | Owner | Released by |
|---|---|---|
| `Window` | `Application::m_window` (`unique_ptr`) | `Application::Shutdown()` or destructor |
| `Renderer` | `Application::m_renderer` (`unique_ptr`) | `Application::Shutdown()` or destructor |
| SDL video subsystem, SDL window, GL context | `Window` | `Window::~Window()` (context, window, then subsystem) |
| Logger state | function-local static in `Log.cpp` | process exit |

Rules the code follows:

- Creation is by factory (`Window::Create`, `Renderer::Create`) returning `nullptr` and logging the reason on failure. There is no way to hold a half-initialized object.
- A partially constructed `Window` cleans up exactly what it had created (each handle is null-checked in the destructor).
- `Renderer` holds a reference to `Window`. The member order in `Application` (window declared before renderer) and the explicit order in `Shutdown()` guarantee the renderer dies first.
- All shutdown paths go through the same code: normal close, Esc, `--frames=N`, SIGINT/SIGTERM (SDL turns these into a quit event) and failed initialization.

## 4. Logging as the observability seam

`xray::log` is the only place engine systems report events. A message is
`{level, category, text, time}` and is delivered to every registered sink. The console sink is
the default.

Today every important lifecycle event is a log line with a category (`App`, `Platform`, `Renderer`).
That is the hook for the future debug module: it registers its own sink to build an in-game log
panel or an event timeline, and no engine code changes.

## 5. How future debugging features will attach

These are **plans**, not code.

| Feature | Where it plugs in |
|---|---|
| **Frame stepping / pause** | `Application::RunLoop()` step 3. A "paused" flag skips `ConsumeFixedStep()`; a "step once" request runs exactly one `OnFixedUpdate`. Works because simulation advances only in fixed steps. |
| **Performance measurement** | Wrap real operations (the fixed-update block, `BeginFrame`..`EndFrame`) with a scope timer that records into a profiler in `debug/`. `FrameTimer` already provides the frame time and FPS baseline. |
| **Collision overlays** | A future collision system will keep its result (pairs, contact points, shapes) in a plain struct. The debug renderer reads that struct in `OnRender()` and draws outlines through `Renderer`. No collision code needs to know about drawing. |
| **Velocity / acceleration vectors** | Same pattern: draw from the real components after the fixed update, with `InterpolationAlpha()` available for smooth drawing. |
| **Entity inspection** | A small inspector (likely Dear ImGui) iterates the scene's entities and components read-only, in `OnRender()`. |
| **Event tracing and cause/effect timeline** | Systems emit structured events (starting with the existing log calls) to a recorder registered as a sink or a dedicated event channel; the timeline reads the recording. |
| **Recording and replay** | Possible because of the fixed step: record inputs per step number and re-run `OnFixedUpdate` with the same sequence. |

Principle: **systems expose data; debug tools read it.** Engine systems do not contain drawing or
UI code for debugging, which keeps each one understandable on its own.

## 6. Build structure

- `xray_engine` (static library) is the engine. `xray_sandbox` and `xray_tests` link to it.
- Compiler warning flags are applied per target through `xray_enable_warnings()` in
  `cmake/CompilerWarnings.cmake`, so third-party code is not affected.
- Dependencies are pinned in `cmake/Dependencies.cmake` (SDL3) and `tests/CMakeLists.txt` (doctest).
