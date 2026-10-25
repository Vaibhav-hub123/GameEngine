#pragma once

namespace xray {

// An RGBA color with each channel in the range 0.0 to 1.0.
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

}  // namespace xray
