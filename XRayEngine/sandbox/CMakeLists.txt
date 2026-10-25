add_executable(xray_sandbox src/main.cpp)

# The sandbox depends on the engine; the engine knows nothing about the sandbox.
target_link_libraries(xray_sandbox PRIVATE XRay::Engine)

xray_enable_warnings(xray_sandbox)
