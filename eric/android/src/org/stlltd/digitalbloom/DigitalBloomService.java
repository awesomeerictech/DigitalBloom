package org.stlltd.digitalbloom;

import java.lang.String;
import android.util.Log;
import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;

import org.qtproject.qt.android.bindings.QtService;

public class DigitalBloomService extends QtService {

    private static final String TAG = "DigitalBloomService";

    @Override
    public void onCreate() {
        super.onCreate();
        //Log.i(TAG, ">>>> Creating Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }

    public void onResume() {
        //Log.i(TAG, ">>>> Resuming Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }

    public void onPause() {
        //Log.i(TAG, ">>>> Pausing Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        //Log.i(TAG, ">>>> Destroying Service >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        return START_STICKY;
    }

    ////////////////////////////////////////////////////////////////////////////

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
