package org.home.oboeplayer;

import android.content.Context;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
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
import static org.home.oboeplayer.Fun.logd;


public class MainActivity extends AppCompatActivity {

  private Context context;
  
  private AudioPlayer audioPlayer;
  
  private Button btnTest;
  
  // private String audioPath;
  
  // private String audioPath = "/storage/emulated/0/_temp/ez_snare.wav";
  // private String audioPath = "/storage/emulated/0/_temp/SnareDrum.wav";
  // private String audioPath = "/storage/emulated/0/_temp/HiHat_Closed.wav";
  // private String audioPath = "/storage/emulated/0/_temp/sine_440_hz.wav";
  // private String audioPath = "/storage/emulated/0/_temp/stereo_sine.wav";
  // private String audioPath = "/storage/emulated/0/_temp/guitarA.wav";
  
  // private String audioPath = "/storage/emulated/0/_temp/CLAP.mp3";
  // private String audioPath = "/storage/emulated/0/_temp/FUNKY_HOUSE.mp3";
  // private String audioPath = "/storage/emulated/0/_temp/clap-mono.mp3";
  // private String audioPath = "/storage/emulated/0/_temp/01. Italian Serenade.mp3";
  // private String audioPath = "/storage/emulated/0/_temp/serenade-mono.mp3";
  // private String audioPath = "/storage/emulated/0/_temp/Battle 2.mp3";
  // private String audioPath = "/storage/emulated/0/_temp/01. Sweet Home Alabama.mp3";
  private String audioPath = "/storage/emulated/0/_temp/02. Corporal Jigsore Quandary.mp3";

  
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    logd("MainActivity.onCreate()");
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    
    context = this;
    requestAppPermissions(context);
    
    init();
    configUI();
  }
  
  @Override
  protected void onDestroy() {
    logd("MainActivity.onDestroy()");
    super.onDestroy();
    if (audioPlayer != null) audioPlayer.stop();
  }
  
  protected void onStart() {
    logd("MainActivity.onStart()");
    super.onStart();
  }
  protected void onStop() {
    logd("MainActivity.onStop()");
    super.onStop();
  }
  protected void onRestart() {
    logd("MainActivity.onRestart()");
    super.onRestart();
  }
  protected void onPause() {
    logd("MainActivity.onPause()");
    super.onPause();
  }
  protected void onResume() {
    logd("MainActivity.onResume()");
    super.onResume();
  }
  
  @Override
  public void onBackPressed() {
    finishAndRemoveTask();
    // System.exit(0);
  }

  private void requestAppPermissions(Context context) {
    if (Build.VERSION.SDK_INT < 30) {
      if (ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) == 0) return;
      requestPermissions(new String[] {Manifest.permission.WRITE_EXTERNAL_STORAGE}, Vars.APP_PERMISSION_REQUEST_ACCESS_EXTERNAL_STORAGE);
      return;
    }
    if (Environment.isExternalStorageManager()) return;
    startActivity(new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION, Uri.parse("package:" + BuildConfig.APPLICATION_ID)));
  }
  
  private void init() {
    audioPlayer = new AudioPlayer();
    audioPlayer.setupAudio();
  }
  
  private void configUI() {
    btnTest = findViewById(R.id.btnTest);
    
    btnTest.setOnTouchListener((v, e) -> {
      int action = e.getActionMasked();
      if (action == MotionEvent.ACTION_DOWN) {
        v.setSelected(!v.isSelected());
        audioPlayer.toggleFilter();
      }
      return true;
    });
  }
  
  public boolean onTouchEvent(MotionEvent event) {
    int action = event.getActionMasked();
    if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN) {
      audioPlayer.play(audioPath);
    }
    return true;
  }
  
}
