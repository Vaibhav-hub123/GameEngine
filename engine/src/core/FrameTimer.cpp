#include "xray/core/FrameTimer.h"

#include <algorithm>
#include <cassert>
#include <chrono>

namespace xray {

FrameTimer::FrameTimer(double fixedStepSeconds, double maxFrameSeconds)
    : m_fixedStepSeconds(fixedStepSeconds), m_maxFrameSeconds(maxFrameSeconds) {
    // Application validates the configuration and reports a readable error before
    // it gets here; this only guards against programming mistakes.
    assert(fixedStepSeconds > 0.0);
    assert(maxFrameSeconds > 0.0);
}

double FrameTimer::NowSeconds() {
    // steady_clock never jumps backwards (unlike the wall clock, which the user
    // or the OS may adjust), which is exactly what a game loop needs.
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration<double>(now).count();
}

void FrameTimer::Tick(double nowSeconds) {
    m_fpsUpdatedThisFrame = false;

    double realFrameSeconds = 0.0;
    if (m_hasPreviousTime) {
        realFrameSeconds = std::max(0.0, nowSeconds - m_previousTimeSeconds);

        // Measure FPS from real durations. The very first Tick has no previous
        // time to compare with, so it is not counted.
        m_fpsWindowSeconds += realFrameSeconds;
        ++m_fpsWindowFrames;
        if (m_fpsWindowSeconds >= kFpsWindowSeconds) {
            m_framesPerSecond = static_cast<double>(m_fpsWindowFrames) / m_fpsWindowSeconds;
            m_fpsWindowSeconds = 0.0;
            m_fpsWindowFrames = 0;
            m_fpsUpdatedThisFrame = true;
        }
    }
    m_hasPreviousTime = true;
    m_previousTimeSeconds = nowSeconds;

    m_frameSeconds = std::min(realFrameSeconds, m_maxFrameSeconds);
    m_accumulatorSeconds += m_frameSeconds;
    ++m_frameCount;
}

bool FrameTimer::ConsumeFixedStep() {
    if (m_accumulatorSeconds < m_fixedStepSeconds) {
        return false;
    }
    m_accumulatorSeconds -= m_fixedStepSeconds;
    ++m_fixedStepCount;
    return true;
}

}  // namespace xray
