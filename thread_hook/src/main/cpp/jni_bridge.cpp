//
// Created by Administrator on 2022/12/28.
//
#include <jni.h>
#include <log.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <syscall.h>
#include <asm-generic/fcntl.h>
#include <fcntl.h>
#include "thread_hook.h"
#include "callstack.h"
#include "xdl.h"

const char *get_lock_owner_symbol_name() {
    if (threadhook::Util::android_api < 29) {
        // android 9.0之前
        return "_ZN3art7Monitor20GetLockOwnerThreadIdEPNS_6mirror6ObjectE";
    } else if (threadhook::Util::android_api <= 30) {
        return "_ZN3art7Monitor20GetLockOwnerThreadIdENS_6ObjPtrINS_6mirror6ObjectEEE";
    } else {
        return "";
    }
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    threadhook::Log::info("LOG_TAG", "JNI_OnLoad");
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    threadhook::Init(vm, env);
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_thread_1hook_ThreadCreateMonitor_start(JNIEnv *env, jobject thiz) {
    threadhook::Start();
}



extern "C"
JNIEXPORT jint JNICALL
Java_com_example_thread_1hook_ThreadCreateMonitor_getNativeTid(JNIEnv *env, jobject thiz,
                                                               jlong native_peer) {
//    auto *tid = reinterpret_cast<uint32_t *>(native_peer +sizeof(uint32_t)+sizeof(int) +sizeof(uint32_t));
//    threadhook::Log::info("test","tid:%d",tid);

    if (native_peer != 0) {
        //long 强转 int
        int *pInt = reinterpret_cast<int *>(native_peer);
        //地址 +3，得到 native id
        //todo:这里貌似是得+4，可能不同版本有不同的偏移量
        pInt = pInt + 4;

//        threadhook::Log::info("test","tid: %d %s",*pInt,getThreadName(*pInt).c_str());
        return *pInt;
    }
    return 0;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_thread_1hook_ThreadCreateMonitor_getThreadInfo(JNIEnv *env, jobject thiz,
                                                                jint tid) {
    return threadhook::sHookLooper->getThreadInfo(tid);
}