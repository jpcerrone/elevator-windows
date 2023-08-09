#pragma once

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