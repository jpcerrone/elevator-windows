#pragma once
#include "platform.h"
#include "graphics.h"
#include "math.h"

struct GameState {
    bool isInitialized;
    bool floorStates[11]; // 0 is the index for floor 1, 9 is the index for floor 10
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
        // Image button..., etc
    };
    images_t images;
};

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