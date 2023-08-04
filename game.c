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
    char failedResource[256];
    Image retImage = {};
    if (result.memory == nullptr) {
        int writtenPrefix = sprintf_s(failedResource, 256, "Failure loading resource ");
        writtenPrefix += sprintf_s(failedResource + writtenPrefix, 256, path);
        sprintf_s(failedResource + writtenPrefix, 256, "\n");
        OutputDebugString(failedResource);
        return retImage;
    }
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

void pickAndPlaceGuys(Guy* guys, int currentFloor, bool *elevatorSpots) {
    for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) {
        if (guys[i].active) {
            if (guys[i].onElevator && (guys[i].desiredFloor == currentFloor)) {
                guys[i] = {};
                elevatorSpots[guys[i].elevatorSpot] = false;
            }
            else {
                if (guys[i].currentFloor == currentFloor) {
                    guys[i].onElevator = true;
                    guys[i].currentFloor = -1;

                    for (int s = 0; s < ELEVATOR_SPOTS; s++) {
                        if (!elevatorSpots[s]) {
                            guys[i].elevatorSpot = s;
                            elevatorSpots[s] = true;
                            break;
                        }
                    }
                }
            }

        }
    }
}

static int floorsY[11] = { 0, 320, 640, 960, 1280, 1600, 1920, 2240, 2560, 2880, 3200 };
void updateAndRender(void* bitMapMemory, int screenWidth, int screenHeight, GameInput input, GameState* state, float delta) {
    if (!state->isInitialized) {
        state->isInitialized = true;
        memset(state->floorStates, 0, sizeof(state->floorStates));
        state->elevatorPosY = floorsY[10];
        state->currentFloor = 10;
        state->currentDestination = 10;
        state->elevatorSpeed = startingSpeed;
        state->direction = 0;
        state->moving = false;
        memset(state->elevatorSpots, 0, sizeof(state->elevatorSpots));

        for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) {
            state->guys[i] = {};
        }
        state->guys[0].active = true;
        state->guys[0].currentFloor = 9;
        state->guys[0].desiredFloor = 5;

        state->guys[3].active = true;
        state->guys[3].currentFloor = 7;
        state->guys[3].desiredFloor = 8;

        state->images.ui = loadBMP("../spr/ui.bmp", state->readFileFunction);
        state->images.button = loadBMP("../spr/button.bmp", state->readFileFunction, 2);
        state->images.uiBottom = loadBMP("../spr/ui-bottom.bmp", state->readFileFunction);
        state->images.uiGuy = loadBMP("../spr/ui-guy.bmp", state->readFileFunction, 4);
        state->images.elevator = loadBMP("../spr/elevator.bmp", state->readFileFunction);
        state->images.guy = loadBMP("../spr/guy.bmp", state->readFileFunction, 4);
        state->images.floorB = loadBMP("../spr/floor_b.bmp", state->readFileFunction);
        state->images.floor = loadBMP("../spr/floor.bmp", state->readFileFunction);
        state->images.vigasB = loadBMP("../spr/vigasB.bmp", state->readFileFunction);
        state->images.vigasF = loadBMP("../spr/vigasF.bmp", state->readFileFunction);
        state->images.elevatorF = loadBMP("../spr/elevator_f.bmp", state->readFileFunction);
        state->images.arrows = loadBMP("../spr/arrow.bmp", state->readFileFunction, 2);
        // TODO: I should close these files maybe, load them into my own structures and then close and free the previous memory, also invert rows.
    }
    fillBGWithColor(bitMapMemory, screenWidth, screenHeight, 0x0);
    /*drawRectangle(bitMapMemory, screenWidth, screenHeight, (screenWidth-elevatorDim.width)/2 ,
        (screenHeight - elevatorDim.height) / 2, (screenWidth + elevatorDim.width) / 2, (screenHeight + elevatorDim.height) / 2, 1.0, 0.0, 0.0);*/

    static int floorSeparationY = 320;
    //drawImage((uint32_t*)bitMapMemory, &state->images.floorB, 0, screenHeight - (state->elevatorPosY % screenHeight), screenWidth, screenHeight);

    int floorYOffset = state->elevatorPosY % (floorSeparationY); // TODO see if we can express theese 16 in some other way, redner only on drawable part.
    if (floorYOffset > 160) {
        floorYOffset = (floorSeparationY - floorYOffset) * -1; // Hack to handle negative mod operation.
    }
    drawImage((uint32_t*)bitMapMemory, &state->images.floorB, 0, (float)16 - floorYOffset, screenWidth, screenHeight);
    drawImage((uint32_t*)bitMapMemory, &state->images.vigasB, 0, 16, screenWidth, screenHeight);
    drawImage((uint32_t*)bitMapMemory, &state->images.elevator, (float)(screenWidth - state->images.elevator.width) / 2,
        (float)(screenHeight - 16 - state->images.elevator.height) / 2 + 16, screenWidth, screenHeight);

    // Update floor states based on input
    for (int i = 0; i < 10; i++) {
        if (input.buttons[i]) {
            if (!state->moving && i == state->currentFloor) { // TODO: Play animation here? Check OG game
                continue;
            }
            state->floorStates[i] = !state->floorStates[i];
        }
    }



    // Move and calculate getting to floors
    if (state->moving) {
        if (state->direction == 1) {
            state->elevatorPosY += (int)((float)state->elevatorSpeed * delta);
        }
        else if (state->direction == -1) {
            state->elevatorPosY -= (int)((float)state->elevatorSpeed * delta);
        }
        if (state->direction == -1) {
            if (state->elevatorPosY < floorsY[state->currentFloor + state->direction]) {
                setNextDirection(state);
                state->currentFloor += state->direction;
                if (state->currentFloor == state->currentDestination) {
                    state->elevatorPosY = floorsY[state->currentFloor]; // Correct elevator position
                    pickAndPlaceGuys(state->guys, state->currentFloor, state->elevatorSpots);
                    state->moving = false;
                    state->direction = 0; // Not strictly needed I think
                    state->floorStates[state->currentDestination] = false;
                }
            }
        }
        else if (state->direction == 1) {
            if (state->elevatorPosY > floorsY[state->currentFloor + state->direction]) {
                setNextDirection(state);
                state->currentFloor += state->direction;
                if (state->currentFloor == state->currentDestination) {
                    state->elevatorPosY = floorsY[state->currentFloor]; // Correct elevator position
                    pickAndPlaceGuys(state->guys, state->currentFloor, state->elevatorSpots);
                    state->moving = false;
                    state->direction = 0; // Not strictly needed I think
                    state->floorStates[state->currentDestination] = false;

                }
            }
        }
    }
    else {
        setNextDirection(state);
    }

    // Display buttons
    for (int j = 0; j < 10; j++) {
        drawImage((uint32_t*)bitMapMemory, &state->images.button, state->images.button.height * 9.0f,
            (float)state->images.button.height + state->images.button.height * j, screenWidth, screenHeight, state->floorStates[j]);
    }

    Vector2i screenCenter = { screenWidth / 2, screenHeight / 2 };

    // Display guys, TODO: could be done only on updates
    drawImage((uint32_t*)bitMapMemory, &state->images.ui, 0, 16, screenWidth, screenHeight);
    for (int j = 0; j < MAX_GUYS_ON_SCREEN; j++) {
        if (state->guys[j].active) {
            if (state->guys[j].onElevator) {
                Vector2i posInElevator = elevatorSpotsPos[state->guys[j].elevatorSpot];
                drawImage((uint32_t*)bitMapMemory, &state->images.guy, (float)screenCenter.x + posInElevator.x,
                    (float)screenCenter.y + posInElevator.y, screenWidth, screenHeight);
            }
            else {
                Vector2i offsetInBox = { -2, -1 };
                drawImage((uint32_t*)bitMapMemory, &state->images.uiGuy, state->images.uiGuy.height * 8.0f + offsetInBox.x,
                    (float)state->images.uiGuy.height + state->images.uiGuy.height * state->guys[j].currentFloor + offsetInBox.y, screenWidth, screenHeight);
                Vector2i arrowOffsetInBox = { 11, 0 };
                int arrowFrame;
                if (state->guys[j].currentFloor < state->guys[j].desiredFloor) {
                    arrowFrame = 0;
                }
                else {
                    arrowFrame = 1;
                }
                drawImage((uint32_t*)bitMapMemory, &state->images.arrows, state->images.uiGuy.height * 8.0f + arrowOffsetInBox.x,
                    (float)state->images.uiGuy.height + state->images.uiGuy.height * state->guys[j].currentFloor, screenWidth, screenHeight, arrowFrame);
            }
        }
    }

    // Draw rest of scene
    drawImage((uint32_t*)bitMapMemory, &state->images.elevatorF, (float)(screenWidth - state->images.elevatorF.width) / 2,
        (float)(screenHeight - 16 - state->images.elevatorF.height) / 2 + 16, screenWidth, screenHeight);
    drawImage((uint32_t*)bitMapMemory, &state->images.floor, 0, (float)16 - floorYOffset, screenWidth, screenHeight);
    drawImage((uint32_t*)bitMapMemory, &state->images.vigasF, 0, 16, screenWidth, screenHeight);
    drawImage((uint32_t*)bitMapMemory, &state->images.uiBottom, 0, 0, screenWidth, screenHeight);

#ifdef SHOWGUYSSTATS
    static const int MAX_GUYS_STRING_SIZE = 19 * MAX_GUYS_ON_SCREEN; // sizeof([g%d: c:%d d:%d e:%d]) * MAX_GUYS_ON_SCREEN
    char guysString[MAX_GUYS_STRING_SIZE];
    guysString[0] = '\0';
    for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) {
        if (state->guys[i].active) { //Using strlen is very inefficient here, but OK for debug code
            sprintf_s(guysString + strlen(guysString), MAX_GUYS_STRING_SIZE, "[g%d: c:%d d:%d e:%d], ", i, state->guys[i].currentFloor, state->guys[i].desiredFloor, state->guys[i].onElevator);
        }
     }
    guysString[strlen(guysString)] = '\0';
    OutputDebugString(guysString);
    OutputDebugStringW(L"\n");
#endif

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

