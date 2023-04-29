#include <stdlib.h>
#include <vector>
#include <algorithm>

#include <android/log.h>

#include "SimpleNoiseMaker.h"

using namespace oboe;


static const char *TAG = "SimpleNoiseMaker";


oboe::Result SimpleNoiseMaker::open() {
  isPlaying = false;
  
  mDataCallback = std::make_shared<MyDataCallback>(this);
  mErrorCallback = std::make_shared<MyErrorCallback>(this);

  AudioStreamBuilder builder;

  builder.setSharingMode(oboe::SharingMode::Exclusive);
  builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
  builder.setFormat(oboe::AudioFormat::I16);
  builder.setChannelCount(kChannelCount);
  builder.setDataCallback(mDataCallback);
  builder.setErrorCallback(mErrorCallback);
  builder.setSampleRate(44100);

  oboe::Result result = builder.openStream(mStream);
  return result;
}

oboe::Result SimpleNoiseMaker::start() {
  return mStream->requestStart();
}

oboe::Result SimpleNoiseMaker::stop() {
  return mStream->requestStop();
}

oboe::Result SimpleNoiseMaker::close() {
  file.close();
  return mStream->close();
}

void SimpleNoiseMaker::play() {
  if (file.tellg() == -1) file.clear();
  file.seekg(0);
  isPlaying = true;
}

bool SimpleNoiseMaker::setFile(string audioPath) {
  this->audioPath = audioPath;
  file.open(this->audioPath, ios::binary | ios::ate);
  
  if (!file.good()) {
    __android_log_print(ANDROID_LOG_ERROR, TAG, "[ERROR]: Failed to open file: %s => %s", this->audioPath.c_str(), strerror(errno));
    return false;
  }
  
  int size = file.tellg();
  __android_log_print(ANDROID_LOG_INFO, TAG, "Size: %d", size);
  file.seekg(0);
  
  return true;
}


/**
 * This callback method will be called from a high priority audio thread.
 * It should only do math and not do any blocking operations like
 * reading or writing files, memory allocation, or networking.
 * 
 * @param audioStream
 * @param audioData pointer to an array of samples to be filled
 * @param numFrames number of frames needed
 * @return
 */
DataCallbackResult SimpleNoiseMaker::MyDataCallback::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
  if (!mParent->isPlaying) {
    return oboe::DataCallbackResult::Continue;
  }
  
  uint8_t * stream = (uint8_t*) audioData;
  
  int size = numFrames * 2 * kChannelCount;
  
  if (!mParent->file.read((char*) stream, size)) {
    // __android_log_print(ANDROID_LOG_INFO, TAG, "Last read / requested: %d / %d", (int) mParent->file.gcount(), size);
  }
  
  return oboe::DataCallbackResult::Continue;
}


void SimpleNoiseMaker::MyErrorCallback::onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) {
  __android_log_print(ANDROID_LOG_ERROR, TAG, "%s() - error = %s", __func__, oboe::convertToText(error));
  if (mParent->open() == Result::OK) {
    mParent->start();
  }
}
