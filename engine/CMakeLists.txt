add_library(xray_engine STATIC
    src/Application.cpp
    src/core/FrameTimer.cpp
    src/core/Log.cpp
    src/platform/Window.cpp
    src/renderer/Camera2D.cpp
    src/renderer/Renderer.cpp
    src/renderer/ShaderProgram.cpp)
    
add_library(XRay::Engine ALIAS xray_engine)

# Public headers live in include/xray/... and are included as <xray/...>.
target_include_directories(xray_engine PUBLIC include)
target_compile_features(xray_engine PUBLIC cxx_std_20)

# SDL3 and GLAD are implementation details: only .cpp files in the engine
# include their headers, so they are linked PRIVATE. Code that uses the engine
# (the sandbox, the tests) never sees SDL or OpenGL headers.
target_link_libraries(xray_engine PRIVATE SDL3::SDL3 xray_glad)

xray_enable_warnings(xray_engine)
