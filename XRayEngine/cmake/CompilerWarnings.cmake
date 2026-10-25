# Compiler-specific warning flags live here and nowhere else, so the rest of
# the build files stay compiler-agnostic.
#
# Usage:  xray_enable_warnings(my_target)
#
# Flags are applied PRIVATE to the given target only, so third-party code
# (SDL, GLAD, doctest) is never subjected to our warning level.

function(xray_enable_warnings target)
    if(MSVC)
        # /W4         high warning level
        # /permissive- strict standards conformance
        # /utf-8      source and execution charset are UTF-8
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
        if(XRAY_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
        if(XRAY_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()
