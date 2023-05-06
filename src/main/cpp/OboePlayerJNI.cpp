#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

#include <android/log.h>

#include "FilePlayer.h"


static const char *TAG = "OboePlayerJNI";


#ifdef __cplusplus
extern "C" {
#endif

using namespace std;


// Use a static object so we don't have to worry about it getting deleted at the wrong time.
static FilePlayer sPlayer;


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_playAudio(JNIEnv *env, jclass obj) {
  sPlayer.play();
  return 0;
}


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_startAudioStreamNative(JNIEnv *env, jclass obj, jstring jaudioPath) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", __func__);
    
    oboe::Result result = sPlayer.open();
    if (result == oboe::Result::OK) {
      const char* audioPathBytes = env->GetStringUTFChars(jaudioPath, 0);
      string audioPath(audioPathBytes);
      env->ReleaseStringUTFChars(jaudioPath, audioPathBytes);
      
      bool fileResult = sPlayer.loadFile(audioPath);
      // bool fileResult = sPlayer.loadFileWav(audioPath);
      
      if (!fileResult) return fileResult;
      result = sPlayer.start();
    }
    
    return (jint) result;
}


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_stopAudioStreamNative(JNIEnv *env, jobject obj) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", __func__);
    oboe::Result result1 = sPlayer.stop();
    oboe::Result result2 = sPlayer.close();
    return (jint) ((result1 != oboe::Result::OK) ? result1 : result2);
}

#ifdef __cplusplus
}
#endif
