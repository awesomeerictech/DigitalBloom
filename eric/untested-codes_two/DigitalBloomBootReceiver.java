/*!
 * DigitalBloom Boot Broadcast Receiver
 * This BroadcastReceiver is responsible for starting the DigitalBloomService
 * (which hosts the C++ servers) automatically when the device finishes booting.
 */

package org.stlltd.digitalbloom;

import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.util.Log;

public class DigitalBloomBootReceiver extends BroadcastReceiver {

    private static final String TAG = "DigitalBloomBootReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        // Check if the received action is ACTION_BOOT_COMPLETED
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            Log.i(TAG, "Received BOOT_COMPLETED. Starting DigitalBloomService...");

            // Use the static helper method from the DigitalBloomService to start the service
            DigitalBloomService.serviceStart(context);
        } else {
            Log.w(TAG, "Received unexpected intent action: " + intent.getAction());
        }
    }
}