package com.aaa.videoplaybackplanepolysampleg

import android.content.Context
import android.graphics.PixelFormat
import android.graphics.SurfaceTexture
import android.opengl.GLSurfaceView
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.Surface
import androidx.media3.common.MediaItem
import androidx.media3.common.Player
import androidx.media3.common.VideoSize
import androidx.media3.exoplayer.ExoPlayer
import com.aaa.videoplaybackplanepolysampleg.databinding.ActivityMainBinding
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MainActivity : AppCompatActivity() {

    private lateinit var _binding: ActivityMainBinding
    private var surfaceTexture: SurfaceTexture? = null
    private var exoPlayer: ExoPlayer? = null
    private var isFrameAvailable = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        _binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(_binding.root)

        _binding.viwGlsurface.setEGLContextClientVersion(3)
        _binding.viwGlsurface.holder.setFormat(PixelFormat.TRANSLUCENT)
        _binding.viwGlsurface.setEGLConfigChooser(8,8,8,8,0,0)
        _binding.viwGlsurface.setZOrderOnTop(true)
        _binding.viwGlsurface.setRenderer(object : GLSurfaceView.Renderer {
            override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
                val textureId = nativeOnSurfaceCreated()
                if (textureId < 0)
                    throw RuntimeException("Failed to create native texture")
                surfaceTexture = SurfaceTexture(textureId)
                surfaceTexture?.setOnFrameAvailableListener(object : SurfaceTexture.OnFrameAvailableListener {
                    override fun onFrameAvailable(surfaceTexture: SurfaceTexture?) {
                        synchronized(this) {
                            isFrameAvailable = true
                            _binding.viwGlsurface.requestRender()
                        }
                    }
                })
                CoroutineScope(Dispatchers.Main).launch {
                    initExoPlayer(this@MainActivity)
                }
            }
            override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
                nativeOnSurfaceChanged(width, height)
            }
            override fun onDrawFrame(gl: GL10) {
                synchronized(this) {
                    if (isFrameAvailable) {
                        surfaceTexture?.updateTexImage()
                        isFrameAvailable = false
                    }
                }
                nativeOnDrawFrame()
            }
        })

    }

    private fun initExoPlayer(context: Context) {
        exoPlayer = ExoPlayer.Builder(context).build().apply {
            val surface = Surface(surfaceTexture)
            setVideoSurface(surface)

            /* res/raw にある動画ファイルを指定 */
            val uri = "android.resource://${context.packageName}/${R.raw.vuforiasizzlereel}"
            Log.d("aaaaa", "uri=$uri")
            val mediaItem = MediaItem.fromUri(uri)
            setMediaItem(mediaItem)

            repeatMode = Player.REPEAT_MODE_ONE /* ループ再生 */
            playWhenReady = true /* すぐに再生開始 */

            addListener(object : Player.Listener {
                override fun onVideoSizeChanged(videoSize: VideoSize) {
                    /* C++側に動画サイズを伝える */
                    nativeSetVideoSize(videoSize.width, videoSize.height)
                }
            })

            prepare()
        }
    }

    override fun onPause() {
        super.onPause()
        _binding.viwGlsurface.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        exoPlayer?.pause()
    }

    override fun onResume() {
        super.onResume()
        exoPlayer?.play()
    }

    companion object {
        init {
            System.loadLibrary("videoplayer")
        }
    }
}