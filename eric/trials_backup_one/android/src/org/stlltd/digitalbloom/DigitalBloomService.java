package org.stlltd.digitalbloom;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.os.Build;
import android.content.Context;
import androidx.annotation.Nullable;

import org.qtproject.qt.android.bindings.QtService;



/**
 * Qt-managed foreground service that bridges to native code via JNI.
 * Extends org.qtproject.qt.android.bindings.QtService (Qt 6).
 */
public class DigitalBloomService extends QtService {
    static {
        // Ensure this matches your native library's target name (libdigitalbloom.so)
        System.loadLibrary("digitalbloom");
    }

    private static final String CHANNEL_ID = "digitalbloom_foreground_channel";
    private static final int NOTIF_ID = 1337;

    /**
     * Native hooks called from Java lifecycle.
     * Implemented in C++ with exact JNI signatures:
     *   Java_com_digitalbloom_service_DigitalBloomService_nativeOnServiceCreate
     *   Java_com_digitalbloom_service_DigitalBloomService_nativeOnServiceDestroy
     */
    private static native void nativeOnServiceCreate();
    private static native void nativeOnServiceDestroy();

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        startForeground(NOTIF_ID, buildNotification("DigitalBloom running"));
        // Notify native side that Service is created (start servers)
        nativeOnServiceCreate();
    }

    @Override
    public int onStartCommand(android.content.Intent intent, int flags, int startId) {
        // Defer to QtService; START_STICKY behavior is managed by QtService implementation
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        // Notify native side to stop servers before Teardown
        nativeOnServiceDestroy();
        super.onDestroy();
    }

    @Nullable
    @Override
    public android.os.IBinder onBind(android.content.Intent intent) {
        // Let QtService handle binding if required
        return super.onBind(intent);
    }

    private Notification buildNotification(String text) {
        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, CHANNEL_ID);
        } else {
            builder = new Notification.Builder(this);
        }
        builder.setContentTitle("DigitalBloom")
               .setContentText(text)
               .setSmallIcon(R.drawable.ic_stat_logo)
               .setOngoing(true);
        return builder.build();
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel chan = new NotificationChannel(
                CHANNEL_ID,
                "DigitalBloom background",
                NotificationManager.IMPORTANCE_LOW
            );
            chan.setDescription("DigitalBloom running");
            NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            if (manager != null) manager.createNotificationChannel(chan);
        }
    }

    public static void serviceStart(android.content.Context context) {
        //Log.i(TAG, ">>>> serviceStart() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

        android.content.Intent pQtAndroidService = new android.content.Intent(context, DigitalBloomService.class);
        pQtAndroidService.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startService(pQtAndroidService);
    }

    public static void serviceStop(android.content.Context context) {
        //Log.i(TAG, ">>>> serviceStop() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

        android.content.Intent pQtAndroidService = new android.content.Intent(context, DigitalBloomService.class);
        context.stopService(pQtAndroidService);
    }
}
