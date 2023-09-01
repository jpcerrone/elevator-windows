#pragma once
struct Image {
    uint32_t* pixelPointer;
    uint32_t width;
    uint32_t height;
    // TODO: offset or a centered flag maybe
    int hframes;
};
