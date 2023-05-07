#ifndef FILE_PLAYER_H
#define FILE_PLAYER_H

#include <string>
#include <fstream>

#include "oboe/Oboe.h"

#include "AudioDecoder.h"
#include "defs.h"

using namespace std;
using namespace oboe;


class FilePlayer {
public:
    
  FilePlayer() {
    mSampleData = nullptr;
    isPlaying = false;
    dataChannels = 0;
    samplesProcessed = 0;
  }
  
  ~FilePlayer() {
    delete decoder;
  }
  
  void init();

  oboe::Result open();
  oboe::Result start();
  oboe::Result stop();
  oboe::Result close();
  
  bool play(string audioPath);
  bool loadFile(string audioPath);
  
  bool loadFileQueueStatic(string audioPath);
  bool loadFileStatic(string audioPath);
  bool loadFileWav(string audioPath);

private:

void resampleData(int destSampleRate);
void writeAudio(float* stream, int32_t numFrames);

void writeAudioStatic(float* stream, int32_t numFrames);
void writeAudioWav(float* stream, int32_t numFrames);

static constexpr int kChannelCount = 2;

AudioDecoder* decoder;
SharedQueue dataQ;
unique_ptr<float[]> mSampleData;

bool isPlaying;
int dataChannels;

int samplesProcessed;
int totalSamples;
int nextSampleId;

int32_t mNumChannels;
int32_t mSampleRate;
int32_t mNumSamples;

ifstream inputFile;

shared_ptr<AudioStream> mStream;

class MyDataCallback;
class MyErrorCallback;

shared_ptr<MyDataCallback> mDataCallback;
shared_ptr<MyErrorCallback> mErrorCallback;


class MyDataCallback : public AudioStreamDataCallback {
public:
    MyDataCallback(FilePlayer *parent) : mParent(parent) {}
    DataCallbackResult onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) override;
private:
    FilePlayer *mParent;
};

class MyErrorCallback : public AudioStreamErrorCallback {
public:
    MyErrorCallback(FilePlayer *parent) : mParent(parent) {}
    virtual ~MyErrorCallback() {}
    void onErrorAfterClose(AudioStream *oboeStream, oboe::Result error) override;
private:
    FilePlayer *mParent;
};

};

#endif //FILE_PLAYER_H
