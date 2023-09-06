#pragma once

struct AudioFile{
	uint16_t* samples;
	uint32_t sampleCount;
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
	uint32_t lengthOffset = 40;
	uint32_t samplesOffset = 44;
	loadedFile.sampleCount = *((uint32_t*)((uint8_t*)result.memory + lengthOffset))/2; // /2 cuz the original one is in bytes and 2B = 1 16b sample 
	loadedFile.samples = (uint16_t*)((uint8_t*)result.memory + samplesOffset);
	uint16_t sample;
	for(uint16_t i=0; i < loadedFile.sampleCount; i++){ 
		sample = *(loadedFile.samples + i);
	}
	return loadedFile;


}

struct AudioClip{
	bool active;
	AudioFile* file;
	float progress;
	float volume;
};
