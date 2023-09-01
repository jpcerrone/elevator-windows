#include "bmp.h"
Image loadBMP(char* path, readFile_t* readFunction, int hframes = 1) {
    FileReadResult result = readFunction(path);
    Image retImage = {};
    if (result.memory == nullptr) {
    	char failedResource[256];
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
