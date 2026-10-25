#include "xray/platform/Window.h"

#include <SDL3/SDL.h>

#include "xray/core/Log.h"

namespace xray {
namespace {

constexpr std::string_view kLogCategory = "Platform";

// Wraps SDL_GL_GetProcAddress in a function with exactly the signature that
// Window::GLLoader promises.
Window::GLFunctionPointer LoadGLFunction(const char* name) {
    return reinterpret_cast<Window::GLFunctionPointer>(SDL_GL_GetProcAddress(name));
}

}  // namespace

std::unique_ptr<Window> Window::Create(const EngineConfig& config) {
    // make_unique cannot call the private constructor, so we use new directly
    // and wrap the pointer immediately. If Initialize() fails, the unique_ptr
    // destroys the half-built Window and its destructor cleans up whatever
    // had been created so far.
    std::unique_ptr<Window> window(new Window());
    if (!window->Initialize(config)) {
        return nullptr;
    }
    return window;
}

bool Window::Initialize(const EngineConfig& config) {
    m_quitOnEscape = config.quitOnEscape;

    const int sdlVersion = SDL_GetVersion();
    log::Info(kLogCategory, "SDL {}.{}.{}",
              SDL_VERSIONNUM_MAJOR(sdlVersion), SDL_VERSIONNUM_MINOR(sdlVersion), SDL_VERSIONNUM_MICRO(sdlVersion));

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        log::Error(kLogCategory, "SDL_InitSubSystem(VIDEO) failed: {}", SDL_GetError());
        return false;
    }
    m_videoSubsystemInitialized = true;
    log::Info(kLogCategory, "video driver: {}", SDL_GetCurrentVideoDriver());

    // Ask for an OpenGL 3.3 core-profile context. These attributes must be set
    // before the window is created.
    const bool attributesOk =
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) &&
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) &&
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) &&
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (!attributesOk) {
        log::Error(kLogCategory, "SDL_GL_SetAttribute failed: {}", SDL_GetError());
        return false;
    }

    m_window = SDL_CreateWindow(config.windowTitle.c_str(), config.windowWidth, config.windowHeight,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (m_window == nullptr) {
        log::Error(kLogCategory, "SDL_CreateWindow failed: {}", SDL_GetError());
        return false;
    }

    // Creating the context also makes it current on this thread.
    m_glContext = SDL_GL_CreateContext(m_window);
    if (m_glContext == nullptr) {
        log::Error(kLogCategory, "SDL_GL_CreateContext failed (is an OpenGL 3.3 driver installed?): {}", SDL_GetError());
        return false;
    }

    // Vsync is a nice-to-have, so failure is a warning, not an error.
    if (!SDL_GL_SetSwapInterval(config.vsync ? 1 : 0)) {
        log::Warning(kLogCategory, "could not set swap interval (vsync): {}", SDL_GetError());
    }

    const FramebufferSize size = GetFramebufferSize();
    log::Info(kLogCategory, "window '{}' created: {}x{} px, vsync {}",
              config.windowTitle, size.width, size.height, config.vsync ? "on" : "off");
    return true;
}

Window::~Window() {
    if (m_glContext != nullptr) {
        SDL_GL_DestroyContext(m_glContext);
    }
    if (m_window != nullptr) {
        SDL_DestroyWindow(m_window);
    }
    if (m_videoSubsystemInitialized) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        log::Info(kLogCategory, "window and video subsystem destroyed");
    }
}

void Window::PollEvents() {
    // SDL collects everything that happened (key presses, mouse moves, the user
    // clicking the close button...) in a queue. We pull events out one at a time
    // until the queue is empty, and react to the ones we care about.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                RequestClose("quit event (window closed, Ctrl+C or SIGTERM)");
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                RequestClose("window close button");
                break;
            case SDL_EVENT_KEY_DOWN:
                if (m_quitOnEscape && event.key.key == SDLK_ESCAPE && !event.key.repeat) {
                    RequestClose("Escape pressed");
                }
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                log::Debug(kLogCategory, "framebuffer resized to {}x{}", event.window.data1, event.window.data2);
                break;
            default:
                break;  // everything else is ignored for now
        }
    }
}

void Window::RequestClose(const char* reason) {
    if (!m_shouldClose) {  // SDL can report the same close more than once
        log::Info(kLogCategory, "close requested: {}", reason);
    }
    m_shouldClose = true;
}

void Window::SwapBuffers() {
    if (!SDL_GL_SwapWindow(m_window)) {
        log::Error(kLogCategory, "SDL_GL_SwapWindow failed: {}", SDL_GetError());
    }
}

void Window::SetTitle(const std::string& title) {
    if (!SDL_SetWindowTitle(m_window, title.c_str())) {
        log::Warning(kLogCategory, "SDL_SetWindowTitle failed: {}", SDL_GetError());
    }
}

FramebufferSize Window::GetFramebufferSize() const {
    FramebufferSize size;
    if (!SDL_GetWindowSizeInPixels(m_window, &size.width, &size.height)) {
        log::Warning(kLogCategory, "SDL_GetWindowSizeInPixels failed: {}", SDL_GetError());
    }
    return size;
}

Window::GLLoader Window::GetGLLoader() {
    return &LoadGLFunction;
}

}  // namespace xray
