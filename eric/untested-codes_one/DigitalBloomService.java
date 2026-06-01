package org.stlltd.digitalbloom;

import android.app.Service;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Intent;
import android.content.Context;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

public class DigitalBloomService extends Service {
    private static final String TAG = "DigitalBloomService";
    private static final String CHANNEL_ID = "digitalbloom_channel";
    public static final String EXTRA_SERVER_ID = "SERVER_ID";
    public static final String EXTRA_INI_PATH = "INI_PATH";
    public static final String EXTRA_SSL_PATH = "SSL_PATH";

    static {
        // name of your compiled native library without the "lib" prefix or ".so"
        System.loadLibrary("digitalbloom");
    }

    // Native methods implemented in JNI (see AndroidServiceJNI.cpp)
    private static native int nativeStartServer(int serverId, String iniPath, String sslPath);
    private static native int nativeStopServer(int serverId);

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        Log.i(TAG, "Service onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int serverId = 0;
        String iniPath = null;
        String sslPath = null;

        if (intent != null) {
            serverId = intent.getIntExtra(EXTRA_SERVER_ID, 0);
            iniPath = intent.getStringExtra(EXTRA_INI_PATH);
            sslPath = intent.getStringExtra(EXTRA_SSL_PATH);
        }

        Log.i(TAG, "onStartCommand serverId=" + serverId + " ini=" + iniPath + " ssl=" + sslPath);

        // Call native start (non-blocking)
        int rc = nativeStartServer(serverId, iniPath, sslPath);
        Log.i(TAG, "nativeStartServer rc=" + rc);

        // Create a low-priority foreground notification so Android keeps service alive
        Notification.Builder nb;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            nb = new Notification.Builder(this, CHANNEL_ID);
        } else {
            nb = new Notification.Builder(this);
        }
        Notification n = nb
                .setContentTitle("DigitalBloom Server")
                .setContentText("Server " + serverId + " running")
                .setSmallIcon(android.R.drawable.stat_sys_download)
                .build();
        startForeground(1000 + serverId, n);

        // Use START_STICKY so the service is restarted if killed (adjust if undesired)
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy: stopping native servers");
        try {
            nativeStopServer(0);
            nativeStopServer(1);
            nativeStopServer(2);
        } catch (Throwable t) {
            Log.w(TAG, "error stopping native servers: " + t.getMessage());
        }
        stopForeground(true);
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        // not used (not a bound service)
        return null;
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = "DigitalBloom";
            String description = "DigitalBloom background servers";
            int importance = NotificationManager.IMPORTANCE_LOW;
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
            channel.setDescription(description);
            NotificationManager nm = getSystemService(NotificationManager.class);
            if (nm != null) nm.createNotificationChannel(channel);
        }
    }

    // Java helper to start service from other Java/Qt code
    public static void serviceStart(Context ctx, int serverId, String iniPath, String sslPath) {
        Intent intent = new Intent(ctx, DigitalBloomService.class);
        intent.putExtra(EXTRA_SERVER_ID, serverId);
        if (iniPath != null) intent.putExtra(EXTRA_INI_PATH, iniPath);
        if (sslPath != null) intent.putExtra(EXTRA_SSL_PATH, sslPath);
        ctx.startService(intent);
    }

    // Java helper to stop a single server instance
    public static void serviceStop(Context ctx, int serverId) {
        // call native stop first
        nativeStopServer(serverId);
        // then stop the Android Service (if nothing else keeps it running)
        Intent intent = new Intent(ctx, DigitalBloomService.class);
        ctx.stopService(intent);
    }
}
