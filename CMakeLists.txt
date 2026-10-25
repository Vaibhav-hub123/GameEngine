cmake_minimum_required(VERSION 3.24)

project(XRayEngine
    VERSION 0.1.0
    DESCRIPTION "See inside your game: a small, debugging-first 2D engine for learning"
    LANGUAGES C CXX)   # C is only needed for the vendored GLAD loader

# ---------------------------------------------------------------------------
# Language settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)               # -std=c++20, not gnu++20
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)       # lets editors (clangd, VS Code) see flags

# All executables land in <build dir>/bin (multi-config generators such as
# Visual Studio add a Debug/ or Release/ sub-folder automatically).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(XRAY_BUILD_SANDBOX "Build the sandbox executable" ON)
option(XRAY_BUILD_TESTS "Build the unit tests" ON)
option(XRAY_WARNINGS_AS_ERRORS "Treat warnings in XRay code as errors" OFF)

# ---------------------------------------------------------------------------
# Project-wide CMake helpers and dependencies
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)   # defines xray_enable_warnings(<target>)
include(Dependencies)       # fetches SDL3

add_subdirectory(third_party/glad)
add_subdirectory(engine)

if(XRAY_BUILD_SANDBOX)
    add_subdirectory(sandbox)
    # Makes "F5" in Visual Studio run the sandbox instead of ALL_BUILD.
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT xray_sandbox)
endif()

if(XRAY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
