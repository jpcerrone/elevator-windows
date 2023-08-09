#pragma once
#include "platform.h"
#include "graphics.h"
#include "math.h"
#include "vector2i.c"


static const int MAX_GUYS_ON_SCREEN = 20;
static const int ELEVATOR_SPOTS = 5;
static const float STARTING_SPEED = 150;
static int floorsY[11] = { 0, 320, 640, 960, 1280, 1600, 1920, 2240, 2560, 2880, 3200 };

static const Vector2i elevatorSpotsPos[ELEVATOR_SPOTS] = {
    {-2, -29}, // TODO: these were done by eye, y is fine
    {-21, -29},
    {-30, -37},
    {7, -37},
    { -9, -37},
};

static const float SPAWN_TIME = 8.0f;
static const float MOOD_TIME = 4.0f;
static const float DOOR_TIME = 0.5f;
struct Guy {
    bool active;

    bool onElevator;
    int elevatorSpot;

    int desiredFloor;
    int currentFloor;

    float mood; //From MOOD_TIME*3 to MOOD_TIME, 0 is game over
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

    float spawnTimer;
    float doorTimer;

    Guy guys[MAX_GUYS_ON_SCREEN];
    bool elevatorSpots[ELEVATOR_SPOTS];
    bool fullFloors[10];

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
        Image door;
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