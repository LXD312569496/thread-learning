#include <jni.h>
#include <string>
#include <android/log.h>
#include <unistd.h>
#include <sstream>
#include "syscall.h"
#include "pthread.h"
#include <sys/prctl.h>
#include "dlfcn.h"
#include "xdl.h"
#include "log.h"
#include "util.h"

namespace deadlock {
    int Util::android_api;
    bool Log::log_enable = false; //默认为false

    //JVMTI相关的逻辑
    JNIEnv *globalJniEnv = NULL;
    JavaVM *globalJavaVm = NULL;

    void Init(JavaVM *vm, JNIEnv *env) {
        globalJavaVm = vm;


        //获取Android API版本
        Util::Init();
        Log::info("LOG_TAG", "init android api:%d", Util::AndroidApi());
        //获取堆栈的函数地址
    }


    JNIEnv *GetEnv(bool doAttach) {
        JNIEnv *env = nullptr;
        int status = globalJavaVm->GetEnv((void **) &env, JNI_VERSION_1_6);
        if ((status == JNI_EDETACHED || env == nullptr) && doAttach) {
            status = globalJavaVm->AttachCurrentThread(&env, nullptr);
            if (status < 0) {
                env = nullptr;
            }
        }
        return env;
    }

}