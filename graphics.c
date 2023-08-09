#include "graphics.h"
#include "assertions.h"
int roundFloat(float value) {
    return (int)(value + 0.5f);
}

uint32_t roundUFloat(float value) {
    return (uint32_t)(value + 0.5f);
}

// Using int's since it's pixel art
void drawRectangle(void* outputStream, int bmWidth, int bmHeight, int minX, int minY, int maxX, int maxY, float r, float g, float b) {
    uint32_t* pixel = (uint32_t*)outputStream;

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

void drawImage(uint32_t* bufferMemory, const Image* image, float x, float y, int screenWidth, int screenHeight, int frame = 0) {
    Assert(frame >= 0);
    Assert(frame <= image->hframes);
    // TODO: implement offset.
    //y += image->offset.y;
    //x += image->offset.x;

    if (!image->pixelPointer) {
        OutputDebugString("Can not display null image\n");
        return;
    }

    int frameWidth = image->width / image->hframes; // TODO: check handling image widths that are odd. (ie 23)  

    int drawHeight = image->height;
    int drawWidth = frameWidth;
    if (drawHeight + y > screenHeight) {
        drawHeight = (int)(screenHeight - y);
    }
    if (drawWidth + x > screenWidth) {
        drawWidth = (int)(screenWidth - x);
    }
    // Go to upper left corner.
    bufferMemory += (int)clamp((float)screenWidth * (screenHeight - (image->height + roundFloat(clamp(y)))));
    bufferMemory += roundFloat(clamp(x));

    uint32_t* pixelPointer = image->pixelPointer;
    pixelPointer += (drawHeight - 1) * image->width; // Go to end row of bmp (it's inverted)
    

    if (x < 0) {
        drawWidth += roundFloat(x);
        pixelPointer -= roundFloat(x);
    }
    if (y < 0) {
        drawHeight += roundFloat(y);
        bufferMemory -= screenWidth * (roundFloat(y));
    }

    int strideToNextRow = screenWidth - drawWidth;
    if (strideToNextRow < 0) {
        strideToNextRow = 0;
    }

    for (int j = 0; j < drawHeight; j++) {
        pixelPointer += frameWidth * frame;
        for (int i = 0; i < drawWidth; i++) {
            float alphaValue = (*pixelPointer >> 24) / 255.0f;
            uint32_t redValueS = (*pixelPointer & 0xFF0000) >> 16;
            uint32_t greenValueS = (*pixelPointer & 0x00FF00) >> 8;
            uint32_t blueValueS = (*pixelPointer & 0x0000FF);

            uint32_t redValueD = (*bufferMemory & 0xFF0000) >> 16;
            uint32_t greenValueD = (*bufferMemory & 0x00FF00) >> 8;
            uint32_t blueValueD = *bufferMemory & 0x0000FF;

            uint32_t interpolatedPixel = (uint32_t)(alphaValue * redValueS + (1 - alphaValue) * redValueD) << 16
                | (uint32_t)(alphaValue * greenValueS + (1 - alphaValue) * greenValueD) << 8
                | (uint32_t)(alphaValue * blueValueS + (1 - alphaValue) * blueValueD);

            *bufferMemory = interpolatedPixel;
            bufferMemory++;
            pixelPointer++; // left to right
        }
        pixelPointer += image->width - drawWidth*(frame+1); // Remainder to get to end of row
        pixelPointer -= 2 * image->width; // start at the top, go down (Since BMPs are inverted) TODO: just load them in the right order
        bufferMemory += strideToNextRow;
    }
}