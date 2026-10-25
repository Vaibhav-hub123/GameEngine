#pragma once

#include <memory>
#include <string>

#include "xray/core/EngineConfig.h"

// Forward declarations of SDL types, so that including this header does not
// pull SDL into the rest of the engine or the sandbox.
struct SDL_Window;
struct SDL_GLContextState;

namespace xray {

struct FramebufferSize {
    int width = 0;
    int height = 0;
};

// Window wraps the operating-system window, its OpenGL context and the event
// queue. SDL3 (an external library) does the platform-specific work; Window is
// XRay's small, SDL-free-looking face of it.
//
// OWNERSHIP: a Window owns everything it creates and releases it in the
// destructor, in reverse order of creation:
//     SDL video subsystem  ->  SDL window  ->  OpenGL context
//     (destroyed:  context, then window, then video subsystem)
// Create one with Window::Create(); there is no way to hold a half-built Window.
class Window {
public:
    // A function that returns the address of an OpenGL function by name. The
    // renderer hands this to GLAD so it can look up every GL function.
    using GLFunctionPointer = void (*)();
    using GLLoader = GLFunctionPointer (*)(const char* functionName);

    // Returns nullptr (after logging the reason) if anything fails.
    static std::unique_ptr<Window> Create(const EngineConfig& config);

    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Drains the operating system's event queue. Call once per frame.
    void PollEvents();

    bool ShouldClose() const { return m_shouldClose; }
    void RequestClose(const char* reason);

    // Presents the frame that was just drawn.
    void SwapBuffers();

    void SetTitle(const std::string& title);

    // Size of the drawable area in real pixels (can differ from the window size
    // on high-DPI displays).
    FramebufferSize GetFramebufferSize() const;

    static GLLoader GetGLLoader();

private:
    Window() = default;
    bool Initialize(const EngineConfig& config);

    SDL_Window* m_window = nullptr;
    SDL_GLContextState* m_glContext = nullptr;
    bool m_videoSubsystemInitialized = false;
    bool m_shouldClose = false;
    bool m_quitOnEscape = true;
};

}  // namespace xray
