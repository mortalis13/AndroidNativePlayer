package org.home.oboeplayer

import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.plus
import kotlin.coroutines.CoroutineContext

object AudioPlayer : DefaultLifecycleObserver {

    // Create a coroutine scope which we can launch coroutines from. This way, if our
    // player is ever destroyed (for example, if it was no longer a singleton and had multiple
    // instances) any jobs would also be cancelled.
    private val coroutineScope = CoroutineScope(Dispatchers.Default) + Job()

    private var _playerState = MutableStateFlow<PlayerState>(PlayerState.NoResultYet)
    val playerState = _playerState.asStateFlow()

    init {
        // Load the library containing the native code including the JNI functions.
        System.loadLibrary("oboeplayer")
    }

    fun setPlaybackEnabled(isEnabled: Boolean) {
        // Start (and stop) Oboe from a coroutine in case it blocks for too long.
        // If the AudioServer has died it may take several seconds to recover.
        // That can cause an ANR if we are starting audio from the main UI thread.
        coroutineScope.launch {

            val result = if (isEnabled) {
                // audioPath = ""
                // assetManager = getAssets()
                // startAudioStreamNative(audioPath, assetManager)
                startAudioStreamNative()
            } else {
                stopAudioStreamNative()
            }

            val newUiState = if (result == 0) {
                if (isEnabled){
                    PlayerState.Started
                } else {
                    PlayerState.Stopped
                }
            } else {
                PlayerState.Unknown(result)
            }

            _playerState.update { newUiState }
        }
    }

    override fun onStop(owner: LifecycleOwner) {
        super.onStop(owner)
        setPlaybackEnabled(false)
    }

    // private external fun startAudioStreamNative(audioPath: String, assetManager: AssetManager): Int
    private external fun startAudioStreamNative(): Int
    private external fun stopAudioStreamNative(): Int
}

sealed interface PlayerState {
    object NoResultYet : PlayerState
    object Started : PlayerState
    object Stopped : PlayerState
    data class Unknown(val resultCode: Int) : PlayerState
}
