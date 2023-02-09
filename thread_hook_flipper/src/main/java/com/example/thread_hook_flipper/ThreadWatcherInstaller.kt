package com.example.thread_hook_flipper

import android.content.ContentProvider
import android.content.ContentValues
import android.content.Context
import android.database.Cursor
import android.net.Uri
import android.util.Log
import com.facebook.flipper.BuildConfig
import com.facebook.flipper.android.AndroidFlipperClient
import com.facebook.flipper.android.utils.FlipperUtils
import com.facebook.soloader.SoLoader

/**
 * 自动初始化
 */
class ThreadWatcherInstaller : ContentProvider() {

    companion object {
        var sContext: Context? = null
    }

    override fun onCreate(): Boolean {
        sContext = context
        SoLoader.init(context, false)
        Log.d("TAG", "onCreate: ${BuildConfig.DEBUG}")
        if (FlipperUtils.shouldEnableFlipper(ThreadWatcherInstaller.sContext)) {
            val flipperClient = AndroidFlipperClient.getInstance(context)
            flipperClient.addPlugin(ThreadWatcherPlugin())
            flipperClient.start()
        }
        return true
    }

    override fun query(
        uri: Uri,
        projection: Array<out String>?,
        selection: String?,
        selectionArgs: Array<out String>?,
        sortOrder: String?
    ): Cursor? {
        return null
    }

    override fun getType(uri: Uri): String? {
        return null
    }

    override fun insert(uri: Uri, values: ContentValues?): Uri? {
        return null
    }

    override fun delete(uri: Uri, selection: String?, selectionArgs: Array<out String>?): Int {
        return 0
    }

    override fun update(
        uri: Uri,
        values: ContentValues?,
        selection: String?,
        selectionArgs: Array<out String>?
    ): Int {
        return 0
    }
}