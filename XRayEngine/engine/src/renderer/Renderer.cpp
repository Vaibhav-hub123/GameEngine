#include "xray/renderer/Renderer.h"

#include <glad/gl.h>

#include "xray/core/Log.h"
#include "xray/platform/Window.h"

namespace xray {
namespace {

constexpr std::string_view kLogCategory = "Renderer";

// glGetString returns unsigned char*; this turns it into a printable string.
const char* GLString(GLenum name) {
    const GLubyte* text = glGetString(name);
    return text != nullptr ? reinterpret_cast<const char*>(text) : "(unknown)";
}

}  // namespace

std::unique_ptr<Renderer> Renderer::Create(Window& window, const Color& clearColor) {
    std::unique_ptr<Renderer> renderer(new Renderer(window, clearColor));
    if (!renderer->Initialize()) {
        return nullptr;
    }
    return renderer;
}

bool Renderer::Initialize() {
    // GLAD asks the loader for the address of every OpenGL 3.3 function.
    const int version = gladLoadGL(reinterpret_cast<GLADloadfunc>(Window::GetGLLoader()));
    if (version == 0) {
        log::Error(kLogCategory, "gladLoadGL failed: could not load OpenGL functions from the driver");
        return false;
    }

    log::Info(kLogCategory, "OpenGL {}.{} loaded", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    log::Info(kLogCategory, "GL_VERSION:  {}", GLString(GL_VERSION));
    log::Info(kLogCategory, "GL_VENDOR:   {}", GLString(GL_VENDOR));
    log::Info(kLogCategory, "GL_RENDERER: {}", GLString(GL_RENDERER));

    ReportGLErrors("Renderer::Initialize");
    return true;
}

void Renderer::BeginFrame() {
    // The viewport maps GL's -1..1 coordinate space onto the window's pixels.
    // Setting it every frame keeps it correct after the window is resized.
    const FramebufferSize size = m_window.GetFramebufferSize();
    glViewport(0, 0, size.width, size.height);

    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::EndFrame() {
    ReportGLErrors("frame");
    m_window.SwapBuffers();
}

void Renderer::ReportGLErrors(const char* where) {
    // OpenGL does not stop or throw when something goes wrong; it just records
    // an error flag that we must ask for. Several flags can be queued, so loop.
    for (int i = 0; i < 8; ++i) {
        const GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            return;
        }
        if (m_errorReportsRemaining > 0) {
            log::Error(kLogCategory, "OpenGL error 0x{:04X} (in {})", error, where);
            if (--m_errorReportsRemaining == 0) {
                log::Warning(kLogCategory, "too many OpenGL errors; further reports are suppressed");
            }
        }
    }
}

}  // namespace xray
