#define LOG_MODULE_NAME "OboePlayerJNI_"

#include <jni.h>

#include <string>

#include "utils/logging.h"
#include "FilePlayer.h"


#ifdef __cplusplus
extern "C" {
#endif

using namespace std;


static FilePlayer sPlayer;


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_playAudio(JNIEnv *env, jclass obj, jstring jaudioPath) {
  LOGI(__func__);
  
  const char* audioPathBytes = env->GetStringUTFChars(jaudioPath, 0);
  string audioPath(audioPathBytes);
  env->ReleaseStringUTFChars(jaudioPath, audioPathBytes);
  
  bool result = sPlayer.play(audioPath);
  // bool result = sPlayer.playStatic();
  
  if (!result) {
    LOGE("Could not properly load audio file. Check the previous logs.");
    return result;
  }
  
  return result ? 0: -1;
}


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_startAudioStreamNative(JNIEnv *env, jclass obj) {
  LOGI(__func__);
  
  // sPlayer.loadFileStatic("");
  
  bool result = sPlayer.init();
  return result ? 0: -1;
}


JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_stopAudioStreamNative(JNIEnv *env, jobject obj) {
  LOGI(__func__);
  
  oboe::Result result1 = sPlayer.stop();
  oboe::Result result2 = sPlayer.close();
  return (jint) ((result1 != oboe::Result::OK) ? result1 : result2);
}

#ifdef __cplusplus
}
#endif
