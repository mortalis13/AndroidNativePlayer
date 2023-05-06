#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <string>
#include <cstdint>

using namespace std;


class AudioDecoder {

public:
  int64_t decode(string filePath, uint8_t* targetData, int32_t channelCount, int32_t sampleRate);
  
  int channels = 0;


private:
  static bool createAVFormatContext(AVFormatContext **avFormatContext);
  
  static bool openAVFormatContext(AVFormatContext *avFormatContext);

  static bool getStreamInfo(AVFormatContext *avFormatContext);
  
  static AVStream* getBestAudioStream(AVFormatContext* avFormatContext);

  static void printCodecParameters(AVCodecParameters* params);
  
};

#endif //AUDIO_DECODER_H
