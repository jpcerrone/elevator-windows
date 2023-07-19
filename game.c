#include <cstdint>
#include "vector2i.c"

int roundFloat(float value) {
    return (int)(value + 0.5f);
}

uint32_t roundUFloat(float value) {
    return (uint32_t)(value + 0.5f);
}

// Using int's since it's pixel art
void drawRectangle(void* bitmap, int bmWidth, int bmHeight, int minX, int minY, int maxX, int maxY, float r, float g, float b) {
    uint32_t* pixel = (uint32_t*)bitmap;

    if (minX < 0)
        minX = 0;
    if (minY < 0)
        minY = 0;
    if (maxX > bmWidth)
        maxX = bmWidth;
    if (maxY > bmHeight)
        maxY = bmHeight;

    int tmp = maxY;
    maxY = bmHeight - minY;
    minY = bmHeight - tmp;

    uint32_t color = (0xFF << 24) | (roundUFloat(r * 255.0f) << 16) | (roundUFloat(g * 255.0f) << 8) | (roundUFloat(b * 255.0f) << 0); //0xAA RR GG BB 0b1111

    // Go to upper left corner.
    pixel += bmWidth * minY;
    pixel += minX;

    int recWidth = maxX - minX;
    int recHeight = maxY - minY;

    for (int y = 0; y < recHeight; y++) {
        for (int x = 0; x < recWidth; x++) {
            *pixel = color;
            pixel++;
        }
        pixel += bmWidth - recWidth;
    }
}

void fillBGWithColor(void* bitMapMemory, int width, int height, uint32_t color = 0x00000000) {
    uint32_t* pixel = (uint32_t*)bitMapMemory;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            *pixel = color;
            pixel++;
        }
    }
}

void updateAndRender(void* bitMapMemory, int screenWidth, int screenHeight) {
    fillBGWithColor(bitMapMemory, screenWidth, screenHeight, 0xFFFFFFFF);
    Vector2i elevatorDim = { 78,90 };
    drawRectangle(bitMapMemory, screenWidth, screenHeight, (screenWidth-elevatorDim.width)/2 , 
        (screenHeight - elevatorDim.height) / 2, (screenWidth + elevatorDim.width) / 2, (screenHeight + elevatorDim.height) / 2, 1.0, 0.0, 0.0);
}

