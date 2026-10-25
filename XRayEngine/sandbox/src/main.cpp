// The sandbox is a tiny "game" built on top of the engine. In Phase 1 it only
// proves the foundation works: a window opens, the fixed-step loop runs, the
// screen is cleared every frame, and everything shuts down cleanly.

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <string_view>
#include <system_error>

#include "xray/Application.h"
#include "xray/core/EngineConfig.h"
#include "xray/core/Log.h"
#include "xray/renderer/Renderer.h"

namespace {

constexpr std::string_view kLogCategory = "Sandbox";

class SandboxApp final : public xray::Application {
public:
    using Application::Application;

protected:
    // Runs at a fixed 60 Hz no matter how fast frames are rendered. The state
    // that changes here (simulated time) drives what is drawn.
    void OnFixedUpdate(double fixedDeltaSeconds) override {
        m_simulatedSeconds += fixedDeltaSeconds;

        // Once per simulated second, report the progress. The number of fixed
        // steps per simulated second should always be ~60 (the fixed step rate),
        // even if the renderer runs at 144 fps.
        if (m_simulatedSeconds + 1e-9 >= m_nextReportSeconds) {
            xray::log::Info(kLogCategory, "{:.0f} s simulated, {} fixed steps so far",
                            m_nextReportSeconds, GetTimer().FixedStepCount());
            m_nextReportSeconds += 1.0;
        }

        // Make the clear color pulse slowly so you can SEE the loop is alive.
        const double t = m_simulatedSeconds;
        xray::Color color;
        color.r = static_cast<float>(0.08 + 0.05 * std::sin(t * 0.9));
        color.g = static_cast<float>(0.10 + 0.05 * std::sin(t * 0.6 + 1.0));
        color.b = static_cast<float>(0.16 + 0.07 * std::sin(t * 0.4 + 2.0));
        GetRenderer().SetClearColor(color);
    }

private:
    double m_simulatedSeconds = 0.0;
    double m_nextReportSeconds = 1.0;
};

void PrintUsage() {
    xray::log::Info(kLogCategory, "usage: xray_sandbox [--frames=N]");
    xray::log::Info(kLogCategory, "  --frames=N   exit by itself after N rendered frames (for smoke tests)");
}

}  // namespace

int main(int argc, char** argv) {
    xray::EngineConfig config;
    config.windowTitle = "XRay Sandbox";

    for (int i = 1; i < argc; ++i) {
        const std::string_view argument = argv[i];
        constexpr std::string_view framesPrefix = "--frames=";

        if (argument.starts_with(framesPrefix)) {
            const std::string_view number = argument.substr(framesPrefix.size());
            const auto result = std::from_chars(number.data(), number.data() + number.size(), config.maxFrames);
            if (result.ec != std::errc{} || result.ptr != number.data() + number.size()) {
                xray::log::Error(kLogCategory, "invalid value for --frames: '{}'", number);
                return EXIT_FAILURE;
            }
        } else if (argument == "--help") {
            PrintUsage();
            return EXIT_SUCCESS;
        } else {
            xray::log::Error(kLogCategory, "unknown argument '{}'", argument);
            PrintUsage();
            return EXIT_FAILURE;
        }
    }

    try {
        SandboxApp app(config);
        return app.Run();
    } catch (const std::exception& exception) {
        xray::log::Error(kLogCategory, "unhandled exception: {}", exception.what());
        return EXIT_FAILURE;
    }
}
