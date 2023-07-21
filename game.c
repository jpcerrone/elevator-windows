#include <cstdint>
#include <stdio.h>

#include "vector2i.c"
#include "graphics.c"

struct GameInput {
    union {
        bool buttons[9];
        struct {
            bool button1;
            bool button2;
            bool button3;
            bool button4;
            bool button5;
            bool button6;
            bool button7;
            bool button8;
            bool button9;
        };
    };

};

static const float startingSpeed = 150;
struct GameState {
    bool isInitialized;
    bool floorStates[11]; // 0 is the index for floor 1, 9 is the index for floor 10
    int elevatorPosY;
    int currentFloor;
    int currentDestination;
    float elevatorSpeed;
    int direction;
    bool moving;
};


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
    /* gdscript
    func calculateNextDirection():
    # Calculate next floor
    # Calculate next stop and direction of movement
    var minDist:int = Globals.INT_MAX
    var currentNext:int = currentFloor
    for i in 10:
        if (enabledFloors[i]):
            var newMin = abs(currentFloor - i)
            if (newMin < minDist):
                minDist = newMin
                currentNext = i
    currentDestination = currentNext
    var oldDirection = direction
    if (currentDestination > currentFloor):
        direction = -1
    else:
        direction = 1
    if (oldDirection != direction):
        speed = startingSpeed*/ //gdscript
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
    }
    fillBGWithColor(bitMapMemory, screenWidth, screenHeight, 0xFFFFFFFF);
    Vector2i elevatorDim = { 78,90 };
    drawRectangle(bitMapMemory, screenWidth, screenHeight, (screenWidth-elevatorDim.width)/2 , 
        (screenHeight - elevatorDim.height) / 2, (screenWidth + elevatorDim.width) / 2, (screenHeight + elevatorDim.height) / 2, 1.0, 0.0, 0.0);

    // Update floor states based on input
    for (int i = 1; i < 11; i++) {
        if (input.buttons[i-1]) {
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


#ifdef SHOWBUTTONSTATES
    char floorsString[11];
    floorsString[0] = 'f';
    for (int i = 1; i < 10; i++) {
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

