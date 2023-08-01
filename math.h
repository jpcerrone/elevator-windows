#pragma once

float clamp(float value) {
    if (value < 0) {
        return 0;
    }
    else {
        return value;
    }
}