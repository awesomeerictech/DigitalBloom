package com.your.packagename; // REPLACE THIS

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class MainActivity extends org.qtproject.qt.android.bindings.QtActivity {
    private static final String TAG = "QtFileReader";
    private static final int PICK_FILE_REQUEST_CODE = 43; // Use a different request code

    // ... (previous static variables and methods like showSaveFileDialog remain) ...

    // --- Method to be called from C++ (via JNI) ---
    public static void showOpenFilePicker() {
        Log.d(TAG, "Java: showOpenFilePicker called.");
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*"); // Allows any file type (PDF, JSON, CSV, etc.)

        // Use the current instance to start the activity
        instance.startActivityForResult(intent, PICK_FILE_REQUEST_CODE);
    }

    // Update onActivityResult to handle the new request code
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == CREATE_FILE_REQUEST_CODE && resultCode == Activity.RESULT_OK) {
            // ... (Handle file saving logic as before) ...
        } 
        
        else if (requestCode == PICK_FILE_REQUEST_CODE && resultCode == Activity.RESULT_OK) {
            if (data != null) {
                Uri uri = data.getData();
                if (uri != null) {
                    Log.d(TAG, "Java: Read File URI received: " + uri.toString());
                    String fileContent = readContentFromUri(uri);
                    if (fileContent != null) {
                        onFileReadSuccess(uri.toString(), fileContent); // JNI Callback
                    }
                }
            }
        }
    }

    private String readContentFromUri(Uri uri) {
        StringBuilder stringBuilder = new StringBuilder();
        try {
            InputStream inputStream = getContentResolver().openInputStream(uri);
            if (inputStream != null) {
                BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8));
                String line;
                while ((line = reader.readLine()) != null) {
                    stringBuilder.append(line);
                    stringBuilder.append('\n'); // Add newline back if needed
                }
                inputStream.close();
            }
        } catch (Exception e) {
            Log.e(TAG, "Java: Failed to read data from URI: " + e.getMessage());
            onFileReadError(e.getMessage()); // JNI Callback
            return null;
        }
        return stringBuilder.toString();
    }
    
    // --- New JNI methods to call back to C++ (and then QML) ---
    public static native void onFileReadSuccess(String uriString, String content);
    public static native void onFileReadError(String errorMessage);
}
