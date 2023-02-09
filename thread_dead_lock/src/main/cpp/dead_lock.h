//
// Created by Administrator on 2022/12/28.
//

#ifndef TKTHREAD_THREAD_HOOK_H
#define TKTHREAD_THREAD_HOOK_H

#include <atomic>

namespace deadlock {

    //JVMTI相关的逻辑
    extern JNIEnv *globalJniEnv;
    extern JavaVM *globalJavaVm;

    extern void Init(JavaVM *vm, JNIEnv *p_env);
    JNIEnv *GetEnv(bool doAttach = true);
}

#endif //TKTHREAD_THREAD_HOOK_H
