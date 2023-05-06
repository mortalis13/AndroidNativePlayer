#include "AudioDecoder.h"

#include <memory>

#include "logging.h"


constexpr int kInternalBufferSize = 1152; // Use MP3 block size. https://wiki.hydrogenaud.io/index.php?title=MP3


int64_t AudioDecoder::decode(string filePath, uint8_t* targetData, int32_t channelCount, int32_t sampleRate) {
  LOGI("Decoder: FFMpeg");
  int result = -1;

  // Create a buffer for FFmpeg to use for decoding (freed in the custom deleter below)
  auto buffer = reinterpret_cast<uint8_t*>(av_malloc(kInternalBufferSize));
  
  AVFormatContext *formatContext = NULL;
  result = avformat_open_input(&formatContext, filePath.c_str(), NULL, NULL);
  if (result != 0) {
    LOGE("Failed to open file. Error code %s", av_err2str(result));
    return result;
  }
  
  result = avformat_find_stream_info(formatContext, NULL);
  if (result != 0) {
    LOGE("Failed to find stream info. Error code %s", av_err2str(result));
    return result;
  }
  
  // Obtain the best audio stream to decode
  AVStream *stream = getBestAudioStream(formatContext);
  if (stream == nullptr || stream->codecpar == nullptr){
      LOGE("Could not find a suitable audio stream to decode");
      return -1;
  }

  printCodecParameters(stream->codecpar);
  this->channels = stream->codecpar->channels;

  // Find the codec to decode this stream
  AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
  if (!codec){
      LOGE("Could not find codec with ID: %d", stream->codecpar->codec_id);
      return -1;
  }

  // Create the codec context, specifying the deleter function
  unique_ptr<AVCodecContext, void(*)(AVCodecContext *)> codecContext {
      nullptr,
      [](AVCodecContext *c) { avcodec_free_context(&c); }
  };
  {
      AVCodecContext *tmp = avcodec_alloc_context3(codec);
      if (!tmp){
          LOGE("Failed to allocate codec context");
          return -1;
      }
      codecContext.reset(tmp);
  }

  // Copy the codec parameters into the context
  if (avcodec_parameters_to_context(codecContext.get(), stream->codecpar) < 0){
      LOGE("Failed to copy codec parameters to codec context");
      return -1;
  }

  // Open the codec
  if (avcodec_open2(codecContext.get(), codec, nullptr) < 0){
      LOGE("Could not open codec");
      return -1;
  }

  // prepare resampler
  int32_t outChannelLayout = (1 << channelCount) - 1;
  LOGD("Channel layout %d", outChannelLayout);

  SwrContext *swr = swr_alloc();
  av_opt_set_int(swr, "in_channel_count", stream->codecpar->channels, 0);
  av_opt_set_int(swr, "out_channel_count", channelCount, 0);
  av_opt_set_int(swr, "in_channel_layout", stream->codecpar->channel_layout, 0);
  av_opt_set_int(swr, "out_channel_layout", outChannelLayout, 0);
  av_opt_set_int(swr, "in_sample_rate", stream->codecpar->sample_rate, 0);
  av_opt_set_int(swr, "out_sample_rate", sampleRate, 0);
  av_opt_set_int(swr, "in_sample_fmt", stream->codecpar->format, 0);
  av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
  av_opt_set_int(swr, "force_resampling", 1, 0);

  // Check that resampler has been inited
  result = swr_init(swr);
  if (result != 0){
      LOGE("swr_init failed. Error: %s", av_err2str(result));
      return result;
  };
  
  if (!swr_is_initialized(swr)) {
      LOGE("swr_is_initialized is false\n");
      return -1;
  }

  // Prepare to read data
  int bytesWritten = 0;
  AVPacket avPacket; // Stores compressed audio data
  av_init_packet(&avPacket);
  AVFrame *decodedFrame = av_frame_alloc(); // Stores raw audio data
  int bytesPerSample = av_get_bytes_per_sample((AVSampleFormat)stream->codecpar->format);

  LOGD("Bytes per sample %d", bytesPerSample);

  LOGD("DECODE START");

  // While there is more data to read, read it into the avPacket
  while (av_read_frame(formatContext, &avPacket) == 0){
    if (avPacket.stream_index == stream->index && avPacket.size > 0) {
      // Pass our compressed data into the codec
      result = avcodec_send_packet(codecContext.get(), &avPacket);
      if (result != 0) {
          LOGE("avcodec_send_packet error: %s", av_err2str(result));
          goto cleanup;
      }

      // Retrieve our raw data from the codec
      result = avcodec_receive_frame(codecContext.get(), decodedFrame);
      if (result == AVERROR(EAGAIN)) {
          // The codec needs more data before it can decode
          LOGI("avcodec_receive_frame returned EAGAIN");
          av_packet_unref(&avPacket);
          continue;
      }
      else if (result != 0) {
          LOGE("avcodec_receive_frame error: %s", av_err2str(result));
          goto cleanup;
      }

      // DO RESAMPLING
      auto dst_nb_samples = (int32_t) av_rescale_rnd(
          swr_get_delay(swr, decodedFrame->sample_rate) + decodedFrame->nb_samples,
          sampleRate, decodedFrame->sample_rate, AV_ROUND_UP);

      short *buffer1;
      av_samples_alloc((uint8_t **) &buffer1, nullptr, channelCount, dst_nb_samples, AV_SAMPLE_FMT_FLT, 0);
      int frame_count = swr_convert(swr, (uint8_t **) &buffer1, dst_nb_samples,
                                    (const uint8_t **) decodedFrame->data, decodedFrame->nb_samples);

      int64_t bytesToWrite = frame_count * sizeof(float) * channelCount;
      memcpy(targetData + bytesWritten, buffer1, (size_t)bytesToWrite);
      bytesWritten += bytesToWrite;
      av_freep(&buffer1);

      av_packet_unref(&avPacket);
    }
  }

  av_frame_free(&decodedFrame);
  avformat_close_input(&formatContext);
  
  LOGD("DECODE END");

  result = bytesWritten;

  cleanup:
  return result;
}


AVStream* AudioDecoder::getBestAudioStream(AVFormatContext* avFormatContext) {
  int streamIndex = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

  if (streamIndex < 0) {
    LOGE("Could not find stream");
    return nullptr;
  }
  else {
    return avFormatContext->streams[streamIndex];
  }
}


void AudioDecoder::printCodecParameters(AVCodecParameters* params) {
  LOGD("Stream properties");
  LOGD("Channels: %d", params->channels);
  LOGD("Channel layout: %" PRId64, params->channel_layout);
  LOGD("Sample rate: %d", params->sample_rate);
  LOGD("Format: %s", av_get_sample_fmt_name((AVSampleFormat)params->format));
  LOGD("Frame size: %d", params->frame_size);
}
