/*!
 * DigitalBloom Service
 * This class implements the core Android Service to host the C++ servers.
 * It extends org.qtproject.qt.android.bindings.QtService for proper integration 
 * with the Qt Android Framework.
 * All server start/stop logic is delegated to the native C++ ServerManager 
 * via two simplified JNI calls.
 */

package org.stlltd.digitalbloom;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import org.qtproject.qt.android.bindings.QtService;

public class DigitalBloomService extends QtService {

    private static final String TAG = "DigitalBloomService";
    private static final String CHANNEL_ID = "digital_bloom_server_channel";
    private static final int NOTIFICATION_ID = 1001;

    // Load the C++ native library 'digitalbloom'
   /* static {
        try {
            System.loadLibrary("digitalbloom");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Could not load native library 'digitalbloom': " + e.getMessage());
        }
    } */
    
    // Simplified JNI declarations: map directly to ServerManager::startAllServers/stopAllServers
    private native void nativeStartAllServers();
    private native void nativeStopAllServers();
    
    private boolean serversRunning = false;


    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Service onCreate: Setting up Foreground Service...");
        
        // Ensure the service is set up as a Foreground Service immediately upon creation
        startForeground(NOTIFICATION_ID, createNotification());
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId); 
        
        Log.i(TAG, "Service onStartCommand: Attempting to start C++ servers...");
        
        startServers();

        // START_STICKY ensures the system tries to recreate the service if it's killed
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "Service onDestroy: Stopping servers and removing foreground status...");
        
        stopServers();
        
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }
    
    // --- Server Logic Calls (Native) ---

    private void startServers() {
        if (!serversRunning) {
            Log.i(TAG, "Starting all servers: Calling nativeStartAllServers()...");
            // JNI call to C++ ServerManager::startAllServers()
            nativeStartAllServers(); 
            serversRunning = true;
        }
    }

    private void stopServers() {
        if (serversRunning) {
            Log.i(TAG, "Stopping all servers: Calling nativeStopAllServers()...");
            // JNI call to C++ ServerManager::stopAllServers()
            nativeStopAllServers();
            serversRunning = false;
        }
    }


    // --- Foreground Notification Setup ---

    private Notification createNotification() {
        // Notification Channel is required for Android O (API 26) and above
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel serviceChannel = new NotificationChannel(
                    CHANNEL_ID,
                    "Digital Bloom Server Control",
                    NotificationManager.IMPORTANCE_LOW
            );
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(serviceChannel);
            }
        }
        
        // Build the foreground notification
        Notification.Builder builder = new Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("Digital Bloom Running")
                .setContentText("Digital Bloom system active")
                .setSmallIcon(android.R.drawable..ic_stat_logo) // Placeholder icon
                .setCategory(Notification.CATEGORY_SERVICE);
                
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            return builder.build();
        }

        return builder.build();
    }

    // --- Static Service Control Methods (Used by Boot Receiver and C++ Controller) ---

    public static void serviceStart(Context context) {
        Intent serviceIntent = new Intent(context, DigitalBloomService.class);
        serviceIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK); 
        
        // Use startForegroundService for Android O (API 26) and above
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(serviceIntent);
        } else {
            context.startService(serviceIntent);
        }
    }

    public static void serviceStop(Context context) {
        Intent serviceIntent = new Intent(context, DigitalBloomService.class);
        context.stopService(serviceIntent);
    }
}