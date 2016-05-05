#include <stdio.h>
#include <vector>
using namespace std;

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

	//Analysis Data
	double* movingAverage;
	int* zeroCrossingData;
	std::vector<unsigned long> m_syncSamples;		//Audio samples to sync (low detected speech)
	std::vector<unsigned long> m_speechSamples;		//Audio samples with detected speech
	std::vector<unsigned short> m_syncFrames;		//Video frames to sync

	//Helper functions
	unsigned int parseWavHeaderField(FILE* inputWAV, unsigned int numBytes);
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
	double getMovingAverageData(unsigned int index);
	int getZeroCrossingData(unsigned int index);
	std::vector<unsigned short> getSyncFrames() const { return m_syncFrames; }

	// Input Output operations
	bool readWAV(FILE* inputWAV);
	bool writeVectortoWAV(char* filename, vector<unsigned short> frames);	//Uses vector of image frames
	
	// Calculations
	bool calcMovingAverage(unsigned int windowSize);
	bool calcZeroCrossing(unsigned int windowSize);
	bool analyzeAudio();
	
};
