#pragma once

#include <cstdint>
#include <string>

#include "xray/core/Color.h"

namespace xray {

// Every tunable value of the engine lives here, in one plain struct with
// defaults. The sandbox creates one, changes what it wants, and hands it to
// Application. There is deliberately no config *file* yet; when we need one,
// it will fill in this same struct.
struct EngineConfig {
    // --- Window -----------------------------------------------------------
    std::string windowTitle = "XRay Engine";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool vsync = true;          // wait for the monitor refresh when presenting
    bool quitOnEscape = true;   // convenience while developing

    // --- Rendering --------------------------------------------------------
    Color clearColor{0.08f, 0.09f, 0.12f, 1.0f};

    // --- Game loop timing -------------------------------------------------
    // Simulation advances in fixed slices of this many seconds (1/60 s).
    double fixedStepSeconds = 1.0 / 60.0;
    // A single frame is never allowed to count for more than this much time.
    // Prevents the "spiral of death" after a long stall (see FrameTimer.h).
    double maxFrameSeconds = 0.25;

    // --- Testing hook -----------------------------------------------------
    // 0 means "run until the user closes the window". Any other value makes the
    // loop exit by itself after that many rendered frames, which lets scripts
    // smoke-test start-up and shutdown without a human clicking the close button.
    std::uint64_t maxFrames = 0;
};

}  // namespace xray
