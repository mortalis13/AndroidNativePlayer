package org.home.oboeplayer;

public class AudioPlayer {
  
  private boolean filterEnabled;
  
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
  
  public void toggleFilter() {
    filterEnabled = !filterEnabled;
    if (filterEnabled) enableFilter();
    else disableFilter();
  }
  
  private native int startAudioStreamNative();
  private native int stopAudioStreamNative();
  private native int playAudio(String audioPath);
  private native void enableFilter();
  private native void disableFilter();
  
}
