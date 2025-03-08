#include "util.h"

float clamp(float value, float minVal, float maxVal) {
    return std::fmax(minVal, std::fmin(value, maxVal));
}
