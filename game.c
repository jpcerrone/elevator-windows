#include <cstdint>
#include <stdio.h>
#include <time.h>

#include "game.h"
#include "vector2i.c"
#include "graphics.c"
#include "platform.h"
#include "intrinsics.h"
#include "assertions.h"
#include "bmp.c"

void setNextDirection(GameState *state) {
    // Get next destination
    int currentNext = state->currentFloor;
    int currentMin = INT32_MAX;
    for (int i = 0; i < 11; i++) {
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
        }
        if (oldDirection != state->direction) {
            state->elevatorSpeed = STARTING_SPEED;
        }
        state->moving = true;
    }
}

void addScoreToTotal(float mood, GameState *state, bool pickingUp) {
    int score = 0;
    switch (ceil(mood / MOOD_TIME)) {
    case 3: {
        score = pickingUp ? 1000 : 200;
    }break;
    case 2: {
        score = pickingUp ? 500 : 100;
    }break;
    case 1: {
        score = pickingUp ? 200: 50;
    }break;
    }
    state->score += score;
}

void pickAndPlaceGuys(GameState* state) {
    for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) {
        if (state->guys[i].active) {
            if (state->guys[i].onElevator && (state->guys[i].desiredFloor == state->currentFloor)) {
                addScoreToTotal(state->guys[i].mood, state, false);
                state->guys[i] = {};
                state->elevatorSpots[state->guys[i].elevatorSpot] = false;
                state->dropOffFloor = state->currentFloor;
                state->dropOffTimer = DROP_OFF_TIME;
            }
            else {
                if (state->guys[i].currentFloor == state->currentFloor) {
                    addScoreToTotal(state->guys[i].mood, state, true);
                    state->guys[i].onElevator = true;
                    state->guys[i].mood = MOOD_TIME * 3; // 3 to get all 4 possible mood state's ranges [0..3]
                    state->fullFloors[state->currentFloor] = false;
                    state->guys[i].currentFloor = -1;


                    if (state->score >= REQUIRED_SCORE * (state->currentLevel + 1) + (500 * state->currentLevel)) {
                        state->currentLevel += 1;
			state->flashTextTimer = FLASH_TIME;
                    }

                    for (int s = 0; s < ELEVATOR_SPOTS; s++) {
                        if (!state->elevatorSpots[s]) {
                            state->guys[i].elevatorSpot = s;
                            state->elevatorSpots[s] = true;
                            break;
                        }
                    }
                }
            }

        }
    }
}

bool areAllFloorsSave1Full(bool *fullFloors) {
    bool oneFree = false;
    for (int i = 0; i < 10; i++) {
        if (!fullFloors[i]) {
            if (!oneFree) {
                oneFree = true;
            }
            else {
                return false;
            }
        }
    }
    return true;
}

bool areMaxGuysOnScreen(Guy *guys) {
    for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) {
        if (!guys[i].active) {
            return false;
        }
    }
    return true;
}

void spawnNewGuy(Guy *guys, bool *fullFloors, int currentFloor) {
    if (areAllFloorsSave1Full(fullFloors) || areMaxGuysOnScreen(guys)) {
        return;
    }

    int randomGuyIdx = rand() % MAX_GUYS_ON_SCREEN;
    int randomCurrent = rand() % 10;
    int randomDest = rand() % 10;

    while (guys[randomGuyIdx].active) { 
        randomGuyIdx = rand() % 10;
    }
    while (fullFloors[randomCurrent] || (randomCurrent == currentFloor)) { // TODO add case when all floors are full
        randomCurrent = rand() % 10;
    }    
    while (randomDest == randomCurrent) {
        randomDest = rand() % 10;
    }

    guys[randomGuyIdx].active = true;
    guys[randomGuyIdx].mood = MOOD_TIME * 3; // 3 to get all 3 possible mood state's ranges + dead [0..3 + 4]
    guys[randomGuyIdx].currentFloor = randomCurrent;
    guys[randomGuyIdx].desiredFloor = randomDest;
    fullFloors[randomCurrent] = true;
}

void initGameState(GameState *state) {

    srand((uint32_t)time(NULL)); // Set random seed

    state->isInitialized = true;
    state->currentScreen = MENU;

    state->transitionInTimer = 0;
    state->transitionOutTimer = 0;
    state->scoreTimer = 0;

    // Load max score
    FileReadResult scoreResult = state->readFileFunction((char *)SCORE_PATH);
    if (scoreResult.memory && (scoreResult.size == sizeof(uint32_t))) {
        state->maxScore = *(uint32_t*)(scoreResult.memory);

    }
    else {
        state->maxScore = 0;
    }
    state->flashTextTimer = FLASH_TIME;

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
    state->images.door = loadBMP("../spr/door.bmp", state->readFileFunction, 2);
    state->images.numbersFont3px = loadBMP("../spr/m3x6Numbers.bmp", state->readFileFunction, 10);
    state->images.numbersFont4px = loadBMP("../spr/4x6Numbers.bmp", state->readFileFunction, 10);
    state->images.uiLabels = loadBMP("../spr/uiLabels.bmp", state->readFileFunction, 4);
    state->images.titleLabels = loadBMP("../spr/titleLabels.bmp", state->readFileFunction, 2);
    // TODO: I should close these files maybe, load them into my own structures and then close and free the previous memory, also invert rows.

}

void resetGame(GameState *state) {
    state->score = 0;
    memset(state->floorStates, 0, sizeof(state->floorStates));
    state->elevatorPosY = floorsY[10];
    state->currentFloor = 10;
    state->currentDestination = 10;
    state->elevatorSpeed = STARTING_SPEED;
    state->direction = 0;
    state->moving = false;
    state->dropOffFloor = -1;
    state->currentLevel = 0;

    state->spawnTimer = 1.5f; // First guy should appear fast
    state->doorTimer = 0;
    state->dropOffTimer = 0;
    state->flashTextTimer = 0;
    state->circleFocusTimer = 0.0;

    memset(state->elevatorSpots, 0, sizeof(state->elevatorSpots));
    memset(state->fullFloors, 0, sizeof(bool) * 10);
    for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) {
        state->guys[i] = {};
    }
}

void updateAndRender(void* bitMapMemory, int screenWidth, int screenHeight, GameInput input, GameState* state, float delta) {
    if (!state->isInitialized) {
        initGameState(state);
    }
    switch (state->currentScreen) {
        case MENU:{
<<<<<<< HEAD
            fillBGWithColor(bitMapMemory, screenWidth, screenHeight, BLACK);
            // Flipping not working
			drawImage((uint32_t*)bitMapMemory, &state->images.titleLabels, screenWidth/2.0f, screenHeight/2.0f, screenWidth, screenHeight, 0, true ,3, true);
=======

	    fillBGWithColor(bitMapMemory, screenWidth, screenHeight, BLACK);
            drawImage((uint32_t*)bitMapMemory, &state->images.titleLabels, screenWidth/2.0f, screenHeight/2.0f, screenWidth, screenHeight, 0, false ,3, true);
>>>>>>> circle
	    int flashPerSecond = 2;
	    if (state->flashTextTimer > 0){
		state->flashTextTimer -= flashPerSecond*delta;
	    } else if(state->flashTextTimer <= 0){
		    state->flashTextTimer = FLASH_TIME;
	    }
	    bool drawFlash = roundFloat(state->flashTextTimer*flashPerSecond) % 2;
	    if (drawFlash){
			drawImage((uint32_t*)bitMapMemory,&state->images.titleLabels, screenWidth/2.0f,screenHeight/2.0f - 20, screenWidth, screenHeight, 1, false, 1, true);
	    }
            for (int i = 0; i < 10; i++) {
                if (input.buttons[i]) {
                    resetGame(state);
                    state->currentScreen = GAME;
                    state->transitionOutTimer = TRANSITION_TIME;
                    break;
                }
            }
        }break;        
        case GAME:{

            // Timers
	    // Circle Focus
	    	int radius = 13;
        	if (state->circleFocusTimer > 2.5) {
                	state->circleFocusTimer -= delta;
			return;
	    } else if (state->circleFocusTimer > 2.2) {
	    	state->circleFocusTimer -= delta;
	    	float focusPercentage = (state->circleFocusTimer- 2.2f)*1.0f/0.3f; 
	    	drawFocusCircle((uint32_t*)bitMapMemory, state->circleSpot.x, state->circleSpot.y, (int)(focusPercentage*screenHeight/2 + (1 - focusPercentage) * radius), screenWidth, screenHeight);
	    	return;

		}
	    	else if (state->circleFocusTimer > 1.4) {
		    	state->circleFocusTimer -= delta;
			return;
	    	}
		else if (state->circleFocusTimer > 1.0) {
			state->circleFocusTimer -= delta;
			float focusPercentage = (state->circleFocusTimer-1.0f)/0.4f; 
			drawFocusCircle((uint32_t*)bitMapMemory, state->circleSpot.x, state->circleSpot.y, (int)(focusPercentage*radius), screenWidth, screenHeight);
			return;
		}
		else if (state->circleFocusTimer > 0){
			state->circleFocusTimer -= delta;
			return;
		}
		else if (state->circleFocusTimer < 0) {// TODO this could go to 0
		state->transitionInTimer = TRANSITION_TIME;
	             state->transitionOutTimer = TRANSITION_TIME;	                     state->scoreTimer = SCORE_TIME;
              state->currentScreen = SCORE;
		state->circleFocusTimer = 0;
		return;
            }
	    
            // Doors
            if (state->doorTimer > 0) {
                state->doorTimer -= delta;
            }
            else if (state->doorTimer < 0) {
                pickAndPlaceGuys(state);
                state->doorTimer = 0;
            }
            // Mood
            if (!(state->doorTimer > 0)) { // Don't update patience when opening doors
                for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) {
                    if (state->guys[i].active) {
                        state->guys[i].mood -= delta;
                        if (state->guys[i].mood <= 0.0) {
				state->circleFocusTimer = CIRCLE_TIME;
				if (state->guys[i].onElevator){
					state->circleSpot = sum(sum(Vector2i{screenWidth/2, screenHeight/2}, elevatorSpotsPos[state->guys[i].elevatorSpot]), Vector2i{11,32});
				} else{
				state->circleSpot ={ screenWidth-27, (state->guys[i].currentFloor+1)*16 +7};
				}
                    return;
                        }
                    }
                }
            }
#ifndef DONTSPAWN
            // Spawn
            state->spawnTimer -= delta;
            if (state->spawnTimer <= 0) {
                state->spawnTimer = SPAWN_TIMES[state->currentLevel];
                spawnNewGuy(state->guys, state->fullFloors, state->currentFloor);
            }
#endif
            // Drop Off
            if (state->dropOffTimer > 0) {
                state->dropOffTimer -= delta;
            }
            if (state->dropOffTimer < 0) {
                state->dropOffTimer = 0;
            }

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
            if (!(state->doorTimer > 0)) {
                if (state->moving) {
                    state->elevatorSpeed *= 1 + delta / 2;
                    if (state->direction == 1) {
                        state->elevatorPosY += (int)((float)state->elevatorSpeed * delta);
                    }
                    else if (state->direction == -1) {
                        state->elevatorPosY -= (int)((float)state->elevatorSpeed * delta);
                    }
                    if (state->direction == -1) {
                        if (state->elevatorPosY < floorsY[state->currentFloor - 1]) {
                            state->currentFloor += state->direction;
                            setNextDirection(state);
                            if (state->currentFloor == state->currentDestination) {
                                state->elevatorPosY = floorsY[state->currentFloor]; // Correct elevator position
                                state->moving = false;
                                state->direction = 0; // Not strictly needed I think
                                state->floorStates[state->currentDestination] = false;
                                state->doorTimer = DOOR_TIME;
                            }
                        }
                    }
                    else if (state->direction == 1) {
                        if (state->elevatorPosY > floorsY[state->currentFloor + 1]) {
                            state->currentFloor += state->direction;
                            setNextDirection(state);
                            if (state->currentFloor == state->currentDestination) {
                                state->elevatorPosY = floorsY[state->currentFloor]; // Correct elevator position
                                state->moving = false;
                                state->direction = 0; // Not strictly needed I think
                                state->floorStates[state->currentDestination] = false;
                                state->doorTimer = DOOR_TIME;
                            }
                        }
                    }
                }
                else {
                    state->elevatorSpeed = STARTING_SPEED; // TODO REMOVE?
                    setNextDirection(state);
                }
            }

            // Display background stuff
            fillBGWithColor(bitMapMemory, screenWidth, screenHeight, BLACK);
            int floorYOffset = state->elevatorPosY % (FLOOR_SEPARATION); // TODO see if we can express theese 16 in some other way, redner only on drawable part.
            if (floorYOffset > 160) {
                floorYOffset = (FLOOR_SEPARATION - floorYOffset) * -1; // Hack to handle negative mod operation.
            }
            drawImage((uint32_t*)bitMapMemory, &state->images.floorB, 0, (float)16 - floorYOffset, screenWidth, screenHeight);
            drawImage((uint32_t*)bitMapMemory, &state->images.vigasB, 0, 16, screenWidth, screenHeight);
            drawImage((uint32_t*)bitMapMemory, &state->images.elevator, (float)(screenWidth - state->images.elevator.width) / 2,
                (float)(screenHeight - 16 - state->images.elevator.height) / 2 + 16, screenWidth, screenHeight);

            // Display buttons
            for (int j = 0; j < 10; j++) {
		float buttonsPosX = state->images.button.width/(float)state->images.button.hframes * 9.0f;
                drawImage((uint32_t*)bitMapMemory, &state->images.button, buttonsPosX, 
                    (float)state->images.button.height + state->images.button.height * j, screenWidth, screenHeight, state->floorStates[j]);
		drawDigit((uint32_t*)bitMapMemory, &state->images.numbersFont4px, buttonsPosX + 6,
				(float)state->images.button.height + state->images.button.height * j + 5, screenWidth, screenHeight, j, 1, state->floorStates[j] ? BLACK : GREY);
            }
            Vector2i screenCenter = { screenWidth / 2, screenHeight / 2 };
            Vector2i floorIndicatorOffset = { 10, 40 }; // TODO: find proper offset from og game

            // Display guys, TODO: could be done only on updates
            drawImage((uint32_t*)bitMapMemory, &state->images.ui, 0, 16, screenWidth, screenHeight);
            for (int j = 0; j < MAX_GUYS_ON_SCREEN; j++) {
                if (state->guys[j].active) {
                    int mood = 3 - ceil(state->guys[j].mood / MOOD_TIME);
                    if (state->guys[j].onElevator) {
                        Vector2i posInElevator = elevatorSpotsPos[state->guys[j].elevatorSpot];
                        drawImage((uint32_t*)bitMapMemory, &state->images.guy, (float)screenCenter.x + posInElevator.x,
                            (float)screenCenter.y + posInElevator.y, screenWidth, screenHeight, mood);
                        drawDigit((uint32_t*)bitMapMemory, &state->images.numbersFont3px, (float)screenCenter.x + posInElevator.x + floorIndicatorOffset.x,
                            (float)screenCenter.y + posInElevator.y + floorIndicatorOffset.y, screenWidth, screenHeight, state->guys[j].desiredFloor, 2);
                    }
                    else {
                        Vector2i offsetInBox = { -2, -1 };
                        drawImage((uint32_t*)bitMapMemory, &state->images.uiGuy, state->images.uiGuy.height * 8.0f + offsetInBox.x,
                            (float)state->images.uiGuy.height + state->images.uiGuy.height * state->guys[j].currentFloor + offsetInBox.y,
                            screenWidth, screenHeight, mood);
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
            int doorFrame = (state->doorTimer > 0) ? 0 : 1;
            drawImage((uint32_t*)bitMapMemory, &state->images.door, (float)(screenWidth - state->images.elevatorF.width) / 2,
                (float)(screenHeight - 16 - state->images.elevatorF.height) / 2 + 16, screenWidth, screenHeight, doorFrame);
            drawImage((uint32_t*)bitMapMemory, &state->images.floor, 0, (float)16 - floorYOffset, screenWidth, screenHeight);

            for (int i = 0; i < MAX_GUYS_ON_SCREEN; i++) { // TODO see if we can add this in to the other loop, once we have Z layering
                if (state->guys[i].active) {
                    int mood = 3 - ceil(state->guys[i].mood / MOOD_TIME);
                    if ((state->guys[i].currentFloor * FLOOR_SEPARATION >= state->elevatorPosY - FLOOR_SEPARATION / 2) &&
                        (state->guys[i].currentFloor * FLOOR_SEPARATION <= state->elevatorPosY + FLOOR_SEPARATION / 2)) {
                        Vector2i waitingGuyPos = { 10, 16 - floorYOffset + 40 };
                        drawImage((uint32_t*)bitMapMemory, &state->images.guy, (float)waitingGuyPos.x, (float)waitingGuyPos.y, screenWidth, screenHeight, mood);
                        drawDigit((uint32_t*)bitMapMemory, &state->images.numbersFont3px, (float)waitingGuyPos.x + floorIndicatorOffset.x,
                            (float)waitingGuyPos.y + floorIndicatorOffset.y, screenWidth, screenHeight, state->guys[i].desiredFloor, 2);
                    }
                }

            }
            if (state->dropOffTimer > 0) {
                if ((state->dropOffFloor * FLOOR_SEPARATION >= state->elevatorPosY - FLOOR_SEPARATION / 2) &&
                    (state->dropOffFloor * FLOOR_SEPARATION <= state->elevatorPosY + FLOOR_SEPARATION / 2)) {
                    drawImage((uint32_t*)bitMapMemory, &state->images.guy, 10, (float)16 - floorYOffset + 40, screenWidth, screenHeight, 0, 1);
                }
            }

            drawImage((uint32_t*)bitMapMemory, &state->images.vigasF, 0, 16, screenWidth, screenHeight);

            // Bottom UI
            drawImage((uint32_t*)bitMapMemory, &state->images.uiBottom, 0, 0, screenWidth, screenHeight);
            // -- Score
            drawNumber(state->score, (uint32_t*)bitMapMemory, &state->images.numbersFont3px, 5, 5, screenWidth, screenHeight, GREY);
	    // -- Level
	    int xOffset = 0;
	    if (state->currentFloor == 10){
		    xOffset = -2;
	    }
	    drawNumber(state->currentFloor, (uint32_t*)bitMapMemory, &state->images.numbersFont3px, screenWidth/2.0f + 1, 5.0f, screenWidth, screenHeight, BLACK, true);
            // -- Elevator numbers
            if (state->currentFloor == 10) {
                drawDigit((uint32_t*)bitMapMemory, &state->images.numbersFont3px, (float)screenCenter.x - 37, (float)screenCenter.y + 38,
                    screenWidth, screenHeight, 1);
                drawDigit((uint32_t*)bitMapMemory, &state->images.numbersFont3px, (float)screenCenter.x - 34, (float)screenCenter.y + 35,
                    screenWidth, screenHeight, 0);
            }
            else {
                drawDigit((uint32_t*)bitMapMemory, &state->images.numbersFont3px, (float)screenCenter.x - 36, (float)screenCenter.y + 37,
                    screenWidth, screenHeight, state->currentFloor);
            }
            // --Level
	    int flashesPerSec = 2; 
	    if (state->flashTextTimer > 0){
		state->flashTextTimer -= delta;
	    } else if (state->flashTextTimer < 0){
		state->flashTextTimer = 0;
	    }
	    if (int(state->flashTextTimer * flashesPerSec) % 2 || state->flashTextTimer == 0){
            	drawDigit((uint32_t*)bitMapMemory, &state->images.numbersFont3px, (float)screenCenter.x + 70, 5,
                screenWidth, screenHeight, state->currentLevel, 1, GREY);
	    }
            // Transition In         
            if (state->transitionOutTimer > 0) {
                drawRectangle((uint32_t*)bitMapMemory, screenWidth, screenHeight, 0, 0, screenWidth, (int)(screenHeight * state->transitionOutTimer), 0, 0, 0);
                state->transitionOutTimer -= delta;
            }

	    // Debug stuff
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
        }break;        
        case SCORE:{
            if (state->score > state->maxScore) {
                state->maxScore = state->score;
            }
            if (state->transitionInTimer > 0) {
                state->transitionInTimer -= delta;
                drawRectangle((uint32_t*)bitMapMemory, screenWidth, screenHeight, 0, (int)(screenHeight* state->transitionInTimer), screenWidth, screenHeight, 0, 0, 0);
            }
            else {
                if (state->scoreTimer > 0) {
                    state->scoreTimer -= delta;
                    drawNumber(state->score, (uint32_t*)bitMapMemory, &state->images.numbersFont3px, screenWidth / 2.0f, screenHeight / 2.0f, screenWidth, screenHeight, GREY, true);
                    drawNumber(state->maxScore, (uint32_t*)bitMapMemory, &state->images.numbersFont3px, screenWidth / 2.0f, screenHeight / 2.0f - 20, screenWidth, screenHeight, GREY, true);
                    state->writeScoreFunction((char *)& SCORE_PATH, state->maxScore);
                }
                else if (state->transitionOutTimer > 0) {
                    drawRectangle((uint32_t*)bitMapMemory, screenWidth, screenHeight, 0, 0, screenWidth, (int)(screenHeight* (1-state->transitionOutTimer)), 0, 0, 0);
                    state->transitionOutTimer -= delta;
                }
                else {
                    state->currentScreen = MENU;
                }
            }
        }break;
    }

}

