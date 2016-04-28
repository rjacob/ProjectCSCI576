#include <stdio.h>

class WAVAudio 
{

private:
	//RIFF header data
	unsigned int chunkID;
	unsigned int chunkSize;
	unsigned int format;

	//WAVE header data
	unsigned int subChunk1ID;
	unsigned int subChunk1Size;
	unsigned int audioFormat;
	unsigned int numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned int blockAlign;
	unsigned int bitsPerSample;
	unsigned int subChunk2ID;
	unsigned int subChunk2Size;

	//WAVE audio data
	int* wavData;

	//Helper functions
	bool parseWavHeader(FILE* inputWAV);
	unsigned int parseWavHeaderSection(FILE* inputWAV, unsigned int numBytes);
	int parseWavDataSample(FILE* inputWAV, unsigned int bytesPerSample);
	int convertTwosComplement(unsigned int sample, unsigned int bytesPerSample);

public:
	// Constructor
	WAVAudio();

	// Destructor
	~WAVAudio();

	// Reader & Writer functions
	unsigned int getNumChannels();
	unsigned int getSampleRate();
	unsigned int getBitsPerSample();
	unsigned int getBitrate();
	unsigned int getNumSamples();
	int getAudioSample(unsigned int index);

	// Input Output operations
	bool readWAV(FILE* inputWAV);
	
	// Calculations
	unsigned int zeroCrossing(unsigned int windowSize);
};
