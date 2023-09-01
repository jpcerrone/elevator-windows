#pragma once

// NOTE: might need to pragma pack?
struct WavFile{
	FOURCC riffLabel; //	4	'RIFF'
	DWORD fileSize;	// 4	Total file size, not including the first 8 bytes
	FOURCC waveLabel;//	4	'WAVE'
	FOURCC fmtLabel;//	4	'fmt '
	DWORD waveFormatExSize;//	4	Size of the WAVEFORMATEX data that follows.
	WAVEFORMATEX waveFormatEx;	// Varies	Audio format header.
	FOURCC dataLabel;	// 4	'data'
	DWORD audioDataSize; //	4	Size of the audio data.
	BYTE[] audioData; //:w/	Varies	Audio data.
}
