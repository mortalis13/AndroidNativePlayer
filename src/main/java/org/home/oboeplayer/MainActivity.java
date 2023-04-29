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
  }
  
  private void init() {
    audioPlayer = new AudioPlayer();
    
    // String audioPath = "/storage/emulated/0/_temp/ez_snare_raw.wav";
    // String audioPath = "/storage/emulated/0/_temp/SnareDrum_raw.wav";
    // String audioPath = "/storage/emulated/0/_temp/HiHat_Closed_raw.wav";
    // String audioPath = "/storage/emulated/0/_temp/sine_440_hz_raw.wav";
    String audioPath = "/storage/emulated/0/_temp/stereo_sine_raw.wav";
    audioPlayer.setupAudio(audioPath);
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
