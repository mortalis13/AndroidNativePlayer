#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <string>
#include <cstdint>

#include "defs.h"

using namespace std;


class AudioDecoder {

public:
  AudioDecoder() {}
  AudioDecoder(SharedQueue* dataQ) {
    this->dataQ = dataQ;
    this->enabled = false;
  }
  
  int initForFile(string filePath);
  void start();
  
  void run();
  bool enabled;
  
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
  static AVStream* getBestAudioStream(AVFormatContext* avFormatContext);

  static void printCodecParameters(AVCodecParameters* params);
  
  void cleanup();
  
  AVFormatContext *formatContext = NULL;
  AVStream *stream;
  AVCodec *codec;
  AVCodecContext *codecContext = NULL;
  SwrContext *swr;
  
  SharedQueue* dataQ;
  
};

#endif //AUDIO_DECODER_H
