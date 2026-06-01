 package com.your.packagename; // REPLACE THIS with your actual package name

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

public class MainActivity extends org.qtproject.qt.android.bindings.QtActivity {
    private static final String TAG = "QtFileSaver";
    private static final int CREATE_FILE_REQUEST_CODE = 42;
    private static Activity instance;
    private static String dataToWrite;
    private static String defaultFileName;

    public MainActivity() {
        instance = this;
    }

    // --- Method to be called from C++ (via JNI) ---
    public static void showSaveFileDialog(String fileName, String data) {
        defaultFileName = fileName;
        dataToWrite = data;
        Log.d(TAG, "Java: showSaveFileDialog called for file: " + fileName);

        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("text/plain"); // Set MIME type (adjust as needed, e.g., "application/pdf")
        intent.putExtra(Intent.EXTRA_TITLE, defaultFileName);

        instance.startActivityForResult(intent, CREATE_FILE_REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == CREATE_FILE_REQUEST_CODE && resultCode == Activity.RESULT_OK) {
            if (data != null) {
                Uri uri = data.getData();
                if (uri != null) {
                    Log.d(TAG, "Java: File URI received: " + uri.toString());
                    // We successfully got a URI, now write the data to it
                    writeContentToUri(uri, dataToWrite);
                }
            }
        } else {
             Log.d(TAG, "Java: File selection cancelled or failed.");
             // You might want to signal back to QML that it failed
        }
    }

    private void writeContentToUri(Uri uri, String content) {
        try {
            OutputStream outputStream = getContentResolver().openOutputStream(uri);
            if (outputStream != null) {
                outputStream.write(content.getBytes(StandardCharsets.UTF_8));
                outputStream.close();
                Log.d(TAG, "Java: Successfully wrote data to URI.");
                // Signal success back to QML if needed
                onFileSaveSuccess(uri.toString());
            }
        } catch (Exception e) {
            Log.e(TAG, "Java: Failed to write data to URI: " + e.getMessage());
             // Signal failure back to QML if needed
             onFileSaveError(e.getMessage());
        }
    }

    // --- JNI methods to call back to C++ (and then QML) ---
    public static native void onFileSaveSuccess(String uriString);
    public static native void onFileSaveError(String errorMessage);
}
