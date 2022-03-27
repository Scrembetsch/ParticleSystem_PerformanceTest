package at.tributsch.particlesystem_performancetest;

import android.content.res.AssetManager;

public class JniBridge {
    static {
        System.loadLibrary("particlesystem_performancetest");
    }

    public static native boolean Init(AssetManager assetManager);
    public static native void Resize(int width, int height);
    public static native void Step();
}
