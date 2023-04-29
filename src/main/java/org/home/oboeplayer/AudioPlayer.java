package org.home.oboeplayer;

public class AudioPlayer {
  
  static {
    System.loadLibrary("oboeplayer");
  }
  
  
  public void setupAudio(String audioPath) {
    startAudioStreamNative(audioPath);
  }
  
  public void play() {
    playAudio();
  }
  
  public void setPlaybackEnabled(boolean isEnabled) {
    if (isEnabled) {
      // startAudioStreamNative(this.audioPath);
    }
    else {
      stopAudioStreamNative();
    }
  }
  
  private native int startAudioStreamNative(String audioPath);
  private native int stopAudioStreamNative();
  private native int playAudio();
  
}