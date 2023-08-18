#pragma once

float clamp(float value) {
    if (value < 0) {
        return 0;
    }
    else {
        return value;
    }
}

int ceil(float value) { // NOTE wont work for negative numbers
    return (int)(value + 0.99);
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

uint32_t pow(int base, int exponent) {
    int retVal = 1;
    for (int i = 1; i <= exponent; i++) {
        retVal *= base;
    }
    return retVal;
}