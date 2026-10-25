#pragma once

#include <cstdint>

namespace xray {

// FrameTimer answers two questions for the game loop:
//
//   1. How long did the last frame take?
//   2. How many fixed-size simulation steps do we owe?
//
// WHY A FIXED STEP?
// Frames take a variable amount of time (a fast PC might render at 240 fps, a
// slow one at 30). If gameplay and physics were advanced by "however long the
// last frame took", results would differ from machine to machine, and bugs would
// become impossible to reproduce. Instead, the simulation always advances in
// identical slices (default 1/60 s). Every frame we add the real elapsed time to
// an "accumulator" (a bucket of owed time) and run as many whole slices as fit:
//
//     frame took 40 ms, step = 16.67 ms
//     accumulator: 0 + 40 = 40 ms  -> run 2 steps (33.33 ms), 6.67 ms stays in the bucket
//
// The leftover is not lost; it carries into the next frame. Running the same
// number of identical steps is also what will make recording and replaying
// gameplay possible later.
//
// WHY CLAMP THE FRAME TIME?
// If the program stalls (window dragged, debugger breakpoint) the next frame
// could report several seconds. Running hundreds of catch-up steps takes even
// longer, causing a longer stall, and so on: the "spiral of death". We cap the
// time one frame may add to the accumulator (maxFrameSeconds).
//
// FrameTimer never reads a clock by itself. The caller passes the current time
// to Tick(), which makes it trivial to unit-test with made-up times.
class FrameTimer {
public:
    explicit FrameTimer(double fixedStepSeconds = 1.0 / 30.0, double maxFrameSeconds = 0.25);

    // Monotonic clock in seconds. The starting point is arbitrary, so only
    // differences between two readings are meaningful.
    static double NowSeconds();

    // Call exactly once at the start of every frame.
    void Tick(double nowSeconds);

    // Call in a loop after Tick(): returns true once for each fixed step owed.
    //     while (timer.ConsumeFixedStep()) { Simulate(timer.FixedStepSeconds()); }
    bool ConsumeFixedStep();

    double FixedStepSeconds() const { return m_fixedStepSeconds; }

    // Duration of the last frame, after clamping to maxFrameSeconds.
    // This is what was added to the accumulator. It is 0 on the very first frame.
    double FrameSeconds() const { return m_frameSeconds; }

    // How far the accumulator has progressed into the next fixed step (0 to <1).
    // A renderer can use it to draw between two simulation states; unused so far.
    double InterpolationAlpha() const { return m_accumulatorSeconds / m_fixedStepSeconds; }

    std::uint64_t FrameCount() const { return m_frameCount; }
    std::uint64_t FixedStepCount() const { return m_fixedStepCount; }

    // Frames per second, averaged over the last measurement window (0.5 s).
    // Uses the real (unclamped) frame times, so stalls show up honestly.
    double FramesPerSecond() const { return m_framesPerSecond; }

    // True only during the frame in which FramesPerSecond() got a new value.
    bool FpsUpdatedThisFrame() const { return m_fpsUpdatedThisFrame; }

private:
    static constexpr double kFpsWindowSeconds = 0.5;

    double m_fixedStepSeconds;
    double m_maxFrameSeconds;

    bool m_hasPreviousTime = false;
    double m_previousTimeSeconds = 0.0;
    double m_frameSeconds = 0.0;
    double m_accumulatorSeconds = 0.0;

    std::uint64_t m_frameCount = 0;
    std::uint64_t m_fixedStepCount = 0;

    double m_fpsWindowSeconds = 0.0;
    std::uint64_t m_fpsWindowFrames = 0;
    double m_framesPerSecond = 0.0;
    bool m_fpsUpdatedThisFrame = false;
};

}  // namespace xray
