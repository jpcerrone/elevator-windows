#pragma once
#include "platform.h"
#include "graphics.h"
#include "math.h"
#include "vector2i.c"
#include "audio.c"

static const uint32_t GREY = 0xFF686868;
static const uint32_t BLACK = 0xFF000000;

static const int MAX_GUYS_ON_SCREEN = 20;
static const int ELEVATOR_SPOTS = 5;
static const float STARTING_SPEED = 150;
static const int FLOOR_SEPARATION = 320;
static int floorsY[11] = { 0, 320, 640, 960, 1280, 1600, 1920, 2240, 2560, 2880, 3200 };
const static int REQUIRED_SCORE = 3000;

static const char SCORE_PATH[MAX_PATH] = "maxScore";// MAX_PATH might be windows only

static const Vector2i elevatorSpotsPos[ELEVATOR_SPOTS] = {
    {-2, -29}, // TODO: these were done by eye, y is fine
    {-21, -29},
    {-30, -37},
    {7, -37},
    { -9, -37},
};

static const float SPAWN_TIMES[13] = { 8, 6.5f, 5.0f, 5.5f, 4.0f, 3.6f, 3.2f, 3, 2.9f, 2.8f, 2.7f, 2.6f, 2.5f };
static const float MOOD_TIME = 4.0f;
static const float DOOR_TIME = 0.5f;
static const float DROP_OFF_TIME = 1.0f;
static const float TRANSITION_TIME = 1.0f;
static const float SCORE_TIME = 3.0f;
static const float FLASH_TIME = 3.0f;
static const float CIRCLE_TIME = 3.2f;

struct Guy {
    bool active;

    bool onElevator;
    int elevatorSpot;

    int desiredFloor;
    int currentFloor;

    float mood; //From MOOD_TIME*3 to MOOD_TIME, 0 is game over
};

enum Screen {
    MENU,
    GAME,
    SCORE,
};

struct GameState {
    bool isInitialized;
    Screen currentScreen;
    int score;
    int maxScore;
    bool floorStates[11]; // 0 is the index for floor 0, 10 is the index for floor 9, there's a starting floor 10.
    int elevatorPosY;
    int currentFloor;
    int currentDestination;
    float elevatorSpeed;
    int direction;
    bool moving;
    int currentLevel;

    float spawnTimer;
    float doorTimer;
    float dropOffTimer;
    float transitionInTimer;
    float transitionOutTimer;
    float scoreTimer;
    float flashTextTimer;
    float circleFocusTimer;
    Vector2i circleSpot;

    int dropOffFloor;

    Guy guys[MAX_GUYS_ON_SCREEN];
    bool elevatorSpots[ELEVATOR_SPOTS];
    bool fullFloors[10];

    readFile_t* readFileFunction;
    writeScoreToFile_t* writeScoreFunction;
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
        Image numbersFont3px;
        Image numbersFont4px;
	Image uiLabels;
	Image titleLabels;
    };
    images_t images;

    struct audioFiles_t {
	    AudioFile click;
    };
    audioFiles_t audioFiles;

    float sampleOffset;
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
