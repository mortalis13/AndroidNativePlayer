
#ifndef SIMPLE_NOISE_MAKER_H
#define SIMPLE_NOISE_MAKER_H

#include "oboe/Oboe.h"

/**
 * Play white noise using Oboe.
 */
class SimpleNoiseMaker {
public:

    /**
     * Open an Oboe stream.
     * @return OK or negative error code.
     */
    oboe::Result open();
    oboe::Result start();
    oboe::Result stop();
    oboe::Result close();

private:

    class MyDataCallback : public oboe::AudioStreamDataCallback {
    public:
        oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override;
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

    static constexpr int kChannelCount = 2;
};

#endif //SIMPLE_NOISE_MAKER_H
