package at.tributsch.particlesystem_performancetest;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGlView extends GLSurfaceView {
    public OpenGlView(Context context) {
        super(context);
        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(3);
        Renderer.sAssetManager = context.getAssets();
        setRenderer(new Renderer());
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public static AssetManager sAssetManager;

        @Override
        public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
            if(!JniBridge.Init(sAssetManager))
            {
                throw new ExceptionInInitializerError();
            }
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            JniBridge.Step();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            JniBridge.Resize(width, height);
        }
    }
}
