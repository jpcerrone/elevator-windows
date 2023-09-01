#pragma once

struct AudioFile{
	uint16_t* audioSamples;
	uint32_t length;
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
	loadedFile.length = *((uint32_t*)((uint8_t*)result.memory + lengthOffset)); //== NumSamples * NumChannels * BitsPerSample/8
	loadedFile.audioSamples = (uint16_t*)((uint8_t*)result.memory + samplesOffset);
	uint16_t sample;
	for(uint32_t i=0; i < loadedFile.length / 2; i++){ ///2 cuz of 16bit samples instead if 8bits
		sample = *(loadedFile.audioSamples + i);
	}
	return loadedFile;


}
