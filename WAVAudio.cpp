#include "WAVAudio.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>
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

	//Analysis Data
	zeroCrossingData = NULL;
	movingAverage = NULL;
}

WAVAudio::~WAVAudio()
{
	if (wavData)
		delete wavData;

	if (movingAverage)
		delete movingAverage;

	if (zeroCrossingData)
		delete zeroCrossingData;
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

//Get moving average value in audio
double WAVAudio::getMovingAverageData(unsigned int index) {
	return movingAverage[index];
}

//Get zero crossing value in audio
int WAVAudio::getZeroCrossingData(unsigned int index) {
	return zeroCrossingData[index];
}

//Low pass filter via moving average
bool WAVAudio::calcMovingAverage(unsigned int windowSize) {
	if (!movingAverage) {
		movingAverage = new double[getNumSamples()];
	}

	//Set up initial sum for index 0
	movingAverage[0] = 0;
	int halfWindowSize = (windowSize - 1) / 2;
	for (unsigned int i = 0; i <= halfWindowSize; i++) {
		movingAverage[0] += (wavData[i] / (double)windowSize);
	}

	//Calculate moving average for rest of audio
	for (long i = 1; i < getNumSamples(); i++) {
		double leftValue = 0;
		double rightValue = 0;
		if (i - (long)halfWindowSize - 1 >= 0) { 
			leftValue = (wavData[i - halfWindowSize - 1] / (double)windowSize);
		}
		if (i + (long)halfWindowSize < getNumSamples()) {
			rightValue = (wavData[i + halfWindowSize] / (double)windowSize);
		}

		movingAverage[i] = movingAverage[i - 1] - leftValue + rightValue;
	}

	//Get average
	//for (long i = 0; i < getNumSamples(); i++) {
	//	movingAverage[i] /= windowSize;
	//}

	return true;
}

//Calculate zero crossing in audio
bool WAVAudio::calcZeroCrossing(unsigned int windowSize) {
	//If moving average not applied, then apply to original audio data
	if (!movingAverage) {
		movingAverage = new double[getNumSamples()];
		for (long i = 0; i < getNumSamples(); i++) {
			movingAverage[i] = (double)wavData[i];
		}
	}

	if (!zeroCrossingData) {
		zeroCrossingData = new int[getNumSamples()];
	}

	//Calculate zero crossings for whole audio
	int *tempZeroCrossings = new int[getNumSamples() + 1];

	tempZeroCrossings[0] = (movingAverage[0] != 0);
	for (unsigned int i = 1; i < getNumSamples(); i++) {
		if ((movingAverage[i - 1] <= 0) && (movingAverage[i] > 0)) { tempZeroCrossings[i] = 1; }
		else if ((movingAverage[i - 1] >= 0) && (movingAverage[i] < 0)) { tempZeroCrossings[i] = 1; }
		else { tempZeroCrossings[i] = 0; }
	}
	tempZeroCrossings[getNumSamples()] = (movingAverage[getNumSamples() - 1] != 0);

	//Set up initial zero crossing value for index 0
	int halfWindowSize = (windowSize - 1) / 2;
	//int zeroCrossings = halfWindowSize;	//Left side of index 0
	int zeroCrossings = 0;
	for (int i = 0; i < halfWindowSize; i++) {
		zeroCrossings += tempZeroCrossings[i];
	}
	zeroCrossingData[0] = zeroCrossings;

	//Calculate zero crossing values for rest of audio
	for (long i = 1; i < getNumSamples(); i++) {
		int leftValue = 0;
		int rightValue = 0;
		if ((i - halfWindowSize) - 1 >= 0) {
			leftValue = tempZeroCrossings[i - halfWindowSize - 1];
		}

		if ((i + halfWindowSize) <= getNumSamples()) {
			rightValue = tempZeroCrossings[i + halfWindowSize];
		}

		zeroCrossingData[i] = zeroCrossingData[i - 1] - leftValue + rightValue;
	}

	delete tempZeroCrossings;
	return true;
}

//Generate vector data for audio sync
bool WAVAudio::analyzeAudio() {
	double average = 0;
	for (unsigned long i = 0; i < getNumSamples(); i++) {
		average += zeroCrossingData[i];
	}
	average /= getNumSamples();

	//Find variance and standard deviation of xSquared values
	double variance = 0;
	for (unsigned long i = 0; i < getNumSamples(); i++) {
		variance += pow(zeroCrossingData[i] - average, 2.0);
	}
	variance /= getNumSamples();
	double standardDeviation = sqrt(variance);

	//Generate sync frames
	double upperBound = average - standardDeviation;	//Values above this, speech
	double lowerBound = average - standardDeviation;	//Values below this, no speech
	cout << upperBound << endl;
	cout << lowerBound << endl;

	//Create audio sync vector
	/*
	int syncFrameCountLimit = 24000;	//Minimum number of frames between syncs
	int syncFrameCount = syncFrameCountLimit;
	for (long i = 0; i < getNumSamples(); i++) {
		if ((zeroCrossingData[i] > upperBound) && (syncFrameCount >= syncFrameCountLimit)) {
			m_syncSamples.push_back(i);
			syncFrameCount = 0;
			//cout << i << endl;
		}

		else {
			syncFrameCount++;
		}
	}*/

	int syncFrameCountLimit = 24000;	//Minimum number of frames between syncs
	int syncFrameCount = syncFrameCountLimit;
	int continuousFrameCount = 0;
	int idealFrameCount = 24000;
	for (long i = 0; i < getNumSamples(); i++) {
		if ((zeroCrossingData[i] < lowerBound)) {
			continuousFrameCount++;
			if (continuousFrameCount >= idealFrameCount) {
				m_syncSamples.push_back(i - (idealFrameCount - 1));
				continuousFrameCount = 0;
			}
		}

		else {
			continuousFrameCount = 0;
		}
	}
	
	//Window sizes for silence vs speech may need to be different
	//Create detected speech vector
	int speechFrameCountLimit = 24000;	//Minimum number of frames for speech
	int speechFrameCount = speechFrameCountLimit;
	for (long i = 0; i < getNumSamples(); i++) {
		if ((zeroCrossingData[i] > upperBound) && (speechFrameCount >= speechFrameCountLimit)) {
			m_speechSamples.push_back(i);
			speechFrameCount = 0;
		}

		else {
			speechFrameCount++;
		}
	}

	//Create video sync frames from audio sync samples
	for (long i = 0; i < m_syncSamples.size(); i++) {
		unsigned short frame = m_syncSamples[i] / 24000 * 15;
		m_syncFrames.push_back(frame);
	}
	
	/*
	cout << "Number of sync frames: " << m_syncSamples.size() << endl;
	cout << "Number of speech frames: " << m_speechSamples.size() << endl;
	for (long i = 0; i < m_syncSamples.size(); i++) {
		cout << m_syncFrames[i] << "\t" << m_syncSamples[i] << "\t" << m_syncSamples[i] / 24000.0 << "s" << endl;
	}*/

	
	//for (long i = 0; i < m_m_speechSamples.size(); i++) {
	//	cout << m_m_speechSamples[i] / 24000.0 << "s" << endl;
	//}

	return true;
}

//Creates output audio file using vector of video frames
bool WAVAudio::writeVectortoWAV(char* filename, vector<unsigned short> frames) {
	FILE* output = fopen(filename, "wb");

	//Conversions
	int videoFramesPerSecond = 15;
	int audioSamplesPerVideoFrame = round(sampleRate / videoFramesPerSecond);

	//Updated values
	unsigned int newSubchunk2Size = frames.size() * audioSamplesPerVideoFrame * numChannels * blockAlign;
	unsigned int newChunkSize = 36 + newSubchunk2Size;

	//RIFF
	fwrite(&chunkID, sizeof(unsigned char), 4, output);
	fwrite(&newChunkSize, sizeof(unsigned char), 4, output);
	fwrite(&format, sizeof(unsigned char), 4, output);

	//fmt
	fwrite(&subChunk1ID, sizeof(unsigned char), 4, output);
	fwrite(&subChunk1Size, sizeof(unsigned char), 4, output);
	fwrite(&audioFormat, sizeof(unsigned char), 2, output);
	fwrite(&numChannels, sizeof(unsigned char), 2, output);
	fwrite(&sampleRate, sizeof(unsigned char), 4, output);
	fwrite(&byteRate, sizeof(unsigned char), 4, output);
	fwrite(&blockAlign, sizeof(unsigned char), 2, output);
	fwrite(&bitsPerSample, sizeof(unsigned char), 2, output);

	//Data
	fwrite(&subChunk2ID, sizeof(unsigned char), 4, output);
	fwrite(&newSubchunk2Size, sizeof(unsigned char), 4, output);
	
	//Debug
	char str[64] = { 0 };
	unsigned long largestIndex = 0;

	//Parse frame vector to add audio samples
	for (unsigned int i = 0; i < frames.size(); i++) {
		unsigned long audioIndex = frames[i] * audioSamplesPerVideoFrame;
		for (unsigned int j = 0; j < audioSamplesPerVideoFrame; j++) {
			unsigned long currentAudioIndex = audioIndex + j;
			int currentSample = wavData[currentAudioIndex];
			fwrite(&currentSample, sizeof(unsigned char), 2, output);
			
			if (frames[i] == 3400) {
				sprintf(str, "%d\t%d\t%d\t%d\t%d\n", currentAudioIndex, largestIndex, audioIndex, j, frames[i]);
				OutputDebugString(_T(str));
			}

			if (currentAudioIndex < largestIndex) {
				sprintf(str, "%d\t%d\t%d\t%d\t%d\n", currentAudioIndex, largestIndex, audioIndex, j, frames[i]);
				OutputDebugString(_T(str));
				largestIndex = currentAudioIndex;
			}

			else {
				largestIndex = currentAudioIndex;
			}
		}
	}

	fclose(output);
	return true;
}
