#pragma once

struct AudioFile{
	int16_t* samples;
	uint32_t sampleCount;
	uint16_t channels;
};

AudioFile loadWavFile(char* path, readFile_t* readFunction){
	FileReadResult result = readFunction(path);
	AudioFile loadedFile = {};
    	if (result.memory == nullptr) {
    		char failedResource[256];
        	int writtenPrefix = sprintf_s(failedResource, 256, "Failure loading resource ");
        	writtenPrefix += sprintf_s(failedResource + writtenPrefix, 256, path);
        	sprintf_s(failedResource + writtenPrefix, 256, "\n");
        	OutputDebugString(failedResource);
        	return loadedFile;
    	}
	uint32_t channelsOffset = 22;
	uint32_t lengthOffset = 40;
	uint32_t samplesOffset = 44;
	loadedFile.sampleCount = *((uint32_t*)((uint8_t*)result.memory + lengthOffset))/2; // /2 cuz the original one is in bytes and 2B = 1 16b sample 
	loadedFile.samples = (int16_t*)((uint8_t*)result.memory + samplesOffset);
	loadedFile.channels = *((uint16_t*)((uint8_t*)result.memory + channelsOffset)); 
	return loadedFile;


}

struct AudioClip{
	bool active;
	AudioFile* file;
	float progress;
	float volume;
	bool loop;
};
