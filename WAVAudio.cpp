#include "WAVAudio.h"
#include <iostream>
using namespace std;

// Constructor and Desctructors
WAVAudio::WAVAudio() {
	//RIFF header data
	chunkID = 0;
	chunkSize = 0;
	format = 0;

	//WAVE header data
	subChunk1ID = 0;
	subChunk1Size = 0;
	audioFormat = 0;
	numChannels = 0;
	sampleRate = 0;
	byteRate = 0;
	blockAlign = 0;
	bitsPerSample = 0;
	subChunk2ID = 0;
	subChunk2Size = 0;

	//WAVE audio data
	wavData = NULL;
}

WAVAudio::~WAVAudio()
{
	if (wavData)
		delete wavData;
}

//Parse each WAV header parameter
unsigned int WAVAudio::parseWavHeaderField(FILE* inputWAV, unsigned int numBytes) {
	//Read into buffer
	unsigned char buffer[4];
	fread(buffer, sizeof(unsigned char), numBytes, inputWAV);
	
	//Buffer to unsigned int
	unsigned int output = 0;
	for (unsigned int i = 0; i < numBytes; i++) {
		output |= (buffer[i] << (8 * i));
	}

	return output;
}

//Parse each WAV data sample
int WAVAudio::parseWavDataSample(FILE* inputWAV, unsigned int bytesPerSample) {
	//Read into buffer
	unsigned char buffer[4];
	fread(buffer, sizeof(unsigned char), bytesPerSample, inputWAV);

	//Buffer to unsigned int
	unsigned int byteData = 0;
	byteData <<= (bytesPerSample * 8);
	for (unsigned int i = 0; i < bytesPerSample; i++) {
		byteData |= (buffer[i] << (8 * i));
	}

	int output = convertTwosComplement(byteData, bytesPerSample);

	return output;
}

//Convert unsigned byte data to signed two's complement integer data
//Used to convert WAV sample data
int WAVAudio::convertTwosComplement(unsigned int sample, unsigned int bytesPerSample) {
	unsigned int significantBitMask = (1 << (bytesPerSample * 8 - 1));
	if ((sample & significantBitMask) == significantBitMask) {
		unsigned int mask = 0xFFFFFFFF;
		for (unsigned int i = 0; i < bytesPerSample * 8; i++) {
			mask <<= 1;
		}

		sample ^= mask;
	}

	return sample;
}

//Read WAV audio file
bool WAVAudio::readWAV(FILE* inputWAV) {
	unsigned char buffer[4];
	//Get RIFF header data
	chunkID = parseWavHeaderField(inputWAV, 4);
	chunkSize = parseWavHeaderField(inputWAV, 4);
	format = parseWavHeaderField(inputWAV, 4);

	//Get WAVE format data
	subChunk1ID = parseWavHeaderField(inputWAV, 4);
	subChunk1Size = parseWavHeaderField(inputWAV, 4);
	audioFormat = parseWavHeaderField(inputWAV, 2);
	numChannels = parseWavHeaderField(inputWAV, 2);
	sampleRate = parseWavHeaderField(inputWAV, 4);
	byteRate = parseWavHeaderField(inputWAV, 4);
	blockAlign = parseWavHeaderField(inputWAV, 2);
	bitsPerSample = parseWavHeaderField(inputWAV, 2);
	subChunk2ID = parseWavHeaderField(inputWAV, 4);
	subChunk2Size = parseWavHeaderField(inputWAV, 4);

	//Get WAVE audio data
	wavData = new int[subChunk2Size / blockAlign];
	for (unsigned int i = 0; i < subChunk2Size / blockAlign; i++) {
		wavData[i] = parseWavDataSample(inputWAV, blockAlign);
	}

	return true;
}

//Get number of audio channels
unsigned int WAVAudio::getNumChannels() {
	return numChannels;
}

//Get audio sampling rate
unsigned int WAVAudio::getSampleRate() {
	return sampleRate;
}

//Get number of bits per sample
unsigned int WAVAudio::getBitsPerSample() {
	return bitsPerSample;
}

//Get audio bit rate
unsigned int WAVAudio::getBitrate() {
	return bitsPerSample * sampleRate;
}

//Get number of samples in audio
unsigned int WAVAudio::getNumSamples() {
	return subChunk2Size / numChannels / blockAlign;
}

//Get sample value in audio
int WAVAudio::getAudioSample(unsigned int index) {
	return wavData[index];
}