//
// Created by Administrator on 2022/12/28.
//
#include <jni.h>
#include "log.h"
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <syscall.h>
#include <asm-generic/fcntl.h>
#include <fcntl.h>
#include "xdl.h"
#include "util.h"
#include "dead_lock.h"

const char *get_lock_owner_symbol_name() {
    if (deadlock::Util::android_api < 29) {
        // android 9.0之前
        return "_ZN3art7Monitor20GetLockOwnerThreadIdEPNS_6mirror6ObjectE";
    } else if (deadlock::Util::android_api <= 30) {
        return "_ZN3art7Monitor20GetLockOwnerThreadIdENS_6ObjPtrINS_6mirror6ObjectEEE";
    } else {
        return "";
    }
}

void *GetContendedMonitor;
void *GetLockOwnerThreadId;


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    deadlock::Log::info("LOG_TAG", "JNI_OnLoad");
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    deadlock::Init(vm, env);

    void *handle = xdl_open("libart.so", RTLD_NOLOAD);
    if (handle != nullptr) {
        GetContendedMonitor = xdl_dsym(handle, "_ZN3art7Monitor19GetContendedMonitorEPNS_6ThreadE",
                                       nullptr);
        if (GetContendedMonitor == nullptr) {
            deadlock::Log::info("test", "GetContendedMonitor is null");
        } else {
            deadlock::Log::info("test", "GetContendedMonitor success");
        }

        GetLockOwnerThreadId = xdl_dsym(handle, get_lock_owner_symbol_name(), nullptr);
        if (GetLockOwnerThreadId == nullptr) {
            deadlock::Log::info("test", "GetLockOwnerThreadId is null");
        } else {
            deadlock::Log::info("test", "GetLockOwnerThreadId success");
        }
        xdl_close(handle);
    }
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_example_thread_1dead_1lock_DeadLockUtils_getCurrentThreadId(JNIEnv *env, jobject thiz,
                                                                     jlong native_peer) {
    int *pInt = reinterpret_cast<int *>(native_peer);
    //地址 +3，得到 native id
    //todo:这里貌似是得+4，可能不同版本有不同的偏移量
    pInt = pInt + 3;
    return *pInt;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_thread_1dead_1lock_DeadLockUtils_getWaitLockThreadId(JNIEnv *env, jobject thiz,
                                                                      jlong native_peer) {
    int *pInt = reinterpret_cast<int *>(native_peer);
    //地址 +3，得到 native id
    //todo:这里貌似是得+4，可能不同版本有不同的偏移量
    pInt = pInt + 3;
    deadlock::Log::info("test", "tid is %d", *pInt);

    int monitorObj = 0;
    int monitor_thread_id = 0;

    if (GetContendedMonitor!= nullptr&&GetLockOwnerThreadId!= nullptr){
        //获取当前线程想要竞争的monitor
        monitorObj = ((int (*)(long)) GetContendedMonitor)(native_peer);
        deadlock::Log::info("test", "GetContendedMonitor is:  %d", monitorObj);

        //这个monitor被哪个线程所持有，返回该线程id
        if (monitorObj != 0) {
            monitor_thread_id = ((int (*)(int)) GetLockOwnerThreadId)(monitorObj);
        }
        deadlock::Log::info("test", "GetLockOwnerThreadId is:  %d", +monitor_thread_id);
    }
    return monitor_thread_id;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_thread_1dead_1lock_DeadLockUtils_setLogEnable(JNIEnv *env, jobject thiz,
                                                               jboolean enable) {
    deadlock::Log::log_enable = enable;
}