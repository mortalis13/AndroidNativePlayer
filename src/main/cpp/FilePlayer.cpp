#define LOG_MODULE_NAME "FilePlayer_"

#include "FilePlayer.h"

#include "utils/logging.h"


bool FilePlayer::init() {
  isPlaying = false;
  
  if (!this->open()) return false;
  if (!this->start()) return false;
  
  if (this->decoder != NULL) {
    this->decoder->stop();
    delete this->decoder;
  }
  this->decoder = new AudioDecoder(&dataQ);
  this->decoder->setChannelCount(mStream->getChannelCount());
  this->decoder->setSampleRate(mStream->getSampleRate());
  
  return true;
}

bool FilePlayer::open() {
  mDataCallback = make_shared<MyDataCallback>(this);
  mErrorCallback = make_shared<MyErrorCallback>(this);

  AudioStreamBuilder builder;

  builder.setSharingMode(SharingMode::Exclusive);
  builder.setPerformanceMode(PerformanceMode::LowLatency);
  builder.setFormat(AudioFormat::Float);
  builder.setFormatConversionAllowed(true);
  builder.setChannelCount(kChannelCount);
  builder.setDataCallback(mDataCallback);
  builder.setErrorCallback(mErrorCallback);
  builder.setSampleRate(48000);
  builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);

  auto result = builder.openStream(mStream);
  if (result != Result::OK) {
    LOGE("Failed to open stream. Error: %s", convertToText(result));
    return false;
  }
  return true;
}

bool FilePlayer::start() {
  auto result = mStream->requestStart();
  if (result != Result::OK) {
    LOGE("Failed to start stream. Error: %s", convertToText(result));
    return false;
  }
  return true;
}

oboe::Result FilePlayer::stop() {
  this->isPlaying = false;
  this->decoder->stop();
  return mStream->requestStop();
}

oboe::Result FilePlayer::close() {
  return mStream->close();
}


bool FilePlayer::play(string audioPath) {
  this->isPlaying = false;
  this->decoder->stop();
  this->emptyQueue();
  
  bool result = loadFile(audioPath);
  if (!result) return result;
  
  this->isPlaying = true;
  
  this->decoder->start();
  LOGI("Decoder started");

  return true;
}


void FilePlayer::emptyQueue() {
  float sample;
  while (this->dataQ.try_dequeue(sample)) {}
  LOGI("Queue emptied");
}


bool FilePlayer::loadFile(string audioPath) {
  int result = this->decoder->loadFile(audioPath);
  if (result == -1) return false;
  return true;
}


void FilePlayer::writeAudio(float* stream, int32_t numFrames) {
  // Audio thread
  for (int i = 0; i < numFrames * kChannelCount; i++) {
    float sample = 0;
    
    if (this->isPlaying) {
      bool popped = this->dataQ.try_dequeue(sample);
    }
    
    *stream++ = sample;
  }
}


DataCallbackResult FilePlayer::MyDataCallback::onAudioReady(AudioStream* audioStream, void* audioData, int32_t numFrames) {
  if (!mParent->isPlaying) {
    memset(audioData, 0, numFrames * kChannelCount * sizeof(float));
    return DataCallbackResult::Continue;
  }
  
  float* stream = (float*) audioData;
  mParent->writeAudio(stream, numFrames);
  
  return DataCallbackResult::Continue;
}


void FilePlayer::MyErrorCallback::onErrorAfterClose(AudioStream* oboeStream, oboe::Result error) {
  LOGE("%s() - error = %s", __func__, oboe::convertToText(error));
  mParent->init();
}
