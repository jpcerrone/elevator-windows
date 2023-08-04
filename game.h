#pragma once
#include "platform.h"
#include "graphics.h"
#include "math.h"
#include "vector2i.c"

static const int MAX_GUYS_ON_SCREEN = 20;

struct Guy {
    bool active;

    bool onElevator;
    int elevatorSpot;

    int desiredFloor;
    int currentFloor;
    // mood
};

static const int ELEVATOR_SPOTS = 5;
static const Vector2i elevatorSpotsPos[ELEVATOR_SPOTS] = {
    {-2, -37},
    {-21, -37},
    {-30, -45},
    {7, -45},
    { -9, -45},
};

struct GameState {
    bool isInitialized;
    bool floorStates[11]; // 0 is the index for floor 0, 10 is the index for floor 9, there's a starting floor 10.
    int elevatorPosY;
    int currentFloor;
    int currentDestination;
    float elevatorSpeed;
    int direction;
    bool moving;


    Guy guys[MAX_GUYS_ON_SCREEN];
    bool elevatorSpots[ELEVATOR_SPOTS];

    readFile_t* readFileFunction;
    //writeFile_t* writeFile;
    //freeFileMemory_t* freeFileMemory;

    struct images_t {
        Image ui;
        Image button;
        Image uiBottom;
        Image uiGuy;
        Image elevator;
        Image elevatorF;
        Image guy;
        Image floorB;
        Image floor;
        Image vigasB;
        Image vigasF;
        Image arrows;
    };
    images_t images;
};

struct GameInput {
    union {
        bool buttons[10];
        struct {
            bool button0;
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