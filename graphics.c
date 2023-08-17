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

void drawNumber(uint32_t* bufferMemory, const Image* image, float x, float y, int screenWidth, int screenHeight, int frame = 0, int scale = 1, uint32_t color = BLACK) {
    // TODO: should x and y be ints?
    Assert(frame >= 0);
    Assert(scale >= 1);
    Assert(frame <= image->hframes);

    if (!image->pixelPointer) {
        OutputDebugString("Can not display null image\n");
        return;
    }

    int frameWidth = (image->width / image->hframes); // TODO: check handling image widths that are odd. (ie 23)  

    int sampleHeight = image->height;
    int sampleWidth = frameWidth;

    int renderHeight = sampleHeight * scale;
    int renderWidth = sampleWidth * scale;

    if (renderHeight + y > screenHeight) {
        int diff = renderHeight + (int)y - screenHeight;
        sampleHeight -= diff / scale;
        renderHeight = (int)(screenHeight - y);
    }
    if (renderWidth + x > screenWidth) {
        renderWidth = (int)(screenWidth - x);
    }
    // Go to upper left corner.
    bufferMemory += (int)clamp((float)screenWidth * (screenHeight - (renderHeight + roundFloat(clamp(y)))));
    bufferMemory += roundFloat(clamp(x));

    uint32_t* pixelPointer = image->pixelPointer;
    pixelPointer += (sampleHeight - 1) * image->width; // Go to end row of bmp (it's inverted)

    if (x < 0) {
        renderWidth += roundFloat(x); // Reducing width
        pixelPointer -= roundFloat(x); // Advancing from where to sample
    }
    if (y < 0) {
        renderHeight += roundFloat(y); // Reducing height
        // No need to advance from where to sample here
        bufferMemory -= screenWidth * (roundFloat(y));
    }

    int strideToNextRow = screenWidth - renderWidth;
    if (strideToNextRow < 0) {
        strideToNextRow = 0;
    }

    for (int j = 0; j < renderHeight; j++) {
        pixelPointer += frameWidth * frame; // Advance to proper frame
        for (int i = 0; i < renderWidth; i++) {
            uint32_t colorValue;
            if (*pixelPointer == 0) {
                colorValue = 0;
            }
            else {
                colorValue = color;
            }
            float alphaValue = (colorValue >> 24) / 255.0f;
            uint32_t redValueS = (colorValue & 0xFF0000) >> 16;
            uint32_t greenValueS = (colorValue & 0x00FF00) >> 8;
            uint32_t blueValueS = (colorValue & 0x0000FF);

            uint32_t redValueD = (*bufferMemory & 0xFF0000) >> 16;
            uint32_t greenValueD = (*bufferMemory & 0x00FF00) >> 8;
            uint32_t blueValueD = *bufferMemory & 0x0000FF;

            uint32_t interpolatedPixel = (uint32_t)(alphaValue * redValueS + (1 - alphaValue) * redValueD) << 16
                | (uint32_t)(alphaValue * greenValueS + (1 - alphaValue) * greenValueD) << 8
                | (uint32_t)(alphaValue * blueValueS + (1 - alphaValue) * blueValueD);

            *bufferMemory = interpolatedPixel;
            bufferMemory++;
            if (scale == 1) {
                pixelPointer++; // left to right
            }
            else {
                if ((i % scale) == 1) { // Advance pointer every "scale" steps
                    pixelPointer++; // left to right
                }
            }
        }
        pixelPointer += image->width - sampleWidth * (frame + 1); // Remainder to get to end of row
        if (scale == 1) {
            pixelPointer -= 2 * image->width;
        }
        else {
            if ((j % scale) == 1) {
                pixelPointer -= 2 * image->width; // start at the top, go down (Since BMPs are inverted) TODO: just load them in the right order
            }
            else {
                pixelPointer -= image->width;
            }

        }
        bufferMemory += strideToNextRow;
    }
}
void drawImage(uint32_t* bufferMemory, const Image* image, float x, float y, int screenWidth, int screenHeight, int frame = 0, bool flip = 0, int scale = 1) {
    // TODO: should x and y be ints?
    Assert(frame >= 0);
    Assert(scale >= 1);
    Assert(frame <= image->hframes);
    // TODO: implement offset.
    //y += image->offset.y;
    //x += image->offset.x;

    if (!image->pixelPointer) {
        OutputDebugString("Can not display null image\n");
        return;
    }


    int frameWidth = (image->width / image->hframes); // TODO: check handling image widths that are odd. (ie 23)  

    int sampleHeight = image->height;
    int sampleWidth = frameWidth ;

    int renderHeight = sampleHeight * scale;
    int renderWidth = sampleWidth * scale;

    if (renderHeight + y > screenHeight) {
        int diff = renderHeight + (int)y - screenHeight;
        sampleHeight -= diff / scale;
        renderHeight = (int)(screenHeight - y);
    }
    if (renderWidth + x > screenWidth) {
#if 0 // Not needed for this game
        int diff = renderWidth + x - sampleWidth;
        sampleWidth -= diff / scale;
#endif
        renderWidth = (int)(screenWidth - x);
    }    
    // Go to upper left corner.
    bufferMemory += (int)clamp((float)screenWidth * (screenHeight - (renderHeight + roundFloat(clamp(y)))));
    bufferMemory += roundFloat(clamp(x));

    uint32_t* pixelPointer = image->pixelPointer;
    pixelPointer += (sampleHeight - 1) * image->width; // Go to end row of bmp (it's inverted)
    
    if (x < 0) {
        renderWidth += roundFloat(x); // Reducing width
        pixelPointer -= roundFloat(x); // Advancing from where to sample
    }
    if (y < 0) {
        renderHeight += roundFloat(y); // Reducing height
        // No need to advance from where to sample here
        bufferMemory -= screenWidth * (roundFloat(y));
    }

    int strideToNextRow = screenWidth - renderWidth;
    if (strideToNextRow < 0) {
        strideToNextRow = 0;
    }

    for (int j = 0; j < renderHeight; j++) {
        pixelPointer += frameWidth * frame; // Advance to proper frame
        if (flip) {
            pixelPointer += frameWidth;
        }
        for (int i = 0; i < renderWidth; i++) {
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
            if (scale == 1) {
                if (flip) {
                    pixelPointer--; // right to left
                }
                else {
                    pixelPointer++; // left to right
                }
            }
            else {
                if ((i % scale) == 1) { // Advance pointer every "scale" steps
                    if (flip) {
                        pixelPointer--; // right to left
                    }
                    else {
                        pixelPointer++; // left to right
                    }
                }
            }
        }
        if (flip) {
            pixelPointer += frameWidth;
        }
        pixelPointer += image->width - sampleWidth*(frame+1); // Remainder to get to end of row
        if (scale == 1) {
            pixelPointer -= 2 * image->width;
        }
        else {
            if ((j % scale) == 1) {
                pixelPointer -= 2 * image->width; // start at the top, go down (Since BMPs are inverted) TODO: just load them in the right order
            }
            else {
                pixelPointer -= image->width;
            }

        }
        bufferMemory += strideToNextRow;
    }
}