# External dependencies, fetched at configure time with FetchContent.
# Everything is pinned to an exact tag so a fresh clone builds the same way
# every time. Fetching requires `git` and network access on the FIRST configure
# of each build directory.

include(FetchContent)

# ---- SDL3: windowing, input, OpenGL context creation ----------------------
# Built from source as a STATIC library so there is no SDL3.dll to copy next to
# the executable. The options below only trim things we do not need.
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.18
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(SDL3)

# GLAD (OpenGL function loader) is NOT fetched: its generated sources are
# vendored in third_party/glad. See third_party/glad/README.md.
# doctest (unit tests) is fetched in tests/CMakeLists.txt, only when tests are on.
