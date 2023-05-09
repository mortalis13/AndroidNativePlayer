#ifndef FILE_PLAYER_H
#define FILE_PLAYER_H

#include <string>

#include "oboe/Oboe.h"

#include "AudioDecoder.h"
#include "defs.h"

using namespace std;
using namespace oboe;


class FilePlayer {
public:
  FilePlayer() {
    isPlaying = false;
  }
  
  ~FilePlayer() {
    if (decoder != NULL) delete decoder;
  }
  
  bool open();
  bool start();
  oboe::Result stop();
  oboe::Result close();
  
  bool init();
  bool play(string audioPath);


private:
  bool loadFile(string audioPath);
  void writeAudio(float* stream, int32_t numFrames);

  void emptyQueue();


private:
  static constexpr int kChannelCount = 2;

  AudioDecoder* decoder = NULL;
  SharedQueue dataQ;

  bool isPlaying;


  class MyDataCallback;
  class MyErrorCallback;

  shared_ptr<AudioStream> mStream;
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
