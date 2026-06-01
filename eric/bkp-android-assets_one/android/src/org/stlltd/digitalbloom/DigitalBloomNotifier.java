package org.stlltd.digitalbloom;

import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.app.TaskStackBuilder;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;

public class DigitalBloomNotifier {

    private static String channelId = "DigitalBloom_app";
    private static String channelName = "DigitalBloom";
    private static int channelColor = Color.WHITE;
    private static int channelImportance = NotificationManager.IMPORTANCE_DEFAULT;

    public static void notify(final Context context, final String title, final String message, final int channel) {

        if (channel == 0) {
            channelId = "DigitalBloom_app";
            channelName = "app notifications";
            channelColor = Color.WHITE;
            channelImportance = NotificationManager.IMPORTANCE_DEFAULT;
        }

        try {
            String packageName = context.getApplicationContext().getPackageName();
            //Context context = getApplicationContext();

            Intent resultIntent = context.getPackageManager().getLaunchIntentForPackage(packageName);
            resultIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);

            PendingIntent resultPendingIntent = PendingIntent.getActivity(context, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

            NotificationManager notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
            Notification.Builder builder;

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                NotificationChannel notificationChannel = new NotificationChannel(channelId, channelName, channelImportance);
                notificationChannel.enableLights(true);
                notificationChannel.setLightColor(channelColor);
                notificationChannel.enableVibration(false);
                //notificationChannel.setVibrationPattern(new long[]{500,500,500,500,500});
                notificationChannel.setLockscreenVisibility(Notification.VISIBILITY_PUBLIC);

                notificationManager.createNotificationChannel(notificationChannel);
                builder = new Notification.Builder(context, notificationChannel.getId());
            } else {
                builder = new Notification.Builder(context);
            }

            builder.setSmallIcon(R.drawable.ic_stat_logo);
            //Bitmap icon = BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_stat_logo);
            //builder.setLargeIcon(icon);
            //builder.setColor(Color.WHITE);
            builder.setContentTitle(title);
            builder.setContentText(message);
            builder.setContentIntent(resultPendingIntent);
            builder.setWhen(System.currentTimeMillis());
            builder.setShowWhen(true);
            builder.setDefaults(Notification.DEFAULT_SOUND);
            builder.setOnlyAlertOnce(true);
            builder.setAutoCancel(true);

            notificationManager.notify(channel, builder.build());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
