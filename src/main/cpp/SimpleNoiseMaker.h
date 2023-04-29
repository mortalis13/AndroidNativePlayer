#ifndef SIMPLE_NOISE_MAKER_H
#define SIMPLE_NOISE_MAKER_H

#include <string>
#include <fstream>

#include "oboe/Oboe.h"

using namespace std;


class SimpleNoiseMaker {
public:

    oboe::Result open();
    oboe::Result start();
    oboe::Result stop();
    oboe::Result close();
    
    bool setFile(string audioPath);
    void play();

    ifstream file;

private:

    class MyDataCallback : public oboe::AudioStreamDataCallback {
    public:
        MyDataCallback(SimpleNoiseMaker *parent) : mParent(parent) {
          currentSampleId = 0;
        }
        
        oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override;
    
    private:
        SimpleNoiseMaker *mParent;
        int currentSampleId;
    };


    class MyErrorCallback : public oboe::AudioStreamErrorCallback {
    public:
        MyErrorCallback(SimpleNoiseMaker *parent) : mParent(parent) {}

        virtual ~MyErrorCallback() {
        }

        void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

    private:
        SimpleNoiseMaker *mParent;
    };
    

    std::shared_ptr<oboe::AudioStream> mStream;
    std::shared_ptr<MyDataCallback> mDataCallback;
    std::shared_ptr<MyErrorCallback> mErrorCallback;
    
    string audioPath;
    bool isPlaying;

    static constexpr int kChannelCount = 1;
};

#endif //SIMPLE_NOISE_MAKER_H
