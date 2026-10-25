#include <doctest/doctest.h>

#include "xray/core/FrameTimer.h"

// Times in these tests are powers of two (0.25, 0.125, ...) because they are
// represented exactly in floating point, so the tests can compare with == and
// never fail from rounding noise.

namespace {

int CountFixedSteps(xray::FrameTimer& timer) {
    int steps = 0;
    while (timer.ConsumeFixedStep()) {
        ++steps;
    }
    return steps;
}

}  // namespace

TEST_CASE("FrameTimer: the first Tick has zero duration and owes no steps") {
    xray::FrameTimer timer(0.25, 1.0);
    timer.Tick(100.0);  // the absolute value of the clock must not matter

    CHECK(timer.FrameSeconds() == 0.0);
    CHECK(timer.FrameCount() == 1);
    CHECK(CountFixedSteps(timer) == 0);
}

TEST_CASE("FrameTimer: one long frame owes several whole steps") {
    xray::FrameTimer timer(0.25, 10.0);
    timer.Tick(0.0);
    timer.Tick(0.75);

    CHECK(timer.FrameSeconds() == 0.75);
    CHECK(CountFixedSteps(timer) == 3);
    CHECK(timer.FixedStepCount() == 3);
    CHECK(timer.ConsumeFixedStep() == false);
}

TEST_CASE("FrameTimer: leftover time carries over to the next frame") {
    xray::FrameTimer timer(0.5, 10.0);
    timer.Tick(0.0);

    timer.Tick(0.25);  // 0.25 s owed: not enough for a 0.5 s step
    CHECK(CountFixedSteps(timer) == 0);
    CHECK(timer.InterpolationAlpha() == 0.5);

    timer.Tick(0.5);   // 0.5 s owed in total: exactly one step
    CHECK(CountFixedSteps(timer) == 1);
    CHECK(timer.InterpolationAlpha() == 0.0);
}

TEST_CASE("FrameTimer: a stall is clamped so it cannot cause a spiral of death") {
    xray::FrameTimer timer(0.25, 0.5);  // never count more than 0.5 s per frame
    timer.Tick(0.0);
    timer.Tick(10.0);                   // pretend the program froze for 10 s

    CHECK(timer.FrameSeconds() == 0.5);
    CHECK(CountFixedSteps(timer) == 2);
}

TEST_CASE("FrameTimer: a clock that goes backwards counts as zero time") {
    xray::FrameTimer timer(0.25, 1.0);
    timer.Tick(5.0);
    timer.Tick(4.0);

    CHECK(timer.FrameSeconds() == 0.0);
    CHECK(CountFixedSteps(timer) == 0);
}

TEST_CASE("FrameTimer: frames per second is measured over a 0.5 s window") {
    xray::FrameTimer timer(0.25, 1.0);
    timer.Tick(0.0);  // not counted: there is no previous frame to measure against

    timer.Tick(0.125);
    timer.Tick(0.25);
    timer.Tick(0.375);
    CHECK(timer.FpsUpdatedThisFrame() == false);
    CHECK(timer.FramesPerSecond() == 0.0);

    timer.Tick(0.5);  // 4 frames in 0.5 s -> 8 fps
    CHECK(timer.FpsUpdatedThisFrame() == true);
    CHECK(timer.FramesPerSecond() == 8.0);

    timer.Tick(0.625);
    CHECK(timer.FpsUpdatedThisFrame() == false);
    CHECK(timer.FramesPerSecond() == 8.0);  // keeps the last measurement
}

TEST_CASE("FrameTimer: NowSeconds never goes backwards") {
    const double first = xray::FrameTimer::NowSeconds();
    const double second = xray::FrameTimer::NowSeconds();
    CHECK(second >= first);
}
