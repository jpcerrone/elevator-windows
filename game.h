#pragma once
#include "platform.h"
#include "graphics.h"
#include "math.h"

struct GameState {
    bool isInitialized;
    bool floorStates[11]; // 0 is the index for floor 0, 10 is the index for floor 9, there's a starting floor 10.
    int elevatorPosY;
    int currentFloor;
    int currentDestination;
    float elevatorSpeed;
    int direction;
    bool moving;

    readFile_t* readFileFunction;
    //writeFile_t* writeFile;
    //freeFileMemory_t* freeFileMemory;

    struct images_t {
        Image ui;
        Image button;
        Image uiBottom;
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