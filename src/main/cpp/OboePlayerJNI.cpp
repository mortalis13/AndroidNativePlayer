#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static const char *TAG = "OboePlayerJNI";

#include <android/log.h>

#include "SimpleNoiseMaker.h"

// JNI functions are "C" calling convention
#ifdef __cplusplus
extern "C" {
#endif

using namespace oboe;

// Use a static object so we don't have to worry about it getting deleted at the wrong time.
static SimpleNoiseMaker sPlayer;

/**
 * Native (JNI) implementation of AudioPlayer.startAudiostreamNative()
 */
// JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_startAudioStreamNative(JNIEnv *env, jclass obj, jstring jaudioPath, jobject jassetManager) {
JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_startAudioStreamNative(JNIEnv *env, jclass obj) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", __func__);
    
    Result result = sPlayer.open();
    if (result == Result::OK) {
        result = sPlayer.start();
    }
    
    return (jint) result;
}

/**
 * Native (JNI) implementation of AudioPlayer.stopAudioStreamNative()
 */
JNIEXPORT jint JNICALL Java_org_home_oboeplayer_AudioPlayer_stopAudioStreamNative(JNIEnv * /* env */, jobject) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", __func__);
    
    // We need to close() even if the stop() fails because we need to delete the resources.
    Result result1 = sPlayer.stop();
    Result result2 = sPlayer.close();
    
    // Return first failure code.
    return (jint) ((result1 != Result::OK) ? result1 : result2);
}
#ifdef __cplusplus
}
#endif
