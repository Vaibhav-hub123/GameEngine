#include "xray/Application.h"

#include <cstdlib>
#include <format>
#include <utility>

#include "xray/core/Log.h"
#include "xray/platform/Window.h"
#include "xray/renderer/Renderer.h"

namespace xray {
namespace {

constexpr std::string_view kLogCategory = "App";

// Rejects settings that would break the loop, with a message saying which one.
bool ValidateConfig(const EngineConfig& config) {
    bool valid = true;
    if (config.windowWidth <= 0 || config.windowHeight <= 0) {
        log::Error(kLogCategory, "invalid config: window size must be positive (got {}x{})",
                   config.windowWidth, config.windowHeight);
        valid = false;
    }
    if (config.fixedStepSeconds <= 0.0) {
        log::Error(kLogCategory, "invalid config: fixedStepSeconds must be > 0 (got {})", config.fixedStepSeconds);
        valid = false;
    }
    if (config.maxFrameSeconds < config.fixedStepSeconds) {
        log::Error(kLogCategory, "invalid config: maxFrameSeconds ({}) must be >= fixedStepSeconds ({})",
                   config.maxFrameSeconds, config.fixedStepSeconds);
        valid = false;
    }
    return valid;
}

}  // namespace

Application::Application(EngineConfig config) : m_config(std::move(config)) {}

// Defined here, not in the header, because destroying the unique_ptr members
// requires Window and Renderer to be complete types.
Application::~Application() = default;

int Application::Run() {
    log::Info(kLogCategory, "XRay Engine starting");

    if (!Initialize()) {
        log::Error(kLogCategory, "initialisation failed; shutting down");
        Shutdown();
        return EXIT_FAILURE;
    }

    RunLoop();
    Shutdown();
    return EXIT_SUCCESS;
}

bool Application::Initialize() {
    if (!ValidateConfig(m_config)) {
        return false;
    }
    m_timer = FrameTimer(m_config.fixedStepSeconds, m_config.maxFrameSeconds);

    // Order matters: the renderer needs the window's OpenGL context.
    m_window = Window::Create(m_config);
    if (!m_window) {
        log::Error(kLogCategory, "could not create the window");
        return false;
    }

    m_renderer = Renderer::Create(*m_window, m_config.clearColor);
    if (!m_renderer) {
        log::Error(kLogCategory, "could not initialise the renderer");
        return false;
    }
    return true;
}

void Application::RunLoop() {
    log::Info(kLogCategory, "entering main loop (fixed step {:.3f} ms)", m_config.fixedStepSeconds * 1000.0);

    // One iteration = one rendered frame:
    //   1. measure time          (how much real time passed?)
    //   2. process events        (did the user do or close anything?)
    //   3. fixed-step updates    (catch the simulation up to real time)
    //   4. render                (draw the current state and present it)
    while (!m_quitRequested && !m_window->ShouldClose()) {
        m_timer.Tick(FrameTimer::NowSeconds());

        m_window->PollEvents();
        if (m_window->ShouldClose()) {
            break;
        }

        while (m_timer.ConsumeFixedStep()) {
            OnFixedUpdate(m_timer.FixedStepSeconds());
        }

        m_renderer->BeginFrame();
        OnRender();
        m_renderer->EndFrame();

        if (m_timer.FpsUpdatedThisFrame()) {
            const RendererStats& stats = m_renderer->GetLastFrameStats();
            log::Trace(kLogCategory, "{:.1f} FPS, {} draw calls, {} vertices", m_timer.FramesPerSecond(),
                       stats.drawCalls, stats.vertices);
            m_window->SetTitle(std::format("{} | {:.1f} FPS | {} draw calls, {} vertices", m_config.windowTitle,
                                           m_timer.FramesPerSecond(), stats.drawCalls, stats.vertices));
        }

        if (m_config.maxFrames != 0 && m_timer.FrameCount() >= m_config.maxFrames) {
            log::Info(kLogCategory, "reached maxFrames ({}); stopping", m_config.maxFrames);
            break;
        }
    }

    log::Info(kLogCategory, "left main loop after {} frames and {} fixed steps",
              m_timer.FrameCount(), m_timer.FixedStepCount());
}

void Application::Shutdown() {
    // Reverse order of creation: the renderer uses the window, so it goes first.
    m_renderer.reset();
    m_window.reset();
    log::Info(kLogCategory, "shutdown complete");
}

void Application::OnFixedUpdate(double /*fixedDeltaSeconds*/) {}

void Application::OnRender() {}

}  // namespace xray
