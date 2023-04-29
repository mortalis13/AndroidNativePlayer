#include <stdlib.h>
#include <vector>
#include <algorithm>

static const char *TAG = "SimpleNoiseMaker";

#include <android/log.h>

#include "SimpleNoiseMaker.h"

using namespace oboe;

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
  
  // // -----
  // char* buf = new char[88200];
  // while (file.read((char*) buf, 100000)) {
  //   __android_log_print(ANDROID_LOG_INFO, TAG, "Data: 0x%02x", buf[0]);
  // }
  // __android_log_print(ANDROID_LOG_INFO, TAG, "Data-end: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
  // // -----
  
  // // -----
  // char* buf = new char[1];
  // while (file.read((char*) buf, 1)) {
  //   __android_log_print(ANDROID_LOG_INFO, TAG, "Data: 0x%02x", buf[0]);
  // }
  // // -----
  
  // // -----
  // istream_iterator<uint8_t> begin(file), end;
  // auto fileData = vector<uint8_t>(begin, end);
  // __android_log_print(ANDROID_LOG_INFO, TAG, "Size: %d", (int) fileData.size());
  // for (auto v: fileData) {
  //   __android_log_print(ANDROID_LOG_INFO, TAG, "Data: 0x%02x", v);
  // }
  // // -----
  
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
  // __android_log_print(ANDROID_LOG_INFO, TAG, "Reading %d frames. Sample rate: %d", numFrames, audioStream->getSampleRate());
  
  if (!mParent->isPlaying) {
    return oboe::DataCallbackResult::Continue;
  }
  
  // ----
  uint8_t * stream = (uint8_t*) audioData;
  
  int size = numFrames * 2 * kChannelCount;
  
  if (!mParent->file.read((char*) stream, size)) {
    // __android_log_print(ANDROID_LOG_INFO, TAG, "Last read / requested: %d / %d", (int) mParent->file.gcount(), size);
  }
  
  return oboe::DataCallbackResult::Continue;
  // ----
  
  
  // // ----
  // uint8_t * stream = (uint8_t*) audioData;
  // int frame = 0;
  
  // while (mParent->file.read((char*) stream++, 1)) {
  //   if (mParent->file.tellg() % 2 == 0) frame++;
  //   if (frame >= numFrames) break;
  // }
  
  // if (frame < numFrames) {
  //   return oboe::DataCallbackResult::Stop;
  // }
  
  // return oboe::DataCallbackResult::Continue;
  // // ----
  
  
  // // ----
  // uint8_t * stream = (uint8_t*) audioData;
  // int frame = 0;
  
  // while (mParent->file.read((char*) stream, 2) && frame < numFrames) {
  //   __android_log_print(ANDROID_LOG_INFO, TAG, "Data: 0x%02x 0x%02x", *stream, *(stream+1));
  //   frame++;
  //   stream += 2;
  // }
  
  // if (frame < numFrames) {
  //   return oboe::DataCallbackResult::Stop;
  // }
  
  // return oboe::DataCallbackResult::Continue;
  // // ----
  
  
  // ----
  // auto stream = static_cast<int16_t*>(audioData);
  
  // istream_iterator<uint8_t> begin(mParent->file), end;
  // auto fileData = vector<uint8_t>(begin, end);
  
  // int leftData = (int) fileData.size() - currentSampleId*2;
  // auto size = std::min(numFrames, leftData);
  
  // __android_log_print(ANDROID_LOG_INFO, TAG, "Size to read: %d", size);
  
  // if (size <= 0) {
  //   __android_log_print(ANDROID_LOG_INFO, TAG, "Data ended: %d", size);
  //   currentSampleId = 0;
  //   return oboe::DataCallbackResult::Stop;
  // }
  
  // for (int i = 0; i < size; i++) {
  //   __android_log_print(ANDROID_LOG_INFO, TAG, "Data[%d]: 0x%02x 0x%02x", currentSampleId, fileData[currentSampleId*2], fileData[currentSampleId*2+1]);
    
  //   int16_t sample = (fileData[currentSampleId*2+1] << 8) | fileData[currentSampleId*2];
  //   stream[i] = sample;
  //   currentSampleId++;
  // }
  
  // return oboe::DataCallbackResult::Continue;
  // ----
  
  
  // ----
  // mParent->dummy();
  // int numSamples = numFrames * kChannelCount;
  // __android_log_print(ANDROID_LOG_INFO, TAG, "Reading %d frames. Sample rate: %d", numFrames, audioStream->getSampleRate());
  
  // if (mParent->file.read((char*) audioData, numSamples)) {
  //   return oboe::DataCallbackResult::Continue;
  // }
  
  // return oboe::DataCallbackResult::Stop;
  // ----
  
  
  // ----
  // float *output = (float *) audioData;
  // for (int i = 0; i < numSamples; i++) {
  //     *output++ = (float) ((drand48() - 0.5) * 0.6);
  // }
  // return oboe::DataCallbackResult::Continue;
  // ----
}

void SimpleNoiseMaker::MyErrorCallback::onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) {
  __android_log_print(ANDROID_LOG_ERROR, TAG, "%s() - error = %s", __func__, oboe::convertToText(error));
  
  // Try to open and start a new stream after a disconnect.
  if (mParent->open() == Result::OK) {
    mParent->start();
  }
}
