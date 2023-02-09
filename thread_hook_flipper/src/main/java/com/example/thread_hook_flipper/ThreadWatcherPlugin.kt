package com.example.thread_hook_flipper

import android.util.Log
import com.example.thread_hook.ThreadCreateMonitor
import com.facebook.flipper.core.FlipperConnection
import com.facebook.flipper.core.FlipperObject
import com.facebook.flipper.core.FlipperPlugin
import org.json.JSONObject


//继承BufferingFlipperPlugin，貌似有些问题，暂时不管，先用FlipperPlugin
//class ThreadWatcherPlugin : BufferingFlipperPlugin() {
class ThreadWatcherPlugin : FlipperPlugin {

    private val TAG = "ThreadWatcherPlugin"
    private var mConnection: FlipperConnection? = null

    override fun getId(): String {
        return "threadwatcherplugin"
    }

    override fun onConnect(connection: FlipperConnection?) {
        Log.d(TAG, "onConnect: ")
        mConnection = connection
        getAllThread()
    }

    override fun onDisconnect() {
        Log.d(TAG, "onDisconnect: ")
        mConnection = null
    }

    override fun runInBackground(): Boolean {
        Log.d(TAG, "runInBackground: ")
        return true
    }

    //onConnect之前的线程信息，也要上报一下
    fun getAllThread() {
        println("getAllThread")
        Thread.getAllStackTraces().keys.forEach {
            ThreadCreateMonitor.getThreadInfo(
                ThreadCreateMonitor.getNativeTid(it)
            ).apply {
                if (this.isNotEmpty()) {
                    addThreadInfo(this)
                }
            }
        }
    }

    //上报线程创建信息
    fun addThreadInfo(resultJson: String) {
        val json = JSONObject(resultJson)

        //先用Java线程名，拿不到的话再用native层的线程名兜底,根据tid进行获取
        var threadName = ""
        val tid = json.optInt("tid")
        val thread = Thread.getAllStackTraces().keys.firstOrNull {
            tid == ThreadCreateMonitor.getNativeTid(it)
        }
        Log.d(TAG, "thread: ${thread == null} ")
        if (thread != null) {
            threadName = thread.name
        } else {
            threadName = json.optString("name")
        }

        val priority = thread?.priority?.toString() ?: "None"
        val threadGroup = thread?.threadGroup?.name ?: "None"

        FlipperObject.Builder()
            .put("tid", tid)
            .put("name", threadName)
            .put("threadGroup", threadGroup)
            .put("priority", priority)
            .put("createCallStack", json.opt("createCallStack"))
            .build()
            .apply {
                mConnection?.send("newThread", this)
            }
    }

    //上报线程数量
    fun reportThreadCount() {

    }

}