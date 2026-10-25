#pragma once

#include <memory>

#include "xray/core/EngineConfig.h"
#include "xray/core/FrameTimer.h"

namespace xray {

class Window;
class Renderer;

// Application is the engine's entry point and owner of everything else.
// It sits at the top of the dependency chain (it knows about the window and the
// renderer; they do not know about it), which is why it lives directly in xray/
// rather than in core/.
//
// A game is a class that inherits from Application and overrides the hooks:
//
//     class MyGame : public xray::Application {
//         void OnFixedUpdate(double dt) override { ... }
//     };
//     int main() { MyGame game(config); return game.Run(); }
//
// Run() performs the whole lifecycle: initialise -> loop until quit -> shut down.
class Application {
public:
    explicit Application(EngineConfig config);
    virtual ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Runs the program. Returns 0 on a clean run, non-zero if start-up failed.
    int Run();

    // Ask the loop to stop after the current frame.
    void RequestQuit() { m_quitRequested = true; }

protected:
    // Advance the simulation by exactly fixedDeltaSeconds. Called zero, one or
    // several times per frame, depending on how much time has passed.
    virtual void OnFixedUpdate(double fixedDeltaSeconds);

    // Draw the frame. Called once per frame, between BeginFrame and EndFrame.
    virtual void OnRender();

    // Valid only while Run() is executing (i.e. from inside the hooks).
    Renderer& GetRenderer() { return *m_renderer; }
    const FrameTimer& GetTimer() const { return m_timer; }
    const EngineConfig& GetConfig() const { return m_config; }

private:
    bool Initialize();
    void RunLoop();
    void Shutdown();

    EngineConfig m_config;
    FrameTimer m_timer;

    // Member order matters: members are destroyed in REVERSE order, so the
    // renderer (which uses the window) is destroyed before the window.
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;

    bool m_quitRequested = false;
};

}  // namespace xray
