#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <string>
#include <fstream>
#include <future>

#include "defs.h"

using namespace std;


const int MODE_STATIC_BUFFER = 0;
const int MODE_BUFFER_QUEUE = 1;


class AudioDecoder {

public:
  AudioDecoder(uint8_t* targetData) {
    this->mode = MODE_STATIC_BUFFER;
    this->targetData = targetData;
    this->enabled = true;
    this->playing = true;
  }
  
  AudioDecoder(SharedQueue* dataQ) {
    this->mode = MODE_BUFFER_QUEUE;
    this->dataQ = dataQ;
    this->enabled = false;
    this->playing = false;
  }
  
  int loadFile(string filePath);
  void start();
  void stop();
  
  void setChannelCount(int32_t channelCount) {
    this->channelCount = channelCount;
  }
  
  void setSampleRate(int32_t sampleRate) {
    this->sampleRate = sampleRate;
  }
  
  int32_t getDataChannels() {
    return dataChannels;
  }
  
  int64_t decodeStatic(string filePath);


private:
  static void printCodecParameters(AVCodecParameters* params);
  
  void run();
  void cleanup();
  int64_t decodeFrames();
  void saveFrame(short* buffer, int64_t bytesWritten, int64_t bytesToWrite);
  
  bool enabled;
  bool playing;
  
  int mode;
  
  int32_t channelCount = 0;
  int32_t sampleRate = 0;
  int32_t dataChannels = 0;
  
  AVFormatContext* formatContext = NULL;
  AVCodecContext* codecContext = NULL;
  SwrContext* swr = NULL;
  
  AVStream* stream = NULL;
  AVCodec* codec = NULL;
  
  SharedQueue* dataQ = NULL;
  uint8_t* targetData = NULL;
  
  // thread runThread;
  future<void> runThread;
  
  ofstream dumpfile;
  string dumppath = "/tmp/dump.wav";  // replaced of audio file loading
  bool dumpOutput = true;
  
};

#endif //AUDIO_DECODER_H
