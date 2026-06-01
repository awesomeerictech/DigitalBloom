package org.stlltd.digitalbloom;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * Manifest-registered receiver to start service after device boot.
 * Requires android.permission.RECEIVE_BOOT_COMPLETED in manifest.
 */
public class BootReceiver extends BroadcastReceiver {
    private static final String TAG = "BootReceiver";
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            Log.i(TAG, "BOOT_COMPLETED received - starting DigitalBloomService");
			Intent startServiceIntent = new Intent(context, DigitalBloomService.class);
            context.startService(startServiceIntent);
        }
    }
	
}
