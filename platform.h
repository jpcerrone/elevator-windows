#pragma once

struct FileReadResult {
    uint64_t size;
    void* memory;
};

typedef FileReadResult readFile_t(char* path);

typedef bool writeScoreToFile_t(char* path, uint32_t score);

void freeFileMemory(void* memory);