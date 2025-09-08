# AndKot-VideoPlaybackPlanePolySampleG
- Android Kotlin sample showing video playback on a C++ flat polygon (Gemini version).  
**It's finally done.**

# Sequence

## Init Sequence.
```mermaid
sequenceDiagram
    participant System
    participant MainActivity
    participant GlSurfaceView
    %% ------------------------
    Note over System, GlSurfaceView: init.
    System->>MainActivity: onCreate()
    MainActivity->>GlSurfaceView: init()
    Note over GlSurfaceView: setEGLContextClientVersion(3)<br/>setFormat(PixelFormat.TRANSLUCENT)<br/>setEGLConfigChooser(8,8,8,8,0,0)<br/>setZOrderOnTop(true)
    MainActivity->>GlSurfaceView: setRenderer(callback)
```

## Resume Sequence.
```mermaid
sequenceDiagram
    participant System
    participant MainActivity
    participant ExoPlayer
    %% ------------------------
    Note over System, ExoPlayer: Resume.
    System->>MainActivity: onResume()
    MainActivity->>ExoPlayer: play()
```

## Pause Sequence.
```mermaid
sequenceDiagram
    participant System
    participant MainActivity
    participant GlSurfaceView
    participant ExoPlayer
    %% ------------------------
    Note over System, ExoPlayer: Pause.
    System->>MainActivity: onPause()
    MainActivity->>GlSurfaceView: renderMode = RENDERMODE_WHEN_DIRTY
    MainActivity->>ExoPlayer: pause()
```

## Surface Created Sequence.
```mermaid
sequenceDiagram
    participant System
    participant MainActivity
    participant SurfaceTexture
    participant GlSurfaceView
    participant ExoPlayer
    participant Surface
    participant Jni
    participant native-lib
    participant OpenGL
    %% ------------------------
    Note over System, OpenGL: Surface Created.
    System->>MainActivity: onSurfaceCreated()
    MainActivity->>Jni: texId = nativeOnSurfaceCreated()
    Jni->>native-lib: texId = nativeOnSurfaceCreated()
    Note over native-lib: Initialize OpenGL: ret = texId
    native-lib->>OpenGL: init.
    Note over OpenGL: Initialize various components
    MainActivity->>SurfaceTexture: new(texId)
    MainActivity->>SurfaceTexture: setOnFrameAvailableListener(callback)
    MainActivity->>MainActivity: initExoPlayer(context)
    MainActivity->>ExoPlayer: build()
      ExoPlayer->>Surface: new(surfaceTexture)
      ExoPlayer->>ExoPlayer: setVideoSurface(surface)
      ExoPlayer->>ExoPlayer: setMediaItem(uri: "res/raw/${R.raw.aaa}")
      ExoPlayer->>ExoPlayer: addListener(onVideoSizeChanged callback)
      ExoPlayer->>ExoPlayer: prepare()
```

## Surface Changed Sequence.
```mermaid
sequenceDiagram
    participant System
    participant MainActivity
    participant GlSurfaceView
    participant ExoPlayer
    participant Surface
    participant SurfaceTexture
    participant Jni
    participant native-lib
    participant OpenGL
    %% ------------------------
    Note over System, OpenGL: Surface Changed.
    System->>MainActivity: onSurfaceChanged()
    MainActivity->>Jni: nativeOnSurfaceChanged(w,h)
    Jni->>native-lib: nativeOnSurfaceChanged(w,h)
    native-lib->>OpenGL: glViewport(0, 0, width, height)
```

## Draw Frame Sequence.
```mermaid
sequenceDiagram
    participant System
    participant MainActivity
    participant GlSurfaceView
    participant ExoPlayer
    participant Surface
    participant SurfaceTexture
    participant Jni
    participant native-lib
    participant OpenGL
    %% ------------------------
    Note over System, OpenGL: Draw Frame.
    SurfaceTexture->>MainActivity: onFrameAvailable()
    MainActivity->>GlSurfaceView: requestRender()
    System-->>MainActivity: onDrawFrame()
    MainActivity->>SurfaceTexture: updateTexImage()
    MainActivity->>Jni: nativeOnDrawFrame()
```
