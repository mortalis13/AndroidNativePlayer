package org.home.oboeplayer;

public class AudioPlayer {
  
  static {
    System.loadLibrary("oboeplayer");
  }
  
  
  public void setupAudio() {
    startAudioStreamNative();
  }
  
  public void play(String audioPath) {
    playAudio(audioPath);
  }
  
  public void stop() {
    stopAudioStreamNative();
  }
  
  private native int startAudioStreamNative();
  private native int stopAudioStreamNative();
  private native int playAudio(String audioPath);
  
}
