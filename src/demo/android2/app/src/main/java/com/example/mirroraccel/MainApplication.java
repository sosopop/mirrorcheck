package com.example.mirroraccel;

import android.app.Application;

import com.baiduren.lemontv.module.MirrorAccel;
import android.util.Log;

public class MainApplication extends Application {
    @Override
    public void onCreate()
    {
        MirrorAccel.init();
        int port = MirrorAccel.create("0.0.0.0:0",
                "{" +
                "\"targets\":[" +
                "{" +
                "\"url\":\"http://42.51.203.25/xszr.mp4?id=1\"" +
                "}," +
                "{" +
                " \"url\":\"http://42.51.203.25/xszr.mp4?id=2\"" +
                "}," +
                "{" +
                " \"url\":\"http://42.51.203.25/xszr.mp4?id=3\"" +
                "}," +
                "{" +
                " \"url\":\"http://42.51.203.25/xszr.mp4?id=4\"" +
                "}," +
                "{" +
                " \"url\":\"http://42.51.203.25/xszr.mp4?id=5\"" +
                "}" +
                "]" +
                "}"
        );
        Log.d("MirrorAccel", "http://127.0.0.1:" + port + "/stream/test.mp4");
        super.onCreate();
    }

    public void onDestroy()
    {
        MirrorAccel.init();
    }
}
