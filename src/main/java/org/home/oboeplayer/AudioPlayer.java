package org.home.oboeplayer;

public class AudioPlayer {
  
  static {
    System.loadLibrary("oboeplayer");
  }
  
  public void setPlaybackEnabled(boolean isEnabled) {
    if (isEnabled) {
      startAudioStreamNative();
    }
    else {
      stopAudioStreamNative();
    }
  }
  
  // private native int startAudioStreamNative(String audioPath, AssetManager assetManager);
  private native int startAudioStreamNative();
  private native int stopAudioStreamNative();
  
}
