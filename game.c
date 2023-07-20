#include <cstdint>
#include "vector2i.c"
#include <stdio.h>
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

struct GameInput {
    union {
        bool buttons[9];
        struct {
            bool button9;
            bool button8;
            bool button7;
            bool button6;
            bool button5;
            bool button4;
            bool button3;
            bool button2;
            bool button1;
        };
    };

};

struct ButtonStates {
    union {
        bool buttons[9];
        struct {
            bool button9;
            bool button8;
            bool button7;
            bool button6;
            bool button5;
            bool button4;
            bool button3;
            bool button2;
            bool button1;
        };
    };
};

struct GameMemory {
    bool isInitialized = false;
    ButtonStates buttonStates;
};

void updateAndRender(void* bitMapMemory, int screenWidth, int screenHeight, GameInput input, GameMemory *memory, float delta) {
    if (!memory->isInitialized) {
        memory->isInitialized = true;
        memory->buttonStates = {};
    }
    fillBGWithColor(bitMapMemory, screenWidth, screenHeight, 0xFFFFFFFF);
    Vector2i elevatorDim = { 78,90 };
    drawRectangle(bitMapMemory, screenWidth, screenHeight, (screenWidth-elevatorDim.width)/2 , 
        (screenHeight - elevatorDim.height) / 2, (screenWidth + elevatorDim.width) / 2, (screenHeight + elevatorDim.height) / 2, 1.0, 0.0, 0.0);

    for (int i = 0; i < 9; i++) {
        if (input.buttons[i]) {
            memory->buttonStates.buttons[i] = !memory->buttonStates.buttons[i];
        }
    }

#if SHOWBUTTONSTATES
    char buttonsString[10];
    for (int i = 0; i < 9; i++) {
        if (memory->buttonStates.buttons[i]) {
            buttonsString[i] = '1';
        }
        else {
            buttonsString[i] = '0';
        }
    }
    buttonsString[9] = '\0';
    OutputDebugString(buttonsString);
    OutputDebugStringW(L"\n");
#endif
}

