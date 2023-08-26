package org.home.nativeplayer;

public class AudioPlayer {
  
  private boolean filterEnabled;
  
  static {
    System.loadLibrary("nativeplayer");
  }
  
  
  public void setupAudio() {
    startAudioStreamNative();
  }
  
  public void play(String audioPath) {
    playAudio(audioPath);
  }
  
  public void pause() {
    pauseAudio();
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
  private native void pauseAudio();
  
  private native void enableFilter();
  private native void disableFilter();
  
  public native void addFrequency(float hz);
  public native void addGain(float db);
  
}
