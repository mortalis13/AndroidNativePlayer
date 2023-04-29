package org.home.oboeplayer;

public class AudioPlayer {
  
  static {
    System.loadLibrary("oboeplayer");
  }
  
  
  private String audioPath;

  
  public void setFile(String file) {
    this.audioPath = file;
  }
  
  public void play() {
    playAudio();
  }
  
  public void setPlaybackEnabled(boolean isEnabled) {
    if (isEnabled) {
      startAudioStreamNative(this.audioPath);
    }
    else {
      stopAudioStreamNative();
    }
  }
  
  private native int startAudioStreamNative(String audioPath);
  private native int stopAudioStreamNative();
  private native int playAudio();
  
}
