#include <cstdint>
#include <stdio.h>

#include "game.h"
#include "vector2i.c"
#include "graphics.c"
#include "platform.h"
#include "intrinsics.h"
#include "assertions.h"

#pragma pack(push, 1)
struct BitmapHeader {
    uint16_t FileType;
    uint32_t FileSize;
    uint16_t Reserved1;
    uint16_t Reserved2;
    uint32_t BitmapOffset;
    uint32_t Size;            /* Size of this header in bytes */
    int32_t  Width;           /* Image width in pixels */
    int32_t  Height;          /* Image height in pixels */
    uint16_t  Planes;          /* Number of color planes */
    uint16_t  BitsPerPixel;    /* Number of bits per pixel */
    uint32_t Compression;     /* Compression methods used */
    uint32_t SizeOfBitmap;    /* Size of bitmap in bytes */
    int32_t  HorzResolution;  /* Horizontal resolution in pixels per meter */
    int32_t  VertResolution;  /* Vertical resolution in pixels per meter */
    uint32_t ColorsUsed;      /* Number of colors in the image */
    uint32_t ColorsImportant; /* Minimum number of important colors */
    /* Fields added for Windows 4.x follow this line */

    uint32_t RedMask;       /* Mask identifying bits of red component */
    uint32_t GreenMask;     /* Mask identifying bits of green component */
    uint32_t BlueMask;      /* Mask identifying bits of blue component */
    uint32_t AlphaMask;     /* Mask identifying bits of alpha component */
};
#pragma pack(pop)

Image loadBMP(char* path, readFile_t* readFunction, int hframes = 1) {
    FileReadResult result = readFunction(path);
    Image retImage = {};
    BitmapHeader* header = (BitmapHeader*)(result.memory);

    Assert(header->Compression == 3);

    retImage.pixelPointer = (uint32_t*)((uint8_t*)result.memory + header->BitmapOffset);
    retImage.width = header->Width;
    retImage.height = header->Height;

    // Modify loaded bmp to set its pixels in the right order. Our pixel format is AARRGGBB, but bmps may vary because of their masks.
    int redOffset = findFirstSignificantBit(header->RedMask);
    int greenOffset = findFirstSignificantBit(header->GreenMask);
    int blueOffset = findFirstSignificantBit(header->BlueMask);
    int alphaOffset = findFirstSignificantBit(header->AlphaMask);

    uint32_t* modifyingPixelPointer = retImage.pixelPointer;
    for (int j = 0; j < header->Height; j++) {
        for (int i = 0; i < header->Width; i++) {
            int newRedValue = ((*modifyingPixelPointer & header->RedMask) >> redOffset) << 16;
            int newGreenValue = ((*modifyingPixelPointer & header->GreenMask) >> greenOffset) << 8;
            int newBlueValue = ((*modifyingPixelPointer & header->BlueMask) >> blueOffset) << 0;
            int newAlphaValue = ((*modifyingPixelPointer & header->AlphaMask) >> alphaOffset) << 24;

            *modifyingPixelPointer = newAlphaValue | newRedValue | newGreenValue | newBlueValue; //OG RRGGBBAA
            modifyingPixelPointer++;
        }
    }

    retImage.hframes = hframes;

    return retImage;
}

static const float startingSpeed = 150;

void setNextDirection(GameState *state) {
    // Get next destination
    int currentNext = state->currentFloor;
    int currentMin = INT32_MAX;
    for (int i = 1; i < 11; i++) {
        if (state->floorStates[i] == true) {
            int newMin = abs(state->currentFloor - i);
            if (newMin < currentMin) {
                currentMin = newMin;
                currentNext = i;
            }
        }
    }
    if (state->currentDestination != currentNext) {
        state->currentDestination = currentNext;
        int oldDirection = state->direction;
        if (state->currentDestination < state->currentFloor) {
            state->direction = -1;
        }
        else if (state->currentDestination > state->currentFloor) { // Changed from gdscript
            state->direction = 1;
            if (oldDirection != state->direction) {
                state->elevatorSpeed = startingSpeed;
            }
        }
        state->moving = true;
    }
}

static int floorsY[11] = { 0, 320, 640, 960, 1280, 1600, 1920, 2240, 2560, 2880, 3200 };
void updateAndRender(void* bitMapMemory, int screenWidth, int screenHeight, GameInput input, GameState *state, float delta) {
    if (!state->isInitialized) {
        state->isInitialized = true;
        memset(state->floorStates, 0, sizeof(state->floorStates));
        state->elevatorPosY = floorsY[10];
        state->currentFloor = 10;
        state->currentDestination = 10;
        state->elevatorSpeed = startingSpeed;
        state->direction = 0;
        state->moving = false;

        state->images.ui = loadBMP("../spr/ui.bmp", state->readFileFunction);
        state->images.button = loadBMP("../spr/button.bmp", state->readFileFunction,2);
        state->images.uiBottom = loadBMP("../spr/ui-bottom.bmp", state->readFileFunction);
        // TODO: I should close these files maybe, load them into my own structures and then close and free the previous memory, also invert rows.
    }
    fillBGWithColor(bitMapMemory, screenWidth, screenHeight, 0x0);
    Vector2i elevatorDim = { 78,90 };
    drawRectangle(bitMapMemory, screenWidth, screenHeight, (screenWidth-elevatorDim.width)/2 , 
        (screenHeight - elevatorDim.height) / 2, (screenWidth + elevatorDim.width) / 2, (screenHeight + elevatorDim.height) / 2, 1.0, 0.0, 0.0);

    drawImage((uint32_t*)bitMapMemory, &state->images.ui, 0, 16, screenWidth, screenHeight);
    drawImage((uint32_t*)bitMapMemory, &state->images.uiBottom, 0, 0, screenWidth, screenHeight);

    // Update floor states based on input
    for (int i = 0; i < 10; i++) {
        if (input.buttons[i]) {
            if (!state->moving && i == state->currentFloor) { // TODO: Play animation here? Check OG game
                continue;
            }
            state->floorStates[i] = !state->floorStates[i];
        }
    }

    setNextDirection(state);

    // Move and calculate getting to floors
    if (state->moving) {
        if (state->direction == 1) {
            state->elevatorPosY += (int)((float)state->elevatorSpeed * delta);
        } else if (state->direction == -1) {
            state->elevatorPosY -= (int)((float)state->elevatorSpeed * delta);
        }
    }
    if (state->direction == -1) {
        if (state->elevatorPosY < floorsY[state->currentFloor + state->direction]) {
            state->currentFloor += state->direction;
            if (state->currentFloor == state->currentDestination) {
                state->moving = false;
                state->direction = 0; // Not strictly needed I think
                state->floorStates[state->currentDestination] = false;
            }
        }
    }
    else if (state->direction == 1) {
        if (state->elevatorPosY > floorsY[state->currentFloor + state->direction]) {
            state->currentFloor += state->direction;
            if (state->currentFloor == state->currentDestination) {
                state->moving = false;
                state->direction = 0; // Not strictly needed I think
                state->floorStates[state->currentDestination] = false;

            }
        }
    }
    for (int j = 0; j < 10; j++) {
        drawImage((uint32_t*)bitMapMemory, &state->images.button, state->images.button.height*9,
            state->images.button.height + state->images.button.height*j, screenWidth, screenHeight, state->floorStates[j]);
    }

#ifdef SHOWBUTTONSTATES
    char floorsString[11];
    floorsString[0] = 'f';
    for (int i = 0; i < 10; i++) {
        if (state->floorStates[i]) {
            floorsString[i] = '1';
        }
        else {
            floorsString[i] = '0';
        }
    }
    floorsString[10] = '\0';
    OutputDebugString(floorsString);
    OutputDebugStringW(L"\n");
#endif

#ifdef SHOWELEVATORSTATS
    char buffer[100];
    sprintf_s(buffer, "y: %d | curFl: %d | curDes: %d | spd: %f | dir:%d | mov:%d\n", 
        state->elevatorPosY, state->currentFloor, state->currentDestination, state->elevatorSpeed, state->direction, state->moving);

    OutputDebugString(buffer);
    //OutputDebugString(L"\n");
#endif
}

