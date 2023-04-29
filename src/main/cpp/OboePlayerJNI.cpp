#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static const char *TAG = "OboePlayerJNI";

#include <android/log.h>

#include "FilePlayer.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace oboe;

// Use a static object so we don't have to worry about it getting deleted at the wrong time.
static FilePlayer sPlayer;


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_playAudio(JNIEnv *env, jclass obj) {
  sPlayer.play();
  return 0;
}


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_startAudioStreamNative(JNIEnv *env, jclass obj, jstring jaudioPath) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", __func__);
    
    Result result = sPlayer.open();
    if (result == Result::OK) {
      bool fileResult = sPlayer.setFile("/storage/emulated/0/_temp/ez_snare_raw.wav");
      // bool fileResult = sPlayer.setFile("/storage/emulated/0/_temp/SnareDrum_raw.wav");
      // bool fileResult = sPlayer.setFile("/storage/emulated/0/_temp/HiHat_Closed_raw.wav");
      // bool fileResult = sPlayer.setFile("/storage/emulated/0/_temp/sine_440_hz_raw.wav");
      
      if (!fileResult) return fileResult;
      result = sPlayer.start();
    }
    
    return (jint) result;
}


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_stopAudioStreamNative(JNIEnv *env, jobject obj) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", __func__);
    Result result1 = sPlayer.stop();
    Result result2 = sPlayer.close();
    return (jint) ((result1 != Result::OK) ? result1 : result2);
}

#ifdef __cplusplus
}
#endif
