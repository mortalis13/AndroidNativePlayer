package org.home.oboeplayer;

import android.content.Context;
import android.os.Bundle;
import android.widget.Button;
import android.view.MotionEvent;
import android.Manifest;
import android.net.Uri;
import android.os.Environment;
import android.provider.Settings;
import android.content.Intent;
import android.os.Build;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import static org.home.oboeplayer.Fun.log;
import static org.home.oboeplayer.Fun.logd;


public class MainActivity extends AppCompatActivity {

  private AudioPlayer audioPlayer;
  
  private Context context;
  private Button btnTest;
  
  // Screen touch gestures
  private boolean moveStarted;
  private boolean movingX;
  private boolean movingY;
  
  private boolean playing;
  
  private float moveStartX;
  private float moveStartY;
  
  // private String audioPath = "/storage/emulated/0/_tests/ez_snare.wav";
  // private String audioPath = "/storage/emulated/0/_tests/SnareDrum.wav";
  // private String audioPath = "/storage/emulated/0/_tests/HiHat_Closed.wav";
  // private String audioPath = "/storage/emulated/0/_tests/sine_440_hz.wav";
  // private String audioPath = "/storage/emulated/0/_tests/stereo_sine.wav";
  // private String audioPath = "/storage/emulated/0/_tests/guitarA.wav";
  
  // private String audioPath = "/storage/emulated/0/_tests/CLAP.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/FUNKY_HOUSE.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/clap-mono.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/01. Italian Serenade.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/serenade-mono.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/Battle 2.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/01. Sweet Home Alabama.mp3";
  private String audioPath = "/storage/emulated/0/_tests/02. Corporal Jigsore Quandary.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/05. Dismembered.mp3";
  // private String audioPath = "/storage/emulated/0/_tests/09. I Wanna Be Somebody (W.A.S.P. cover).mp3";

  
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
    float x = event.getX();
    float y = event.getY();
    
    if (action == MotionEvent.ACTION_DOWN /*|| action == MotionEvent.ACTION_POINTER_DOWN*/) {
      moveStartX = x;
      moveStartY = y;
      movingX = false;
      movingY = false;
    }
    else if (action == MotionEvent.ACTION_UP) {
      if (!moveStarted) {
        if (!playing) audioPlayer.play(audioPath);
        else audioPlayer.pause();
        playing = !playing;
      }
      moveStarted = false;
    }
    else if (action == MotionEvent.ACTION_MOVE) {
      // Change frequency when moving by X and gain when moving by Y
      // Lock one direction after movement started
      moveStarted = true;
      
      float deltaX = x - moveStartX;
      float deltaY = y - moveStartY;
      
      if (movingX || !movingY && Math.abs(deltaX) >= Math.abs(deltaY)) {
        movingX = true;
        
        float hz = deltaX / 10;
        if (Math.abs((int) hz) > 0) {
          if (Math.abs(hz) > 5) hz *= 10;  // boost
          audioPlayer.addFrequency(hz);
          moveStartX = x;
        }
      }
      else {
        movingY = true;
        
        float db = -deltaY / 10;
        if (Math.abs((int) db) > 0) {
          db /= 10;
          audioPlayer.addGain(db);
          moveStartY = y;
        }
      }
    }
    
    return true;
  }
  
}
