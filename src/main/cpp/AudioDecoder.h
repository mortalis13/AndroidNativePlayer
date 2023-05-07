#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <string>
#include <future>

#include "defs.h"

using namespace std;


class AudioDecoder {

public:
  AudioDecoder() {}
  AudioDecoder(SharedQueue* dataQ) {
    this->dataQ = dataQ;
    this->enabled = false;
    this->playing = false;
  }
  
  int loadFile(string filePath);
  void start();
  void stop();
  
  void run();
  
  bool enabled;
  bool playing;
  
  void setChannelCount(int32_t channelCount) {
    this->channelCount = channelCount;
  }
  
  void setSampleRate(int32_t sampleRate) {
    this->sampleRate = sampleRate;
  }
  
  int64_t decode(string filePath, uint8_t* targetData, int32_t channelCount, int32_t sampleRate);
  
  int32_t in_channels = 0;
  int32_t channelCount = 0;
  int32_t sampleRate = 0;


private:
  static void printCodecParameters(AVCodecParameters* params);
  
  void cleanup();
  
  AVFormatContext *formatContext = NULL;
  AVCodecContext *codecContext = NULL;
  SwrContext *swr = NULL;
  
  AVStream *stream;
  AVCodec *codec;
  
  SharedQueue* dataQ;
  
  // thread runThread;
  future<void> runThread;
  
};

#endif //AUDIO_DECODER_H
