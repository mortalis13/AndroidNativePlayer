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
    
  bool isPlaying;
  int dataChannels;
  
  unique_ptr<float[]> mSampleData;
  
  int samplesProcessed;
  int totalSamples;
  int nextSampleId;
  
  int32_t mNumChannels;
  int32_t mSampleRate;
  int32_t mNumSamples;
  
  AudioDecoder* decoder;
  
  SharedQueue dataQ;

  FilePlayer() {
    mSampleData = nullptr;
    isPlaying = false;
    dataChannels = 0;
    samplesProcessed = 0;
  }

  oboe::Result open();
  oboe::Result start();
  oboe::Result stop();
  oboe::Result close();
  
  bool loadFile(string audioPath);
  
  bool loadFileQueueStatic(string audioPath);
  bool loadFileFull(string audioPath);
  bool loadFileWav(string audioPath);
  void play();

  ifstream file;
  
  ofstream dumpfile;
  string dumppath = "/storage/emulated/0/_temp/dump.wav";

private:

  void resampleData(int destSampleRate);


class MyDataCallback : public AudioStreamDataCallback {
public:
    MyDataCallback(FilePlayer *parent) : mParent(parent) {
      currentSampleId = 0;
    }
    
    DataCallbackResult onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) override;

private:
    FilePlayer *mParent;
    int currentSampleId;
};


class MyErrorCallback : public AudioStreamErrorCallback {
public:
    MyErrorCallback(FilePlayer *parent) : mParent(parent) {}

    virtual ~MyErrorCallback() {
    }

    void onErrorAfterClose(AudioStream *oboeStream, oboe::Result error) override;

private:
    FilePlayer *mParent;
};


  static constexpr int kChannelCount = 2;
  
  shared_ptr<AudioStream> mStream;
  
  shared_ptr<MyDataCallback> mDataCallback;
  shared_ptr<MyErrorCallback> mErrorCallback;

};

#endif //FILE_PLAYER_H
