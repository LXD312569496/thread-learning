package com.example.thread_hook_flipper

import android.util.Log
import androidx.annotation.Keep
import com.facebook.flipper.android.AndroidFlipperClient

@Keep
object NativeHandler {

    private val TAG = "NativeHandler"

    @JvmStatic
    fun nativeReport(resultJson: String) {
        Log.d(TAG, "nativeReport,  resultJson:$resultJson")

        if (ThreadWatcherInstaller.sContext != null) {
            val client = AndroidFlipperClient.getInstance(ThreadWatcherInstaller.sContext)
            if (client != null) {
                val plugin = client.getPluginByClass(ThreadWatcherPlugin::class.java)
                plugin?.addThreadInfo(resultJson)
            }
        }
    }
}