package org.home.oboeplayer;

import android.content.Context;
import android.os.Bundle;
import android.widget.TextView;
import android.view.MotionEvent;

import androidx.appcompat.app.AppCompatActivity;

import static org.home.oboeplayer.Fun.log;


public class MainActivity extends AppCompatActivity {

  private Context context;
  
  private AudioPlayer audioPlayer;
  
  private boolean isPlaying;
  private String audioPath;

  
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    
    context = this;
    
    init();
    configUI();
  }
  
  @Override
  protected void onStart() {
    super.onStart();
  }
  
  @Override
  protected void onResume() {
    super.onResume();
    if (audioPlayer != null) audioPlayer.setupAudio(audioPath);
  }
  
  @Override
  protected void onPause() {
    super.onPause();
    if (audioPlayer != null) audioPlayer.stop();
  }
  
  private void init() {
    audioPlayer = new AudioPlayer();
    
    // audioPath = "/storage/emulated/0/_temp/ez_snare.wav";
    // audioPath = "/storage/emulated/0/_temp/SnareDrum.wav";
    // audioPath = "/storage/emulated/0/_temp/HiHat_Closed.wav";
    // audioPath = "/storage/emulated/0/_temp/sine_440_hz.wav";
    // audioPath = "/storage/emulated/0/_temp/stereo_sine.wav";
    audioPath = "/storage/emulated/0/_temp/guitarA.wav";
  }
  
  private void configUI() {
    TextView activeTitle = findViewById(R.id.activeTitle);
    activeTitle.setOnTouchListener((view, event) -> {
      if (event.getAction() == MotionEvent.ACTION_DOWN) {
        audioPlayer.play();
      }
      return true;
    });
  }
  
}
