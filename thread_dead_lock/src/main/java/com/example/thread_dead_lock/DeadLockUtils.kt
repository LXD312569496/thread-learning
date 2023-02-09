package com.example.thread_dead_lock

import androidx.annotation.Keep
import java.lang.reflect.Field
import java.util.*

@Keep
object DeadLockUtils {

    init {
        // Used to load the 'thread_hook' library on application startup.
        System.loadLibrary("dead_lock")
    }

    //获取当前线程的id
    private external fun getCurrentThreadId(nativePeer: Long): Int

    //获取持有锁的线程id
    private external fun getWaitLockThreadId(nativePeer: Long): Int

    //打印日志调试，默认为false，需要手动设置为true
    external fun setLogEnable(enable: Boolean)

    /**
     * 打印死锁相关的数据
     * Triple
     * first:是否死锁
     * second:有向图,key是等锁的线程tid，value是持有锁的线程tid
     * third:线程名称，key是线程tid，value是Java线程名
     */
    @JvmStatic
    fun getThreadDeadLockData(): Triple<Boolean, String, String> {
        //存放信息,key 当前显示，value 等待的锁对应的线程
        val threadLockMap = mutableMapOf<Int, Int>()
        val threadNameMap = mutableMapOf<Int, String>()

        //根据线程状态得到 blocked 的线程，并获取它的内存地址指针 nativePeer
        val allThreads = Thread.getAllStackTraces().keys
        for (thread in allThreads) {
//            println("thread:$thread, state:${thread.state}")
            if (thread!!.state == Thread.State.BLOCKED) {
                val nativePeer = getNativePeer(thread)
                if (nativePeer == 0L) {
                    // 内存地址指针等于0，说明未创建、启动或已销毁
                    continue
                }
                val currentId = getCurrentThreadId(nativePeer)
                //获取目标锁被哪个线程持有
                val waitThreadId = getWaitLockThreadId(nativePeer)
                if (currentId != 0 && waitThreadId != 0) {
                    threadLockMap.put(currentId, waitThreadId)
                    threadNameMap.put(currentId, thread.name)
                }
            }
        }

        println("threadLockMap:$threadLockMap")
        println("threadNameMap:$threadNameMap")

        if (threadLockMap.isEmpty() || threadNameMap.isEmpty()) {
            return Triple(false, threadLockMap.toString(), threadNameMap.toString())
        }

        val isDeadLock = isDeadLock(threadLockMap)
        return Triple(isDeadLock, threadLockMap.toString(), threadNameMap.toString())
    }


    //通过反射获取 Thread 的nativePeer值
    private fun getNativePeer(t: Thread?): Long {
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


    /**
     * https://blog.csdn.net/login_sonata/article/details/78002042
     * 判断有向图是否存在环
     */
    private fun isDeadLock(threadIdMap: Map<Int, Int>): Boolean {
        //线程id列表
        val threadIdList = (threadIdMap.keys + threadIdMap.values).toList().apply {
            println("threadIdList :${this.size}")
        }
        //邻接矩阵
        val graph = Array(threadIdList.size) {
            IntArray(threadIdList.size) {
                0
            }
        }
        //记录每个结点的入度，初始化为0
        val count = IntArray(threadIdList.size) {
            0
        }
        //节点的个数
        val vNum = threadIdList.size
        //边的个数
        val eNum = threadIdMap.size
        //用队列保存拓扑序列
        val queue = LinkedList<Int>()


        //根据threadIdMap，生成邻接矩阵
        threadIdMap.forEach { entry ->
            val keyIndex = threadIdList.indexOf(entry.key)
            val valueIndex = threadIdList.indexOf(entry.value)
            graph.get(keyIndex).set(valueIndex, 1)
        }

        println("graph:")
        graph.forEach {
            println(it.joinToString(","))
        }

        //计算每个结点的入度
        for (i in 0 until vNum) {
            for (j in 0 until vNum) {
                if (graph[i][j] == 1) {
                    count[j] = count[j] + 1
                }
            }
        }
        println("count:")
        println(count.joinToString(","))

        //拓扑排序


        //入度为0的结点的个数，也就是入队个数
        var number = 0
        //暂时存放拓扑序列
        val tempQueue = LinkedList<Int>()
        for (i in 0 until vNum) {
            if (count[i] == 0) {
                queue.offer(i)
            }
        }
        println("queue:${queue.joinToString(",")}")

        //删除这些被删除结点的出边（即对应结点入度减一）
        while (queue.isNotEmpty()) {
            val i = queue.peek() ?: 0
            tempQueue.offer(queue.poll())
            number++
            for (j in 0 until vNum) {
                if (graph[i][j] == 1) {
                    count[j] -= 1
                    //出现了新的入度为0的结点，删除
                    if (count[j] == 0) {
                        queue.offer(j)
                    }
                }
            }
        }

        if (number != vNum) {
            System.out.println("最后存在入度为1的结点，这个有向图是有回路的。");
            return true
        } else {
            System.out.println("这个有向图不存在回路，拓扑序列为：" + tempQueue.toString())
            return false
        }
    }

}