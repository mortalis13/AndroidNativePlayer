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
  file.close();
  return mStream->close();
}

void FilePlayer::play() {
  if (file.tellg() == -1) file.clear();
  file.seekg(0);
  isPlaying = true;
}

bool FilePlayer::setFile(string audioPath) {
  this->audioPath = audioPath;
  this->dataChannels = 1;
  
  file.open(this->audioPath, ios::binary | ios::ate);
  
  if (!file.good()) {
    __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to open file: %s => %s", this->audioPath.c_str(), strerror(errno));
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
DataCallbackResult FilePlayer::MyDataCallback::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
  if (!mParent->isPlaying) {
    return DataCallbackResult::Continue;
  }
  
  uint16_t* stream = (uint16_t*) audioData;

  for (int i = 0; i < numFrames; i++) {
    uint16_t sample = 0;
    
    for (int ch = 0; ch < mParent->dataChannels; ch++) {
      if (!mParent->file.read((char*) &sample, 2)) {
        mParent->isPlaying = false;
        return DataCallbackResult::Continue;
      }
      
      *stream++ = sample;
    }
    
    if (mParent->dataChannels == 1) {
      *stream++ = sample;
    }
  }
  
  
  // for (int i = 0; i < numFrames; i++) {
  //   mParent->file.read((char*) stream, 2);
  //   stream[1] = stream[0];
  //   stream += 2;
  // }
  
  
  // char* stream = (char*) audioData;
  // int size = numFrames * 2 * mParent->dataChannels;
  
  // int x = 0;
  // for (int i = 0; i < size; i+=2) {
  //   char buf1;
  //   char buf2;
    
  //   if (!mParent->file.read(&buf1, 1)) break;
  //   if (!mParent->file.read(&buf2, 1)) break;
    
  //   stream[x++] = buf1;
  //   stream[x++] = buf2;

  //   stream[x++] = buf1;
  //   stream[x++] = buf2;
  // }
  
  return DataCallbackResult::Continue;
}


void FilePlayer::MyErrorCallback::onErrorAfterClose(AudioStream *oboeStream, oboe::Result error) {
  __android_log_print(ANDROID_LOG_ERROR, TAG, "%s() - error = %s", __func__, oboe::convertToText(error));
  if (mParent->open() == Result::OK) {
    mParent->start();
  }
}
