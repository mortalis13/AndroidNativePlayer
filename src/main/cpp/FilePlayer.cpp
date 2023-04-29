#include "FilePlayer.h"

#include <stdlib.h>
#include <vector>
#include <algorithm>

#include <android/log.h>

#include <stream/MemInputStream.h>
#include <wav/WavStreamReader.h>

#include <resampler/MultiChannelResampler.h>

using namespace parselib;
using namespace RESAMPLER_OUTER_NAMESPACE::resampler;


static const char *TAG = "FilePlayer";


oboe::Result FilePlayer::open() {
  isPlaying = false;
  
  mDataCallback = make_shared<MyDataCallback>(this);
  mErrorCallback = make_shared<MyErrorCallback>(this);

  AudioStreamBuilder builder;

  builder.setSharingMode(SharingMode::Exclusive);
  builder.setPerformanceMode(PerformanceMode::LowLatency);
  builder.setFormat(AudioFormat::Float);
  builder.setChannelCount(kChannelCount);
  builder.setDataCallback(mDataCallback);
  builder.setErrorCallback(mErrorCallback);
  builder.setSampleRate(44100);
  builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);

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
  file.open(audioPath, ios::binary | ios::ate);
  
  if (!file.good()) {
    __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to open file: %s => %s", audioPath.c_str(), strerror(errno));
    return false;
  }
  
  int size = file.tellg();
  this->totalSamples = size / sizeof(uint16_t);
  __android_log_print(ANDROID_LOG_INFO, TAG, "Size: %d", size);
  file.seekg(0);
  
  unsigned char* buf = new unsigned char[size];
  
  file.read((char*) buf, size);
  file.close();
  
  MemInputStream stream(buf, size);
  WavStreamReader reader(&stream);
  reader.parse();
  
  this->mNumChannels = reader.getNumChannels();
  __android_log_print(ANDROID_LOG_INFO, TAG, "Audio channels: %d", this->mNumChannels);
  
  reader.positionToAudio();

  mSampleRate = reader.getSampleRate();
  mNumSamples = reader.getNumSampleFrames() * reader.getNumChannels();
  
  if (mSampleData != NULL) {
    delete[] mSampleData;
  }
  mSampleData = new float[mNumSamples];
  
  reader.getDataFloat(mSampleData, reader.getNumSampleFrames());
  
  this->resampleData(mStream->getSampleRate());
  
  delete[] buf;
  return true;
}


void FilePlayer::resampleData(int destSampleRate) {
  if (mSampleRate == destSampleRate) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "No need to resemple from %d to %d", mSampleRate, destSampleRate);
    return;
  }
  __android_log_print(ANDROID_LOG_INFO, TAG, "Resampling from %d to %d", mSampleRate, destSampleRate);
  
  double temp = ((double) mNumSamples * (double) destSampleRate) / (double) mSampleRate;
  int32_t numOutFramesAllocated = (int32_t) (temp + 0.5);
  numOutFramesAllocated += 8;

  MultiChannelResampler *resampler = MultiChannelResampler::make(mNumChannels, mSampleRate, destSampleRate, MultiChannelResampler::Quality::Medium);
  
  float* inputBuffer = mSampleData;
  float* outputBuffer = new float[numOutFramesAllocated];
  float* resampledData = outputBuffer;

  int numOutputSamples = 0;
  int inputSamplesLeft = mNumSamples;
  
  while (inputSamplesLeft > 0 && numOutputSamples < numOutFramesAllocated) {
      if (resampler->isWriteNeeded()) {
          resampler->writeNextFrame(inputBuffer);
          inputBuffer += mNumChannels;
          inputSamplesLeft -= mNumChannels;
      }
      else {
          resampler->readNextFrame(outputBuffer);
          outputBuffer += mNumChannels;
          numOutputSamples += mNumChannels;
      }
  }
  
  delete resampler;
  delete[] mSampleData;

  mSampleData = resampledData;
  mNumSamples = numOutputSamples;
  mSampleRate = destSampleRate;
}


DataCallbackResult FilePlayer::MyDataCallback::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
  if (!mParent->isPlaying) {
    memset(audioData, 0, numFrames * kChannelCount * sizeof(float));
    return DataCallbackResult::Continue;
  }
  
  float* stream = (float*) audioData;
  
  for (int i = 0; i < numFrames; i++) {
    float sample = 0;
    
    for (int ch = 0; ch < mParent->mNumChannels; ch++) {
      if (mParent->nextSampleId < mParent->totalSamples) {
        sample = mParent->mSampleData[mParent->nextSampleId++];
        mParent->samplesProcessed++;
      }
      *stream++ = sample;
    }
    
    if (mParent->mNumChannels == 1) {
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
