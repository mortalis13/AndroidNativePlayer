#include <stdlib.h>
#include <vector>
#include <algorithm>

#include <android/log.h>

#include "FilePlayer.h"


static const char *TAG = "FilePlayer";


oboe::Result FilePlayer::open() {
  isPlaying = false;
  
  mDataCallback = make_shared<MyDataCallback>(this);
  mErrorCallback = make_shared<MyErrorCallback>(this);

  AudioStreamBuilder builder;

  builder.setSharingMode(SharingMode::Exclusive);
  builder.setPerformanceMode(PerformanceMode::LowLatency);
  builder.setFormat(AudioFormat::I16);
  builder.setChannelCount(kChannelCount);
  builder.setDataCallback(mDataCallback);
  builder.setErrorCallback(mErrorCallback);
  builder.setSampleRate(44100);

  oboe::Result result = builder.openStream(mStream);
  return result;
}

oboe::Result FilePlayer::start() {
  return mStream->requestStart();
}

oboe::Result FilePlayer::stop() {
  return mStream->requestStop();
}

oboe::Result FilePlayer::close() {
  return mStream->close();
}

void FilePlayer::play() {
  this->isPlaying = true;
  this->samplesProcessed = 0;
  this->nextSampleId = 0;
}

bool FilePlayer::loadFile(string audioPath) {
  this->audioPath = audioPath;
  this->dataChannels = 2;
  
  file.open(this->audioPath, ios::binary | ios::ate);
  
  if (!file.good()) {
    __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to open file: %s => %s", this->audioPath.c_str(), strerror(errno));
    return false;
  }
  
  int size = file.tellg();
  this->totalSamples = size / sizeof(uint16_t);
  __android_log_print(ANDROID_LOG_INFO, TAG, "Size: %d", size);
  file.seekg(0);
  
  if (fileBuffer != NULL) {
    delete fileBuffer;
  }
  fileBuffer = new uint16_t[size];
  
  file.read((char*) fileBuffer, size);
  file.close();
  
  return true;
}


DataCallbackResult FilePlayer::MyDataCallback::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
  if (!mParent->isPlaying) {
    memset(audioData, 0, numFrames * kChannelCount * sizeof(uint16_t));
    return DataCallbackResult::Continue;
  }
  
  uint16_t* stream = (uint16_t*) audioData;
  
  for (int i = 0; i < numFrames; i++) {
    uint16_t sample = 0;
    
    for (int ch = 0; ch < mParent->dataChannels; ch++) {
      if (mParent->nextSampleId < mParent->totalSamples) {
        sample = mParent->fileBuffer[mParent->nextSampleId++];
      }
      
      *stream++ = sample;
      mParent->samplesProcessed++;
    }
    
    if (mParent->dataChannels == 1) {
      *stream++ = sample;
    }
  }
  
  if (mParent->nextSampleId >= mParent->totalSamples) {
    mParent->isPlaying = false;
    __android_log_print(ANDROID_LOG_INFO, TAG, "samplesProcessed: %d, last sample ID: %d", mParent->samplesProcessed, mParent->nextSampleId);
  }
  
  return DataCallbackResult::Continue;
}


void FilePlayer::MyErrorCallback::onErrorAfterClose(AudioStream *oboeStream, oboe::Result error) {
  __android_log_print(ANDROID_LOG_ERROR, TAG, "%s() - error = %s", __func__, oboe::convertToText(error));
  if (mParent->open() == Result::OK) {
    mParent->start();
  }
}
