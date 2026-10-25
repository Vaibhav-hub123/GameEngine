# XRay Engine

> **See inside your game.**

XRay Engine is a small 2D game engine written in C++20, built to be **understood**
rather than just used. Its long-term goal is to make game behavior observable:
collision shapes, velocities, entity state, event timelines and performance numbers
should be one keypress away.

It is also a learning project. The code is meant to be read, debugged and changed
by the person who wrote it.

> **Status: Phase 1 (foundation).** The engine opens a window, runs a fixed-timestep
> game loop, clears the screen with OpenGL and shuts down cleanly. Nothing else yet.

## Design philosophy

- **Code first.** Engine runtime and debugging infrastructure matter more than editors or visual polish.
- **Readable over clever.** Small classes, plain C++, no frameworks, no patterns for their own sake.
- **Explicit ownership.** Every resource has one owner and is released in the destructor (RAII).
- **Observable by design.** Systems report what they do through one logging interface, so later debug tools can listen without changing engine code.
- **Real data only.** Debug features will read real engine state. No fake metrics.

## What is implemented (Phase 1)

| Feature | Written by XRay | Provided by a library |
|---|---|---|
| Application lifecycle (initialize, loop, shutdown) | yes | |
| Fixed-timestep game loop with frame timing and FPS measurement | yes | |
| Logging with levels, categories and pluggable sinks | yes | |
| Central `EngineConfig` | yes | |
| Window, event queue, OpenGL context | XRay `Window` wrapper | SDL3 |
| OpenGL function loading | XRay `Renderer` | GLAD (generated, vendored) |
| Clear color and present each frame, GL error reporting | yes | OpenGL driver |
| Unit tests (frame timer, logger) | yes | doctest |

## What is NOT implemented yet

Entities and components, transforms, collision detection, debug drawing, Dear ImGui,
event tracing, profiler, pause/step controls, Box2D, audio, asset loading, textures,
drawing of any shape. See [docs/roadmap.md](docs/roadmap.md).

## Requirements (Windows 11)

1. **Visual Studio 2022** (Community is free) with the workload **"Desktop development with C++"**.
   This provides the MSVC compiler and the Windows SDK.
2. **CMake 3.24 or newer** on your PATH. The simplest route is to install it from
   <https://cmake.org/download/> and tick *"Add CMake to the system PATH"*.
3. **Git**, <https://git-scm.com/download/win>. CMake uses it to download SDL3 and doctest.
4. An internet connection for the **first configure** of a build folder.
5. A GPU/driver that supports **OpenGL 3.3** (practically anything made in the last decade).

Check your tools in PowerShell:

```powershell
cmake --version    # must be 3.24 or newer
git --version
```

You do **not** need to install SDL3, GLAD, doctest or Python. They are handled by the build.

## Build and run (Windows, PowerShell)

From the folder containing this README:

```powershell
# 1. Configure (downloads SDL3 and doctest, takes a few minutes the first time)
cmake --preset vs2022

# 2. Build the Debug configuration (compiles SDL3 too the first time)
cmake --build --preset vs2022-debug

# 3. Run the unit tests
ctest --preset vs2022-debug

# 4. Run the sandbox
.\build\vs2022\bin\Debug\xray_sandbox.exe
```

A window titled **XRay Sandbox | NN.N FPS** opens with a slowly pulsing dark background and
log lines appear in the console. Close the window or press **Esc** to quit.

For an optimized build use `vs2022-release` instead of `vs2022-debug`
(and `.\build\vs2022\bin\Release\xray_sandbox.exe`).

**Smoke test without clicking anything:**

```powershell
.\build\vs2022\bin\Debug\xray_sandbox.exe --frames=300
echo $LASTEXITCODE        # 0 means the run and shutdown were clean
```

**Using the Visual Studio IDE instead:** run `cmake --preset vs2022` once, then open
`build\vs2022\XRayEngine.sln`. `xray_sandbox` is the startup project, so F5 runs it.
(Alternatively use *File > Open > Folder* on this directory; Visual Studio reads `CMakePresets.json`.)

### Other platforms

Not the target, but the same code builds on Linux with the `ninja` preset
(`cmake --preset ninja && cmake --build --preset ninja-debug`). SDL3 needs X11 or Wayland
development packages there, see <https://wiki.libsdl.org/SDL3/README-linux>.

## Common problems

| Symptom | Cause and fix |
|---|---|
| `'cmake' is not recognized` | CMake is not on PATH. Reinstall it with the PATH option, then open a **new** terminal. |
| `CMake Error: Could not find CMAKE_C_COMPILER` / generator `Visual Studio 17 2022` not found | Visual Studio 2022 or its C++ workload is missing. Run the Visual Studio Installer and add *Desktop development with C++*. |
| `Unknown preset "vs2022"` or `No such preset` | You are not in the project root, or your CMake is older than 3.24. |
| `Could not find git` / `Failed to clone` during configure | Install Git and reopen the terminal; check your internet connection or proxy. |
| Configure fails half-way after a network error | Delete the `build` folder and configure again. |
| `OpenGL 3.3 ... SDL_GL_CreateContext failed` in the log | Outdated or missing graphics driver (also happens inside some remote-desktop sessions). Update the GPU driver. |
| `could not set swap interval (vsync)` warning | Harmless: vsync is unsupported in this environment, so the loop runs unthrottled. |
| Antivirus quarantines `xray_sandbox.exe` | Unsigned freshly built executables are sometimes flagged. Add the `build` folder to exclusions. |

## Project structure

```
XRayEngine/
  CMakeLists.txt          top-level build, options, wiring
  CMakePresets.json       one-word configure/build/test commands
  cmake/                  CompilerWarnings.cmake, Dependencies.cmake (SDL3)
  engine/                 the engine, built as the static library xray_engine
    include/xray/         public headers, included as <xray/...>
      Application.h         lifecycle owner, the hooks a game overrides
      core/                 Log, FrameTimer, EngineConfig, Color
      platform/             Window (SDL3)
      renderer/             Renderer (OpenGL via GLAD)
    src/                  implementations, mirroring include/
  sandbox/                the demo executable; depends on the engine
  tests/unit/             doctest unit tests (no window or GPU needed)
  third_party/glad/       generated OpenGL loader, vendored on purpose
  docs/                   architecture.md, roadmap.md
```

Folders from the originally proposed layout that do not exist yet: `assets/`, `config/`,
`scripts/`, `engine/*/scene/`, `engine/*/debug/`. They would be empty today, and Git does not
track empty folders anyway. Each is added in the milestone that first puts something in it.

Read next: [docs/architecture.md](docs/architecture.md) explains how the pieces fit together.

## License

Not chosen yet. Add a `LICENSE` file before publishing the repository.
