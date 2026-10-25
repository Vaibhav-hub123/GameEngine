include(FetchContent)

# doctest: a single-header C++ test framework. We only need its header, so we
# download it but skip its own CMake project (SOURCE_SUBDIR points at a folder
# that does not exist). This also avoids build problems with old
# cmake_minimum_required values in third-party projects on new CMake versions.
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.12
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  xray_skip_doctest_cmake)
FetchContent_MakeAvailable(doctest)

add_library(xray_doctest INTERFACE)
target_include_directories(xray_doctest SYSTEM INTERFACE "${doctest_SOURCE_DIR}")

add_executable(xray_tests
    unit/main.cpp
    unit/Camera2DTests.cpp
    unit/FrameTimerTests.cpp
    unit/LogTests.cpp)
    
target_link_libraries(xray_tests PRIVATE XRay::Engine xray_doctest)
xray_enable_warnings(xray_tests)

# These tests need no window or GPU, so they run anywhere, including CI.
add_test(NAME xray_unit_tests COMMAND xray_tests)
