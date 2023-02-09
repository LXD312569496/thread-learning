#include <jni.h>
#include <string>
#include <xhook.h>
#include <android/log.h>
#include <unistd.h>
#include <sstream>
#include "syscall.h"
#include "pthread.h"
#include <sys/prctl.h>
#include <log.h>
#include <util.h>
#include <callstack.h>
#include <hook_looper.h>
#include "dlfcn.h"
#include "xdl.h"
#include "thread_hooker.h"

namespace threadhook {
    int Util::android_api;
    bool Log::log_enable = false; //默认为false
    bool CallStack::canDumpNativeStack = false; //默认为false

    //JVMTI相关的逻辑
    JNIEnv *globalJniEnv = NULL;
    JavaVM *globalJavaVm = NULL;

    jclass flipper_class;
    jmethodID flipper_method;

    std::atomic<bool> isRunning;
    HookLooper *sHookLooper;

    void Init(JavaVM *vm, JNIEnv *env) {
        globalJavaVm = vm;

        auto clazz = env->FindClass("com/example/thread_hook_flipper/NativeHandler");
        if (clazz != nullptr) {
            //不接入flipper插件的话，这个是会为空的
            flipper_class = static_cast<jclass>(env->NewGlobalRef(clazz));
            flipper_method = env->GetStaticMethodID(flipper_class, "nativeReport",
                                                    "(Ljava/lang/String;)V");
        }


        //获取Android API版本
        Util::Init();
        Log::info("LOG_TAG", "init android api:%d", Util::AndroidApi());
        //获取堆栈的函数地址
        threadhook::CallStack::Init();
    }

    void Start() {
        if (isRunning) {
            return;
        }
        //初始化Looper
        delete sHookLooper;
        sHookLooper = new HookLooper();
        //native hook线程相关的方法
        threadhook::ThreadHooker::Start();
        isRunning = true;
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

    void FlipperCallBack(const char *value, bool doAttach = true) {
        JNIEnv *env = GetEnv(doAttach);
        if (env != nullptr && value != nullptr
            && flipper_class != nullptr && flipper_method != nullptr) {
            jstring string_value = env->NewStringUTF(value);
            env->CallStaticVoidMethod(flipper_class, flipper_method, string_value);
        }
    }

}