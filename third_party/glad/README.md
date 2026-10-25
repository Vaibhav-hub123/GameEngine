# Vendored GLAD (OpenGL loader)

Everything under `include/` and `src/` is **generated code** from
[GLAD 2](https://github.com/Dav1dde/glad) (version 2.0.8) for:

- API: OpenGL 3.3, core profile
- Extensions: none
- Loader: none built in (XRay passes `SDL_GL_GetProcAddress` to `gladLoadGL`)

The generated files carry their own license headers (WTFPL / CC0 for GLAD
itself, Apache-2.0 for the Khronos registry content, MIT-style for
`khrplatform.h`). Do not edit them by hand.

## Why vendored?

GLAD's generator needs Python. Committing the generated output means a clean
checkout builds on a Windows machine without Python installed.

## Regenerating

```
pip install glad2
# an empty file means "no extensions"
type nul > noext.txt            (Windows)   |   : > noext.txt   (Linux/macOS)
glad --api gl:core=3.3 --extensions noext.txt --out-path glad_out c
```

Then copy `glad_out/include` and `glad_out/src` over this folder. Note that the
generator downloads the OpenGL registry from the internet.

If a later milestone needs a newer OpenGL version or an extension, regenerate
with the matching `--api` / `--extensions` values.
