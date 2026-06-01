package org.stlltd.utils;

import java.lang.String;

import android.util.Log;
import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.provider.Settings;
import android.provider.Settings.Secure;

public class QGpsUtils
{
    protected QGpsUtils() {
       //Log.d("QGpsUtils", "QGpsUtils()");
    }

    public static boolean checkGpsEnabled(final Context context) {

        // Android 9 'P' // API level 28+
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {

            LocationManager lm = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
            boolean gps_enabled = false;
            boolean network_enabled = false;

            try {
                gps_enabled = lm.isProviderEnabled(LocationManager.GPS_PROVIDER);
            } catch(Exception e) {
                e.printStackTrace();
                return false;
            }

            try {
                network_enabled = lm.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
            } catch(Exception e) {
                e.printStackTrace();
                return false;
            }

            return gps_enabled;

        } else { // up to API level 27

            final int locationMode;
            try {
                // This constant was deprecated in API level 28.
                locationMode = Settings.Secure.getInt(context.getContentResolver(),
                                                      Settings.Secure.LOCATION_MODE);
            } catch (Settings.SettingNotFoundException e) {
                e.printStackTrace();
                return false;
            }

            switch (locationMode) {
                case Settings.Secure.LOCATION_MODE_HIGH_ACCURACY:
                case Settings.Secure.LOCATION_MODE_SENSORS_ONLY:
                    return true;

                case Settings.Secure.LOCATION_MODE_BATTERY_SAVING:
                case Settings.Secure.LOCATION_MODE_OFF:
                default:
                    return false;
            }
        }
    }

    public static boolean forceGpsEnabled(final Context context) {
        return false;
    }

    public static Intent openLocationSettings() {
        Intent gpsActivationIntent = new Intent(android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        return gpsActivationIntent;
    }
}
