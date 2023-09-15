#include "graphics.h"
#include "assertions.h"
#include "math.h"

// Using int's since it's pixel art
void drawRectangle(void* outputStream, int bmWidth, int bmHeight, int minX, int minY, int maxX, int maxY, uint32_t color) {
    uint32_t* pixel = (uint32_t*)outputStream;

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

    //uint32_t color = (0xFF << 24) | (roundUFloat(r * 255.0f) << 16) | (roundUFloat(g * 255.0f) << 8) | (roundUFloat(b * 255.0f) << 0); //0xAA RR GG BB 0b1111

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


struct Render{
	Image* image;
	float x;
	float y;
	int frame;
	bool flip;
	int scale;
	bool centered;
	int zLayer;
	uint32_t recolor;
};

void renderImage(Render* render, uint32_t* bufferMemory, int screenWidth, int screenHeight){
    int frameWidth = (render->image->width / render->image->hframes); // TODO: check handling image widths that are odd. (ie 23)  
    if (render->centered) {
        render->x = render->x - (frameWidth*render->scale / 2.0f);
    }
    int sampleHeight = render->image->height;
    int sampleWidth = frameWidth ;

    int renderHeight = sampleHeight * render->scale;
    int renderWidth = sampleWidth * render->scale;

    if (renderHeight + render->y > screenHeight) {
        int diff = (int)(renderHeight + render->y - screenHeight);
        sampleHeight -= diff / render->scale; // TODO (float) diff
        renderHeight = (int)(screenHeight - (render->y));
    }
    if (renderWidth + render->x > screenWidth) {
        renderWidth = (int)(screenWidth - (render->x)); 
    }    
    // Go to upper left corner.
    bufferMemory += (int)clampPos((float)screenWidth * (screenHeight - (renderHeight + roundFloat(clampPos(render->y)))));
    bufferMemory += roundFloat(clampPos(render->x));

    uint32_t* pixelPointer = render->image->pixelPointer;
    pixelPointer += (sampleHeight - 1) * render->image->width; // Go to end row of bmp (it's inverted)
    
    if (render->x < 0) {
        renderWidth += roundFloat(render->x); // Reducing width
    }
    if (render->y < 0) {
        renderHeight += roundFloat(render->y); // Reducing height
        // No need to advance from where to sample here
        bufferMemory -= screenWidth * (roundFloat(render->y));
    }

    int strideToNextRow = screenWidth - renderWidth;
    if (strideToNextRow < 0) {
        strideToNextRow = 0;
    }
    for (int j = 0; j < renderHeight; j++) {
        int p = 0;
        pixelPointer += frameWidth * render->frame; // Advance to proper frame
        if (render->flip) {
            pixelPointer += frameWidth -1;
            p += frameWidth - 1;
        }
        if (render->x < 0) {
	    if (render->flip){
		pixelPointer += roundFloat(render->x/render->scale);
		p += roundFloat(render->x/render->scale);
	    } else{
            pixelPointer -= roundFloat(render->x/render->scale);
            p -= roundFloat(render->x/render->scale);
	    }
        }
        for (int i = 0; i < renderWidth; i++) {
            float alphaValue = (*pixelPointer >> 24) / 255.0f;
            uint32_t redValueS = (*pixelPointer & 0xFF0000) >> 16;
            uint32_t greenValueS = (*pixelPointer & 0x00FF00) >> 8;
            uint32_t blueValueS = (*pixelPointer & 0x0000FF);
		if(render->recolor != 0){
				redValueS = (render->recolor & 0xFF0000) >> 16;
				greenValueS = (render->recolor & 0x00FF00) >> 8;
            			blueValueS = (render->recolor & 0x0000FF);
	}
            uint32_t redValueD = (*bufferMemory & 0xFF0000) >> 16;
            uint32_t greenValueD = (*bufferMemory & 0x00FF00) >> 8;
            uint32_t blueValueD = *bufferMemory & 0x0000FF;

            uint32_t interpolatedPixel = (uint32_t)(alphaValue * redValueS + (1 - alphaValue) * redValueD) << 16
                | (uint32_t)(alphaValue * greenValueS + (1 - alphaValue) * greenValueD) << 8
                | (uint32_t)(alphaValue * blueValueS + (1 - alphaValue) * blueValueD);

            *bufferMemory = interpolatedPixel;
            bufferMemory++;
            if (render->scale == 1) {
                if (render->flip) {
                    pixelPointer--; // right to left
                    p--;
                }
                else {
                    pixelPointer++; // left to right
                    p++;
                }
            }
            else {
                if ((i % render->scale) == render->scale - 1) { // Advance pointer every "scale" steps
                    if (render->flip) {
                        pixelPointer--; 
                        p--;// right to left
                    }
                    else {
                        pixelPointer++; 
                        p++; // left to right
                    }
                }
            }
        }
        if (render->flip) {
            pixelPointer += frameWidth;
            p += frameWidth;
        }
		pixelPointer += (int)(frameWidth - p); // Advance till end of frame
        pixelPointer += render->image->width - frameWidth * (render->frame + 1); // Advance till end of current row in the image

        if (render->scale == 1) {
            pixelPointer -= 2 * render->image->width;
        }
        else {
            if ((j % render->scale) == render->scale-1) {
                pixelPointer -= 2 * render->image->width; // start at the top, go down (Since BMPs are inverted) TODO: just load them in the right order
            }
            else {
                pixelPointer -= render->image->width;
            }

        }
        bufferMemory += strideToNextRow;
    }


}

void renderImages(Render* renders, int arraySize, uint32_t* buffer, int screenWidth, int screenHeight){
	// Sort 
	bool hasSwaped = true;
	while(hasSwaped){
		hasSwaped = false;
		for(int i =0; i < arraySize - 1; i++){
            if (renders[i+1].image == NULL) {
                break;
            }
			if(renders[i].zLayer > renders[i+1].zLayer){
				Render temp = renders[i+1];
				renders[i+1] = renders[i];
				renders[i] = temp;
				hasSwaped=true;
			}
		}
	}

	// Render
	for(int i=0; i < arraySize; i++){
		if (renders[i].image) {
		    renderImage(&renders[i], buffer, screenWidth, screenHeight);
		}
	}
}
void drawImage(Render* renders, int rendersSize, Image* image, float x, float y, int zLayer = 0, int frame = 0, bool flip = 0, int scale = 1, bool centered = false) {
    // TODO: should x and y be ints?
    Assert(frame >= 0);
    Assert(scale >= 1);
    Assert(frame <= image->hframes);
    // NOTE: offset could be implemented here.
    //y += image->offset.y;
    //x += image->offset.x;

    if (!image->pixelPointer) {
        OutputDebugString("Can not display null image\n");
        return;
    }

    Render render = {};
    render.image = image;
    render.x = x;
    render.y = y;
    render.frame = frame;
    render.flip = flip;
    render.scale = scale;
    render.centered = centered;
    render.zLayer = zLayer;
	for(int i=0; i < rendersSize; i++){
		if(!renders[i].image){
			renders[i] = render;
			break;
		}
	}

}

void drawFocusCircle(void* bufferMemory, int x, int y, int radius, int screenWidth, int screenHeight){
	uint32_t* pixel = (uint32_t*) bufferMemory;
    pixel += screenWidth * (screenHeight-1); //Advance till the end //TODO can this be stored differently?
	for(int j=0; j < screenHeight; j++){
		for(int i=0; i < screenWidth; i++){
			Vector2i circleCenter = {x,y};
			Vector2i point = {i, j};
			if (distance(circleCenter, point) >= radius){
				*pixel = BLACK;	
			}
			pixel++;
		}
        pixel -= 2 * screenWidth;
	}
	return;
}
void getDigitsFromNumber(uint32_t number, int* digits, int maxDigits) {
    int currentDigit = 0;
    int currentNumber = number;
    for (int i = maxDigits - 1; i >= 0; i--) {
        int significantVal = pow(10, i);
        int numBySignificantVal = currentNumber / significantVal;
        if ((numBySignificantVal) >= 1) {
            digits[currentDigit] = (int)(numBySignificantVal);
            currentNumber = currentNumber - significantVal * numBySignificantVal;
        }
        currentDigit++;
    }
}

void drawNumber(uint32_t number, Render* renders, int rendersSize, Image* font, float x, float y, int zLayer = 0, int scale = 1.0, uint32_t color = BLACK, bool centered = false, float spacing = 1.0) {
    const int MAX_DIGITS_DISPLAY = 6;
    float digitSeparation =(float)( font->width / font->hframes + spacing);
    Assert(number < pow(10, MAX_DIGITS_DISPLAY));
    int digits[MAX_DIGITS_DISPLAY] = { };
    int digitsToDraw = MAX_DIGITS_DISPLAY;
    getDigitsFromNumber(number, digits, MAX_DIGITS_DISPLAY);
    for (int i = 0; i < MAX_DIGITS_DISPLAY; i++) {
        if (digits[i] == 0) {
            digitsToDraw--;
        }
        else {
            break;
        }
    }
    digitsToDraw = max(digitsToDraw, 1); // So that '0' can be drawn as a single digit.
    if (centered) {
        x = x - digitsToDraw * digitSeparation / 2.0f;
    }
    for (int i = 0; i < digitsToDraw; i++) {
	    Render render;
	    render.image = font;
	    render.x = x + i * digitSeparation;
	    render.y = y;
	    render.frame = digits[MAX_DIGITS_DISPLAY - digitsToDraw + i];
	    render.flip = false; //TODO add parameter to drawNumberCall
	    render.scale = scale;
	    render.centered = centered;
	    render.zLayer = zLayer; 
        render.recolor = color;	
	for(int j=0; j < rendersSize; j++){
		if(!renders[j].image){
			renders[j] = render;
			break;
		}
	}
    }
}
