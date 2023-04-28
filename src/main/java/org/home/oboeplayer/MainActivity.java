package org.home.oboeplayer;

import android.content.Context;
import android.os.Bundle;
import android.widget.TextView;

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
  }
  
  private void configUI() {
    TextView activeTitle = findViewById(R.id.activeTitle);
    activeTitle.setOnClickListener(v -> {
      log("text clicked");
      
      isPlaying = !isPlaying;
      audioPlayer.setPlaybackEnabled(isPlaying);
    });
  }
  
}
