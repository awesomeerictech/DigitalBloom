package org.stlltd.digitalbloom;

import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;

public class DigitalBloomReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Intent startServiceIntent = new Intent(context, DigitalBloomService.class);
        context.startService(startServiceIntent);
    }
}
