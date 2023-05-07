package org.home.oboeplayer;

import android.content.Context;
import android.os.Bundle;
import android.widget.TextView;
import android.view.MotionEvent;
import android.view.View;
import android.Manifest;
import android.net.Uri;
import android.os.Environment;
import android.provider.Settings;
import android.content.Intent;
import android.os.Build;
import android.content.pm.PackageManager;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

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
    requestAppPermissions(context);
    
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
    if (audioPlayer != null) audioPlayer.setupAudio();
  }
  
  @Override
  protected void onPause() {
    super.onPause();
    if (audioPlayer != null) audioPlayer.stop();
  }
  
  private void requestAppPermissions(Context context) {
    if (Build.VERSION.SDK_INT < 30) {
      boolean isWriteGranted = ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
      if (isWriteGranted) return;
      
      requestPermissions(new String[] {
        Manifest.permission.WRITE_EXTERNAL_STORAGE
      }, Vars.APP_PERMISSION_REQUEST_ACCESS_EXTERNAL_STORAGE);
    }
    else {
      if (!Environment.isExternalStorageManager()) {
        Uri uri = Uri.parse("package:" + BuildConfig.APPLICATION_ID);
        Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION, uri);
        startActivity(intent);
      }
    }
  }
  
  private void init() {
    audioPlayer = new AudioPlayer();
    
    audioPath = "/storage/emulated/0/_temp/ez_snare.wav";
    // audioPath = "/storage/emulated/0/_temp/SnareDrum.wav";
    // audioPath = "/storage/emulated/0/_temp/HiHat_Closed.wav";
    // audioPath = "/storage/emulated/0/_temp/sine_440_hz.wav";
    // audioPath = "/storage/emulated/0/_temp/stereo_sine.wav";
    // audioPath = "/storage/emulated/0/_temp/guitarA.wav";
    
    // audioPath = "/storage/emulated/0/_temp/CLAP.mp3";
    // audioPath = "/storage/emulated/0/_temp/FUNKY_HOUSE.mp3";
    // audioPath = "/storage/emulated/0/_temp/clap-mono.mp3";
    // audioPath = "/storage/emulated/0/_temp/01. Italian Serenade.mp3";
  }
  
  private void configUI() {
    
  }
  
  public boolean onTouchEvent(MotionEvent event) {
    int action = event.getActionMasked();
    if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN) {
      audioPlayer.play(audioPath);
    }
    return true;
  }
  
}
