#define LOG_MODULE_NAME "FilePlayer_"

#include "FilePlayer.h"

#include <stream/MemInputStream.h>
#include <wav/WavStreamReader.h>
#include <resampler/MultiChannelResampler.h>

#include "utils/logging.h"

using namespace parselib;
using namespace RESAMPLER_OUTER_NAMESPACE::resampler;


constexpr int kMaxCompressionRatio { 12 };


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
  
  bool result = loadFile(audioPath);
  if (!result) return result;
  
  this->isPlaying = true;
  this->samplesProcessed = 0;
  this->nextSampleId = 0;
  
  this->decoder->start();
  LOGI("Decoder started");

  return true;
}

bool FilePlayer::loadFile(string audioPath) {
  int result = this->decoder->loadFile(audioPath);
  if (result == -1) return false;
  this->mNumChannels = this->decoder->getDataChannels();
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


DataCallbackResult FilePlayer::MyDataCallback::onAudioReady(AudioStream* audioStream, void* audioData, int32_t numFrames) {
  if (!mParent->isPlaying) {
    memset(audioData, 0, numFrames * kChannelCount * sizeof(float));
    return DataCallbackResult::Continue;
  }
  
  float* stream = (float*) audioData;
  mParent->writeAudio(stream, numFrames);
  // mParent->writeAudioStatic(stream, numFrames);
  
  return DataCallbackResult::Continue;
}


void FilePlayer::MyErrorCallback::onErrorAfterClose(AudioStream* oboeStream, oboe::Result error) {
  LOGE("%s() - error = %s", __func__, oboe::convertToText(error));
  mParent->init();
}


// --------------------------------------------------------------------
bool FilePlayer::playStatic() {
  this->isPlaying = true;
  this->samplesProcessed = 0;
  this->nextSampleId = 0;
  return true;
}

bool FilePlayer::loadFileStatic(string audioPath) {
  ifstream file(audioPath, ios::binary | ios::ate);
  int fileSize = file.tellg();
  file.close();
  
  const long maximumDataSizeInBytes = kMaxCompressionRatio * fileSize * sizeof(float);
  auto decodedData = new uint8_t[maximumDataSizeInBytes];
  
  AudioDecoder decoder(decodedData);
  decoder.setChannelCount(mStream->getChannelCount());
  decoder.setSampleRate(mStream->getSampleRate());
  int64_t bytesDecoded = decoder.decodeStatic(audioPath);
  if (bytesDecoded == -1) {
    return false;
  }
  auto numSamples = bytesDecoded / sizeof(float);
  
  auto outputBuffer = make_unique<float[]>(numSamples);
  memcpy(outputBuffer.get(), decodedData, (size_t) bytesDecoded);
  
  this->mSampleData = std::move(outputBuffer);
  this->totalSamples = numSamples;
  this->mNumChannels = decoder.getDataChannels();
  
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
    LOGI("samplesProcessed: %d, last sample ID: %d", this->samplesProcessed, this->nextSampleId);
  }
}


bool FilePlayer::loadFileQueueStatic(string audioPath) {
  ifstream file(audioPath, ios::binary | ios::ate);
  int fileSize = file.tellg();
  file.close();
  
  const long maximumDataSizeInBytes = kMaxCompressionRatio * fileSize * sizeof(float);
  auto decodedData = new uint8_t[maximumDataSizeInBytes];
  
  AudioDecoder decoder(decodedData);
  decoder.setChannelCount(mStream->getChannelCount());
  decoder.setSampleRate(mStream->getSampleRate());
  int64_t bytesDecoded = decoder.decodeStatic(audioPath);
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
  
  this->mNumChannels = decoder.getDataChannels();
  
  delete[] decodedData;
  return true;
}


bool FilePlayer::loadFileWav(string audioPath) {
  LOGI("Performance mode: %s", oboe::convertToText(mStream->getPerformanceMode()));
  
  ifstream file(audioPath, ios::binary | ios::ate);
  
  if (!file.good()) {
    LOGE("Failed to open file: %s => %s", audioPath.c_str(), strerror(errno));
    return false;
  }
  
  int size = file.tellg();
  this->totalSamples = size / sizeof(uint16_t);
  LOGI("Size: %d", size);
  file.seekg(0);
  
  unsigned char* buf = new unsigned char[size];
  
  file.read((char*) buf, size);
  file.close();
  
  MemInputStream stream(buf, size);
  WavStreamReader reader(&stream);
  reader.parse();
  
  this->mNumChannels = reader.getNumChannels();
  LOGI("Audio channels: %d", this->mNumChannels);
  
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
    LOGI("No need to resemple from %d to %d", mSampleRate, destSampleRate);
    return;
  }
  LOGI("Resampling from %d to %d", mSampleRate, destSampleRate);
  
  double temp = ((double) mNumSamples * (double) destSampleRate) / (double) mSampleRate;
  int32_t numOutFramesAllocated = (int32_t) (temp + 0.5);
  numOutFramesAllocated += 8;

  MultiChannelResampler* resampler = MultiChannelResampler::make(mNumChannels, mSampleRate, destSampleRate, MultiChannelResampler::Quality::Medium);
  
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
    LOGI("samplesProcessed: %d, last sample ID: %d", this->samplesProcessed, this->nextSampleId);
  }
}
