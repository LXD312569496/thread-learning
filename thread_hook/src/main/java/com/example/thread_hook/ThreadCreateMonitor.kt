package com.example.thread_hook

import androidx.annotation.Keep
import java.lang.reflect.Field


/**
 * 监控线程的创建，pthread_create
 */
@Keep
object ThreadCreateMonitor {

    private const val TAG = "ThreadCreateMonitor"

    init {
        // Used to load the 'thread_hook' library on application startup.
        System.loadLibrary("thread_hook")
    }

    external fun start()


    //通过反射获取 Thread 的nativePeer值
    fun getNativePeer(t: Thread?): Long {
        return try {
            val nativePeerField: Field = Thread::class.java.getDeclaredField("nativePeer")
            nativePeerField.setAccessible(true)
            nativePeerField.get(t) as Long
        } catch (e: NoSuchFieldException) {
            throw IllegalAccessException("failed to get nativePeer value")
        } catch (e: IllegalAccessException) {
            throw e
        }
    }

    fun getNativeTid(t: Thread?): Int {
        val nativePeer = getNativePeer(t)
        return getNativeTid(nativePeer)
    }

    external fun getNativeTid(nativePeer: Long): Int

    external fun getThreadInfo(tid: Int):String
}