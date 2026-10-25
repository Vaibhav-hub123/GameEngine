#pragma once

#include <memory>

#include "xray/core/Color.h"

namespace xray {

class Window;

// Renderer owns "talking to OpenGL". Today that means: load the GL functions,
// clear the screen to a color each frame, and present the result.
//
// OpenGL is a state machine living inside the graphics driver. Before any GL
// call can work, (1) a GL context must exist and be current on this thread (the
// Window did that) and (2) we must find the address of each GL function, because
// the operating system does not hand them out automatically. GLAD, a small
// generated library, does that lookup for us.
//
// LIFETIME: a Renderer keeps a reference to the Window it draws into, so the
// Window must outlive it. Application guarantees this by destroying the Renderer first.
class Renderer {
public:
    // Returns nullptr (after logging the reason) if OpenGL cannot be initialised.
    static std::unique_ptr<Renderer> Create(Window& window, const Color& clearColor);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void SetClearColor(const Color& color) { m_clearColor = color; }

    // Frame bracket: BeginFrame() prepares and clears, EndFrame() presents.
    // Everything that draws goes in between.
    void BeginFrame();
    void EndFrame();

private:
    Renderer(Window& window, const Color& clearColor) : m_window(window), m_clearColor(clearColor) {}

    bool Initialize();
    void ReportGLErrors(const char* where);

    Window& m_window;
    Color m_clearColor;
    int m_errorReportsRemaining = 10;  // keeps a persistent error from flooding the log
};

}  // namespace xray
