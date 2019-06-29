package com.baiduren.lemontv.module;

public class MirrorAccel {
    static {
        System.loadLibrary("mirroraccel");
    }

    public native static void init();

    public native static void uninit();

    public native static int create(String addr, String option);

    public native static void destroy(int port);
}