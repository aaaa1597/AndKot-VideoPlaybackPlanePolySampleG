#include <jni.h>
#include <string>
#include <android/log.h>
#include <GLES3/gl31.h>

#define LOG_TAG "aaaaa"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//#include <GLES2/gl2ext.h>
#define GL_TEXTURE_EXTERNAL_OES           0x8D65

/* シェーダーのソース */
const char* VERTEX_SHADER =
        "attribute vec4 a_Position;\n"
        "attribute vec2 a_TexCoord;\n"
        "varying vec2 v_TexCoord;\n"
        "void main() {\n"
        "  gl_Position = a_Position;\n"
        "  v_TexCoord = a_TexCoord;\n"
        "}\n";

const char* FRAGMENT_SHADER =
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 v_TexCoord;\n"
        "uniform samplerExternalOES s_Texture;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(s_Texture, v_TexCoord);\n"
        "}\n";

/* OpenGLのハンドル */
GLuint g_program = 0;
GLuint g_textureId = 0;
GLint g_aPositionLoc = -1;
GLint g_aTexCoordLoc = -1;
GLint g_sTextureLoc = -1;

/* 画面と動画のサイズ */
float g_viewWidth = 0.0f;
float g_viewHeight = 0.0f;
float g_videoWidth = 0.0f;
float g_videoHeight = 0.0f;

/* シェーダーをコンパイルするヘルパー関数 */
GLuint loadShader(GLenum type, const char* shaderSrc) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE("glCreateShader failed");
        return 0;
    }
    glShaderSource(shader, 1, &shaderSrc, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            LOGE("Shader compilation error: %s", infoLog);
            delete[] infoLog;
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// シェーダープログラムを作成するヘルパー関数
GLuint createProgram(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        LOGE("glCreateProgram failed");
        return 0;
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            LOGE("Program linking error: %s", infoLog);
            delete[] infoLog;
        }
        glDeleteProgram(program);
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_aaa_videoplaybackplanepolysampleg_JniKt_nativeOnSurfaceCreated(JNIEnv *env, jclass clazz) {
    g_program = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (g_program == 0) {
        LOGE("Failed to create program");
        return -1;
    }

    g_aPositionLoc = glGetAttribLocation(g_program, "a_Position");
    g_aTexCoordLoc = glGetAttribLocation(g_program, "a_TexCoord");
    g_sTextureLoc = glGetUniformLocation(g_program, "s_Texture");

    glGenTextures(1, &g_textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_textureId);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

    return g_textureId;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aaa_videoplaybackplanepolysampleg_JniKt_nativeOnSurfaceChanged(JNIEnv *env, jclass clazz,
                                                                        jint width, jint height) {
    g_viewWidth = width;
    g_viewHeight = height;
    glViewport(0, 0, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aaa_videoplaybackplanepolysampleg_JniKt_nativeSetVideoSize(JNIEnv *env, jclass clazz,
                                                                    jint width, jint height) {
    g_videoWidth = width;
    g_videoHeight = height;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aaa_videoplaybackplanepolysampleg_JniKt_nativeOnDrawFrame(JNIEnv *env, jclass clazz) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (g_program == 0 || g_videoWidth == 0.0f) {
        return;
    }

    glUseProgram(g_program);

    // アスペクト比を考慮した頂点座標の計算
    float viewAspect = g_viewWidth / g_viewHeight;
    float videoAspect = g_videoWidth / g_videoHeight;

    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if (viewAspect > videoAspect) { // Viewが動画より横長の場合
        scaleX = videoAspect / viewAspect;
    } else { // Viewが動画より縦長または同じ比率の場合
        scaleY = viewAspect / videoAspect;
    }

    GLfloat vertices[] = {
        -scaleX, -scaleY, 0.0f, // 左下
        scaleX, -scaleY, 0.0f, // 右下
        -scaleX,  scaleY, 0.0f, // 左上
        scaleX,  scaleY, 0.0f  // 右上
    };

    GLfloat texCoords[] = {
            0.0f, 1.0f, // 左下
            1.0f, 1.0f, // 右下
            0.0f, 0.0f, // 左上
            1.0f, 0.0f  // 右上
    };

    glVertexAttribPointer(g_aPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(g_aTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    glEnableVertexAttribArray(g_aPositionLoc);
    glEnableVertexAttribArray(g_aTexCoordLoc);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_textureId);
    glUniform1i(g_sTextureLoc, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(g_aPositionLoc);
    glDisableVertexAttribArray(g_aTexCoordLoc);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    glUseProgram(0);
}
