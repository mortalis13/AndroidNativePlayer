#include "FilePlayer.h"

#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <future>

#include <android/log.h>

#include <stream/MemInputStream.h>
#include <wav/WavStreamReader.h>

#include <resampler/MultiChannelResampler.h>

#include "utils/logging.h"

using namespace parselib;
using namespace RESAMPLER_OUTER_NAMESPACE::resampler;


static const char *TAG = "FilePlayer";
constexpr int kMaxCompressionRatio { 12 };


oboe::Result FilePlayer::open() {
  isPlaying = false;
  
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

  oboe::Result result = builder.openStream(mStream);
  if (result != Result::OK) {
    LOGE("Failed to open stream. Error: %s", convertToText(result));
  }
  
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
  
  this->decoder->start();
  LOGI("Decoder started");
}


bool FilePlayer::loadFile(string audioPath) {
  // return loadFileStatic(audioPath);
  
  this->decoder = new AudioDecoder(&dataQ);
  this->decoder->setChannelCount(mStream->getChannelCount());
  this->decoder->setSampleRate(mStream->getSampleRate());
  int result = this->decoder->initForFile(audioPath);
  
  if (result == -1) {
    return false;
  }
  this->mNumChannels = this->decoder->in_channels;

  return true;
}


void FilePlayer::writeAudio(float* stream, int32_t numFrames) {
  for (int i = 0; i < numFrames * kChannelCount; i++) {
    float sample = 0;
    
    bool popped = this->dataQ.pop(sample);
    if (!popped) {
      sample = 0;
    }
    
    *stream++ = sample;
  }
}


DataCallbackResult FilePlayer::MyDataCallback::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
  if (!mParent->isPlaying) {
    memset(audioData, 0, numFrames * kChannelCount * sizeof(float));
    return DataCallbackResult::Continue;
  }
  
  float* stream = (float*) audioData;
  mParent->writeAudio(stream, numFrames);
  // mParent->writeAudioStatic(stream, numFrames);
  
  return DataCallbackResult::Continue;
}


void FilePlayer::MyErrorCallback::onErrorAfterClose(AudioStream *oboeStream, oboe::Result error) {
  __android_log_print(ANDROID_LOG_ERROR, TAG, "%s() - error = %s", __func__, oboe::convertToText(error));
  if (mParent->open() == Result::OK) {
    mParent->start();
  }
}


// -------------

bool FilePlayer::loadFileStatic(string audioPath) {
  inputFile.open(audioPath, ios::binary | ios::ate);
  int fileSize = inputFile.tellg();
  inputFile.close();
  
  const long maximumDataSizeInBytes = kMaxCompressionRatio * fileSize * sizeof(float);
  auto decodedData = new uint8_t[maximumDataSizeInBytes];
  
  AudioDecoder decoder;
  int64_t bytesDecoded = decoder.decode(audioPath, decodedData, mStream->getChannelCount(), mStream->getSampleRate());
  if (bytesDecoded == -1) {
    return false;
  }
  auto numSamples = bytesDecoded / sizeof(float);
  
  auto outputBuffer = make_unique<float[]>(numSamples);
  memcpy(outputBuffer.get(), decodedData, (size_t) bytesDecoded);
  
  this->mSampleData = std::move(outputBuffer);
  this->totalSamples = numSamples;
  this->mNumChannels = decoder.in_channels;
  
  LOGI("bytesDecoded: %d", bytesDecoded);
  LOGI("numSamples: %d", numSamples);
  LOGI("player.mNumChannels: %d", this->mNumChannels);
  
  delete[] decodedData;
  return true;
}

void FilePlayer::writeAudioStatic(float* stream, int32_t numFrames) {
  float* data = this->mSampleData.get();
  
  for (int i = 0; i < numFrames * kChannelCount; i++) {
    float sample = 0;
    
    if (this->nextSampleId < this->totalSamples) {
      sample = data[this->nextSampleId++];
      this->samplesProcessed++;
    }
    
    *stream++ = sample;
  }
  
  if (this->nextSampleId >= this->totalSamples) {
    this->isPlaying = false;
    __android_log_print(ANDROID_LOG_INFO, TAG, "samplesProcessed: %d, last sample ID: %d", this->samplesProcessed, this->nextSampleId);
  }
}


bool FilePlayer::loadFileQueueStatic(string audioPath) {
  inputFile.open(audioPath, ios::binary | ios::ate);
  int fileSize = inputFile.tellg();
  inputFile.close();
  
  const long maximumDataSizeInBytes = kMaxCompressionRatio * fileSize * sizeof(float);
  auto decodedData = new uint8_t[maximumDataSizeInBytes];
  
  AudioDecoder decoder;
  int64_t bytesDecoded = decoder.decode(audioPath, decodedData, mStream->getChannelCount(), mStream->getSampleRate());
  if (bytesDecoded == -1) {
    return false;
  }
  auto numSamples = bytesDecoded / sizeof(float);
  
  auto outputBuffer = make_unique<float[]>(numSamples);
  memcpy(outputBuffer.get(), decodedData, (size_t) bytesDecoded);
  
  for (int i=0; i<numSamples; i++) {
    float sample;
    // memcpy(&sample, decodedData+i*sizeof(float), sizeof(float));
    sample = outputBuffer.get()[i];
    dataQ.push(sample);
  }
  
  this->mNumChannels = decoder.in_channels;

  return true;
}


bool FilePlayer::loadFileWav(string audioPath) {
  LOGI("Performance mode: %s", oboe::convertToText(mStream->getPerformanceMode()));
  
  inputFile.open(audioPath, ios::binary | ios::ate);
  
  if (!inputFile.good()) {
    __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to open file: %s => %s", audioPath.c_str(), strerror(errno));
    return false;
  }
  
  int size = inputFile.tellg();
  this->totalSamples = size / sizeof(uint16_t);
  __android_log_print(ANDROID_LOG_INFO, TAG, "Size: %d", size);
  inputFile.seekg(0);
  
  unsigned char* buf = new unsigned char[size];
  
  inputFile.read((char*) buf, size);
  inputFile.close();
  
  MemInputStream stream(buf, size);
  WavStreamReader reader(&stream);
  reader.parse();
  
  this->mNumChannels = reader.getNumChannels();
  __android_log_print(ANDROID_LOG_INFO, TAG, "Audio channels: %d", this->mNumChannels);
  
  reader.positionToAudio();

  mSampleRate = reader.getSampleRate();
  mNumSamples = reader.getNumSampleFrames() * reader.getNumChannels();
  
  mSampleData = make_unique<float[]>(mNumSamples);
  reader.getDataFloat(mSampleData.get(), reader.getNumSampleFrames());
  
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
  
  float* inputBuffer = mSampleData.get();
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

  mSampleData.reset(resampledData);
  mNumSamples = numOutputSamples;
  mSampleRate = destSampleRate;
}

void FilePlayer::writeAudioWav(float* stream, int32_t numFrames) {
  float* data = this->mSampleData.get();
  
  for (int i = 0; i < numFrames; i++) {
    float sample = 0;
    
    for (int ch = 0; ch < this->mNumChannels; ch++) {
      sample = 0;
      
      if (this->nextSampleId < this->totalSamples) {
        sample = data[this->nextSampleId++];
        this->samplesProcessed++;
      }
      
      *stream++ = sample;
    }
    
    if (this->mNumChannels == 1) {
      *stream++ = sample;
    }
  }
  
  if (this->nextSampleId >= this->totalSamples) {
    this->isPlaying = false;
    __android_log_print(ANDROID_LOG_INFO, TAG, "samplesProcessed: %d, last sample ID: %d", this->samplesProcessed, this->nextSampleId);
  }
}
