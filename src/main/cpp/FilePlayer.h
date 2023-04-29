#ifndef FILE_PLAYER_H
#define FILE_PLAYER_H

#include <string>
#include <fstream>

#include "oboe/Oboe.h"

using namespace std;


class FilePlayer {
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
        MyDataCallback(FilePlayer *parent) : mParent(parent) {
          currentSampleId = 0;
        }
        
        oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override;
    
    private:
        FilePlayer *mParent;
        int currentSampleId;
    };


    class MyErrorCallback : public oboe::AudioStreamErrorCallback {
    public:
        MyErrorCallback(FilePlayer *parent) : mParent(parent) {}

        virtual ~MyErrorCallback() {
        }

        void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

    private:
        FilePlayer *mParent;
    };
    

    std::shared_ptr<oboe::AudioStream> mStream;
    std::shared_ptr<MyDataCallback> mDataCallback;
    std::shared_ptr<MyErrorCallback> mErrorCallback;
    
    string audioPath;
    bool isPlaying;

    static constexpr int kChannelCount = 1;
};

#endif //FILE_PLAYER_H
