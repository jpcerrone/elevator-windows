#pragma once

float clamp(float value) {
    if (value < 0) {
        return 0;
    }
    else {
        return value;
    }
}

#if 0
float getRandomFloat(float min, float max) {
    static int resolution = 100; // Assumes 100-step resolution.
    int randomInt = rand() % resolution; //000 to 10
    float randomFloat = randomInt / (float)resolution;
    float floatBetweenMinMax = randomFloat * (max - min) + min;
    return floatBetweenMinMax;
}
#endif
