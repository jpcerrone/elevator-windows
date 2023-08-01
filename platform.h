#pragma once

struct FileReadResult {
    uint64_t size;
    void* memory;
};

typedef FileReadResult readFile_t(char* path);

void freeFileMemory(void* memory);