//
// Created by Administrator on 2022/12/28.
//

#include <dlfcn.h>
#include <xdl.h>
#include "callstack.h"
#include "tls.h"
#include "xunwind.h"

namespace threadhook {

    const char *callstack_tag = "threadhook-callstack";

    //静态变量初始化
    pthread_key_t CallStack::pthread_key_self;
    dump_java_stack_above_o_ptr CallStack::dump_java_stack_above_o;
    dump_java_stack_ptr CallStack::dump_java_stack;

    std::mutex CallStack::dumpJavaLock;


    void CallStack::Init() {
        if (threadhook::Util::AndroidApi() < __ANDROID_API_L__) {
            threadhook::Log::error(callstack_tag, "android api < __ANDROID_API_L__");
            return;
        }
        //通过dlopen寻找到对应的函数地址
        void *handle = xdl_open("libart.so", RTLD_LAZY | RTLD_LOCAL);
        if (threadhook::Util::AndroidApi() >= __ANDROID_API_O__) {
            dump_java_stack_above_o = reinterpret_cast<
                    dump_java_stack_above_o_ptr>(xdl_dsym(
                            handle,
                            "_ZNK3art6Thread13DumpJavaStackERNSt3__113basic_ostreamIcNS1_11char_"
                            "traitsIcEEEEbb", NULL));
            if (dump_java_stack_above_o == nullptr) {
                threadhook::Log::error(callstack_tag, "dump_java_stack_above_o is null");
            }
        } else if (threadhook::Util::AndroidApi() >= __ANDROID_API_L__) {
            dump_java_stack = reinterpret_cast<
                    dump_java_stack_ptr>(xdl_dsym(
                            handle,
                            "_ZNK3art6Thread13DumpJavaStackERNSt3__113basic_ostreamIcNS1_11char_"
                            "traitsIcEEEE", NULL));
            if (dump_java_stack == nullptr) {
                threadhook::Log::error(callstack_tag, "dump_java_stack is null");
            }
        }

        if (threadhook::Util::AndroidApi() < __ANDROID_API_N__) {
            auto *pthread_key_self_art = (pthread_key_t *) xdl_dsym(
                    handle, "_ZN3art6Thread17pthread_key_self_E", NULL);
            if (pthread_key_self_art != nullptr) {
                pthread_key_self = reinterpret_cast<pthread_key_t>(*pthread_key_self_art);
            } else {
                threadhook::Log::error(callstack_tag, "pthread_key_self_art is null");
            }
        }

        xdl_close(handle);
    }

    void *CallStack::GetCurrentThread() {
        if (threadhook::Util::AndroidApi() >= __ANDROID_API_N__) {
            return __get_tls()[TLS_SLOT_ART_THREAD_SELF];
        }
        if (threadhook::Util::AndroidApi() >= __ANDROID_API_L__) {
            return pthread_getspecific(pthread_key_self);
        }
        threadhook::Log::info(callstack_tag, "GetCurrentThread return");
        return nullptr;
    }

    void CallStack::JavaStackTrace(void *thread, std::ostream &os) {
        if (dumpJavaLock.try_lock()) {
            if (threadhook::Util::AndroidApi() >= __ANDROID_API_O__) {
                //不dump locks，有稳定性问题
                dump_java_stack_above_o(thread, os, true, false);
            } else if (threadhook::Util::AndroidApi() >= __ANDROID_API_L__) {
                dump_java_stack(thread, os);
            }
            dumpJavaLock.unlock();
        }
    }

    void CallStack::NativeStackTrace(uintptr_t *buf, size_t num_entries, std::string &nativeStack) {
        char *result = xunwind_cfi_get(getpid(), gettid(), nullptr, "");
//        Log::info("NativeStackTrace", "result:%s", result);

//        std::vector<std::string> splitNatives = threadhook::Util::Split(result, '\n');

//        for (const auto &split: splitNatives) {
//            Log::info("NativeStackTrace", "%s", split.c_str());
//        }
        nativeStack.assign(result);
        delete result;
    }

    size_t CallStack::FastUnwind(uintptr_t *buf, size_t num_entries) {
//        return xunwind_eh_unwind(buf, num_entries, nullptr);
        return xunwind_fp_unwind(buf, num_entries, nullptr);
    }
}