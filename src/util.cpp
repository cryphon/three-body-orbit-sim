#ifndef UTIL_HPP
#define UTIL_HPP

#include <cmath>
#include <GLFW/glfw3.h>

float clamp(float value, float minVal, float maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}

#endif
