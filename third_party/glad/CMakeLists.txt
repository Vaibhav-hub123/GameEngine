# GLAD: loads OpenGL function pointers at runtime. The sources in this folder
# were generated (see README.md) and are compiled as a tiny static library.

add_library(xray_glad STATIC src/gl.c)

# SYSTEM suppresses warnings from the generated header in code that includes it.
target_include_directories(xray_glad SYSTEM PUBLIC include)

set_target_properties(xray_glad PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)
