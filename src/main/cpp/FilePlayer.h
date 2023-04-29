#ifndef FILE_PLAYER_H
#define FILE_PLAYER_H

#include <string>
#include <fstream>

#include "oboe/Oboe.h"

using namespace std;
using namespace oboe;


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

    class MyDataCallback : public AudioStreamDataCallback {
    public:
        MyDataCallback(FilePlayer *parent) : mParent(parent) {
          currentSampleId = 0;
        }
        
        DataCallbackResult onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) override;
    
    private:
        FilePlayer *mParent;
        int currentSampleId;
    };


    class MyErrorCallback : public AudioStreamErrorCallback {
    public:
        MyErrorCallback(FilePlayer *parent) : mParent(parent) {}

        virtual ~MyErrorCallback() {
        }

        void onErrorAfterClose(AudioStream *oboeStream, oboe::Result error) override;

    private:
        FilePlayer *mParent;
    };
    

    shared_ptr<AudioStream> mStream;
    
    shared_ptr<MyDataCallback> mDataCallback;
    shared_ptr<MyErrorCallback> mErrorCallback;
    
    string audioPath;
    bool isPlaying;
    int dataChannels;

    static constexpr int kChannelCount = 2;
};

#endif //FILE_PLAYER_H
