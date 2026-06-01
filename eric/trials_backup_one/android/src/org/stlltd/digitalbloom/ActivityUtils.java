package org.stlltd.digitalbloom;

import android.content.Context;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.util.Log;

/**
 * Helper to register BroadcastReceiver dynamically from native code.
 * Receives BOOT_COMPLETED and custom control actions to start/stop/restart the Qt Service.
 */
public class ActivityUtils {
    private static final String TAG = "ActivityUtils";

    public static final String ACTION_START_SERVICE   = "com.digitalbloom.action.START_SERVICE";
    public static final String ACTION_STOP_SERVICE    = "com.digitalbloom.action.STOP_SERVICE";
    public static final String ACTION_RESTART_SERVICE = "com.digitalbloom.action.RESTART_SERVICE";

    /**
     * Register a BroadcastReceiver dynamically using the application context.
     * Native code should call this to register runtime receiver (avoids having to edit manifest).
     */
    public static void registerServiceBroadcastReceiver(final Context ctx) {
        try {
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_BOOT_COMPLETED);
            filter.addAction(Intent.ACTION_SHUTDOWN);
            filter.addAction(Intent.ACTION_PACKAGE_REPLACED);

            // App-specific control actions
            filter.addAction(ACTION_START_SERVICE);
            filter.addAction(ACTION_STOP_SERVICE);
            filter.addAction(ACTION_RESTART_SERVICE);

            BroadcastReceiver receiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    String action = intent.getAction();
					android.content.Intent pQtAndroidService = new android.content.Intent(ctx, DigitalBloomService.class);
                    if (action == null) return;
                    Log.i(TAG, "ServiceBroadcastReceiver onReceive: " + action);

                    switch (action) {
                        case Intent.ACTION_BOOT_COMPLETED:
                        case Intent.ACTION_PACKAGE_REPLACED:
                        case ACTION_START_SERVICE:
                            // Start the Qt-managed foreground service
							ctx.startService(pQtAndroidService);
                            break;

                        case ACTION_STOP_SERVICE:
                        case Intent.ACTION_SHUTDOWN:
                            // Stop the Qt-managed foreground service
                            ctx.stopService(pQtAndroidService);
                            break;

                        case ACTION_RESTART_SERVICE:
                            ctx.stopService(pQtAndroidService);
                            // immediate restart (can be improved with small delayed handler if required)
                            ctx.startService(pQtAndroidService);
                            break;

                        default:
                            // ignore
                            break;
                    }
                }
            };

            // Use application context so receiver remains valid beyond activity lifecycle
            ctx.getApplicationContext().registerReceiver(receiver, filter);
            Log.i(TAG, "Service broadcast receiver registered");
        } catch (Throwable t) {
            Log.e(TAG, "registerServiceBroadcastReceiver failed", t);
        }
    }
}
