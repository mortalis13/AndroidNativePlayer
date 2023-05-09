#define LOG_MODULE_NAME "AudioDecoder_"

#include "AudioDecoder.h"

#include "utils/logging.h"


void AudioDecoder::start() {
  LOGD("AudioDecoder::start()");
  this->enabled = true;
  this->playing = true;
  
  runThread = std::async(&AudioDecoder::run, this);
  // runThread = thread(&AudioDecoder::run, this);
  LOGI("Decoder thread stated");
}


void AudioDecoder::stop() {
  LOGD("AudioDecoder::stop()");
  
  this->playing = false;
  this->enabled = false;
  
  if (runThread.valid()) {
    LOGI("--wating for thread");
    runThread.wait();
    LOGI("--after wait");
  }
  
  this->cleanup();
  LOGI("Decoder stopped");
}


void AudioDecoder::run() {
  // Decoder thread
  LOGD("AudioDecoder::run()");
  this->decodeFrames();
  LOGI("Decoder thread ended");
}


int64_t AudioDecoder::decodeFrames() {
  int result;
  AVPacket avPacket; // Stores compressed audio data
  av_init_packet(&avPacket);
  AVFrame* decodedFrame = av_frame_alloc(); // Stores raw audio data

  int bytesWritten = 0;

  int bytesPerSample = av_get_bytes_per_sample((AVSampleFormat) stream->codecpar->format);
  LOGD("Bytes per sample %d", bytesPerSample);
  
  // Seek to start
  result = av_seek_frame(formatContext, stream->index, 0, AVSEEK_FLAG_BYTE);
  if (result < 0) {
    LOGE("Error when seeking to the start of the stream");
  }
  
  LOGD("DECODE START");
  
  while (this->enabled) {
    if (this->playing) {
      result = av_read_frame(formatContext, &avPacket);
      if (result != 0) break;
      
      if (avPacket.stream_index == stream->index && avPacket.size > 0) {
        // Pass our compressed data into the codec
        result = avcodec_send_packet(codecContext, &avPacket);
        if (result != 0) {
          LOGE("avcodec_send_packet error: %s", av_err2str(result));
          this->cleanup();
          break;
        }

        // Retrieve our raw data from the codec
        result = avcodec_receive_frame(codecContext, decodedFrame);
        if (result == AVERROR(EAGAIN)) {
          // The codec needs more data before it can decode
          LOGI("avcodec_receive_frame returned EAGAIN");
          av_packet_unref(&avPacket);
          continue;
        }
        else if (result != 0) {
          LOGE("avcodec_receive_frame error: %s", av_err2str(result));
          this->cleanup();
          break;
        }

        // DO RESAMPLING
        auto dst_nb_samples = (int32_t) av_rescale_rnd(
            swr_get_delay(swr, decodedFrame->sample_rate) + decodedFrame->nb_samples,
            sampleRate, decodedFrame->sample_rate, AV_ROUND_UP);
        
        short* buffer;
        av_samples_alloc((uint8_t **) &buffer, nullptr, this->channelCount, dst_nb_samples, AV_SAMPLE_FMT_FLT, 0);
        int frame_count = swr_convert(swr, (uint8_t **) &buffer, dst_nb_samples,
                                      (const uint8_t **) decodedFrame->data, decodedFrame->nb_samples);

        int64_t bytesToWrite = frame_count * sizeof(float) * this->channelCount;
        saveFrame(buffer, bytesWritten, bytesToWrite);
        bytesWritten += bytesToWrite;

        av_freep(&buffer);
        av_frame_unref(decodedFrame);
        av_packet_unref(&avPacket);
      }
    }
  }
  
  this->enabled = false;
  this->playing = false;
  
  av_frame_unref(decodedFrame);
  av_frame_free(&decodedFrame);
  
  return bytesWritten;
}


void AudioDecoder::saveFrame(short* buffer, int64_t bytesWritten, int64_t bytesToWrite) {
  if (this->mode == MODE_STATIC_BUFFER) {
    memcpy(targetData + bytesWritten, buffer, (size_t) bytesToWrite);
  }
  else if (this->mode == MODE_BUFFER_QUEUE) {
    int pushedBytes = 0;
    
    while (pushedBytes < bytesToWrite) {
      if (!this->enabled) break;
      
      float sample;
      memcpy(&sample, (uint8_t*) buffer + pushedBytes, sizeof(float));
      
      bool pushed = this->dataQ->push(sample);
      if (!pushed) continue;
      
      pushedBytes += sizeof(float);
    }
  }
}


int AudioDecoder::loadFile(string filePath) {
  LOGI("Decoder: FFMpeg => %s", filePath.c_str());
  int result = -1;
  
  result = avformat_open_input(&formatContext, filePath.c_str(), NULL, NULL);
  if (result != 0) {
    LOGE("Failed to open file. Error code %s", av_err2str(result));
    this->cleanup();
    return result;
  }
  
  result = avformat_find_stream_info(formatContext, NULL);
  if (result != 0) {
    LOGE("Failed to find stream info. Error code %s", av_err2str(result));
    this->cleanup();
    return result;
  }
  
  // Obtain the best audio stream to decode
  int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  stream = nullptr;
  if (streamIndex >= 0) {
    stream = formatContext->streams[streamIndex];
  }
  
  if (stream == nullptr || stream->codecpar == nullptr) {
    LOGE("Could not find a suitable audio stream to decode");
    this->cleanup();
    return -1;
  }

  printCodecParameters(stream->codecpar);
  this->dataChannels = stream->codecpar->channels;

  // Find the codec to decode this stream
  codec = avcodec_find_decoder(stream->codecpar->codec_id);
  if (!codec){
    LOGE("Could not find codec with ID: %d", stream->codecpar->codec_id);
    this->cleanup();
    return -1;
  }
  
  codecContext = avcodec_alloc_context3(codec);
  if (!codecContext){
    LOGE("Failed to allocate codec context");
    this->cleanup();
    return -1;
  }

  // Copy the codec parameters into the context
  if (avcodec_parameters_to_context(codecContext, stream->codecpar) < 0){
    LOGE("Failed to copy codec parameters to codec context");
    this->cleanup();
    return -1;
  }

  // Open the codec
  if (avcodec_open2(codecContext, codec, nullptr) < 0){
    LOGE("Could not open codec");
    this->cleanup();
    return -1;
  }

  // prepare resampler
  int32_t outChannelLayout = (1 << this->channelCount) - 1;
  LOGD("Channel layout %d", outChannelLayout);

  swr = swr_alloc();
  av_opt_set_int(swr, "in_channel_count", stream->codecpar->channels, 0);
  av_opt_set_int(swr, "out_channel_count", this->channelCount, 0);
  av_opt_set_int(swr, "in_channel_layout", stream->codecpar->channel_layout, 0);
  av_opt_set_int(swr, "out_channel_layout", outChannelLayout, 0);
  av_opt_set_int(swr, "in_sample_rate", stream->codecpar->sample_rate, 0);
  av_opt_set_int(swr, "out_sample_rate", this->sampleRate, 0);
  av_opt_set_int(swr, "in_sample_fmt", stream->codecpar->format, 0);
  av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
  av_opt_set_int(swr, "force_resampling", 1, 0);

  // Check that resampler has been inited
  result = swr_init(swr);
  if (result != 0){
    LOGE("swr_init failed. Error: %s", av_err2str(result));
    this->cleanup();
    return result;
  };
  
  if (!swr_is_initialized(swr)) {
    LOGE("swr_is_initialized is false\n");
    this->cleanup();
    return -1;
  }

  return 0;
}


void AudioDecoder::cleanup() {
  LOGD("AudioDecoder::cleanup()");
  avformat_close_input(&formatContext);
  avcodec_free_context(&codecContext);
  swr_free(&swr);
}


int64_t AudioDecoder::decodeStatic(string filePath) {
  int result = loadFile(filePath);
  if (result == -1) return -1;
  
  int64_t bytesWritten = decodeFrames();
  cleanup();
  return bytesWritten;
}


void AudioDecoder::printCodecParameters(AVCodecParameters* params) {
  LOGD("Stream properties:");
  LOGD("Channels: %d", params->channels);
  LOGD("Channel layout: %" PRId64, params->channel_layout);
  LOGD("Sample rate: %d", params->sample_rate);
  LOGD("Format: %s", av_get_sample_fmt_name((AVSampleFormat) params->format));
  LOGD("Frame size: %d", params->frame_size);
  LOGD("END Stream properties");
}
