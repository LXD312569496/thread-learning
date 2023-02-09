package com.example.threadlearning

import android.app.Application
import android.content.Context
import com.example.thread_hook.ThreadCreateMonitor

class MyApplication: Application() {

    override fun attachBaseContext(base: Context?) {
        ThreadCreateMonitor.start()
        super.attachBaseContext(base)
    }

}