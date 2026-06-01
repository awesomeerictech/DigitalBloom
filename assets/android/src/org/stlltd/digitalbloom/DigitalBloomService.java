package org.stlltd.digitalbloom;

import java.lang.String;
import android.util.Log;
import android.content.Context;
import android.content.Intent;

import org.qtproject.qt.android.bindings.QtService;

// Import necessary classes for Notifications and Foreground Service
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.os.Build;
import android.graphics.Color;

public class DigitalBloomService extends QtService {

    private static final String TAG = "DigitalBloomService";
    private static final int SERVICE_NOTIFICATION_ID = 103; // Unique ID for the *Foreground Service* Notification
    private static final String CHANNEL_ID_SERVICE = "DigitalBloom_orders"; // Corresponds to channel 1 in DigitalBloomNotifier

    @Override
    public void onCreate() {
        super.onCreate();
        // Log.i(TAG, ">>>> Creating Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        // --- START FOREGROUND SERVICE ---
        startServiceInForeground();
        
        return START_STICKY;
    }

    // New method to create and start the Foreground Service
    private void startServiceInForeground() {
        // 1. Create the Notification Channel for Android O (API 26) and above
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    CHANNEL_ID_SERVICE,
                    "orders_notifications", // Must match channel 3 name in DigitalBloomNotifier
                    NotificationManager.IMPORTANCE_LOW); // IMPORTANCE_LOW is typically used for ongoing background tasks

            channel.enableLights(true);
            channel.setLightColor(Color.BLUE);
            channel.enableVibration(false);
            channel.setLockscreenVisibility(Notification.VISIBILITY_PUBLIC);

            NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            manager.createNotificationChannel(channel);
        }

        // 2. Create the Notification for the Foreground Service status
        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, CHANNEL_ID_SERVICE);
        } else {
            builder = new Notification.Builder(this);
        }

        builder.setSmallIcon(R.drawable.ic_stat_logo) // Use your existing small icon
               .setContentTitle("DigitalBloom Running")
               .setContentText("Your service is active in the background.")
               .setOngoing(true); // This makes the notification unswipeable

        Notification notification = builder.build();

        // 3. Call startForeground with a unique ID and the Notification
        startForeground(SERVICE_NOTIFICATION_ID, notification);

        Log.i(TAG, "Service started in Foreground mode.");
    }

    ////////////////////////////////////////////////////////////////////////////


    public void onResume() {
        //Log.i(TAG, ">>>> Resuming Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }

    
    public void onPause() {
        //Log.i(TAG, ">>>> Pausing Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }

    @Override
    public void onDestroy() {
        // Ensure the foreground status is removed when the service is destroyed
        stopForeground(true); 
        super.onDestroy();
        //Log.i(TAG, ">>>> Destroying Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }
    
    // ... (serviceStart and serviceStop remain the same) ...
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