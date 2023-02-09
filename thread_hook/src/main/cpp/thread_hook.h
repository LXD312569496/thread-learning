//
// Created by Administrator on 2022/12/28.
//

#ifndef TKTHREAD_THREAD_HOOK_H
#define TKTHREAD_THREAD_HOOK_H

#include <atomic>
#include <hook_looper.h>

namespace threadhook {

    //JVMTI相关的逻辑
    extern JNIEnv *globalJniEnv;
    extern JavaVM *globalJavaVm;

    extern jclass flipper_class;
    extern jmethodID flipper_method;


    extern HookLooper *sHookLooper;

    extern std::atomic<bool> isRunning;

    extern void Init(JavaVM *vm, JNIEnv *p_env);
    extern void Start();


    JNIEnv *GetEnv(bool doAttach = true);

    void FlipperCallBack(const char *value, bool doAttach = true);

}

#endif //TKTHREAD_THREAD_HOOK_H
