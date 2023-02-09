//
// Created by Administrator on 2022/12/28.
//

#include <log.h>
#include <xhook.h>
#include <util.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include "thread_hooker.h"
#include "item.h"
#include "unistd.h"
#include "../thread_hook.h"
#include <syscall.h>
#include <callstack.h>
#include <xunwind.h>
#include "pthread.h"

namespace threadhook {

    const char *thread_tag = "thread-hook";

    const char *ignore_libs[] = {"koom-thread", "liblog.so", "perfd", "memtrack"};


    void ThreadHooker::Start() {
        InitHook();
    }


    void ThreadHooker::InitHook() {
        threadhook::Log::info(thread_tag, "init hook");
        //使用xhook，hook线程相关的方法
        HookLibs();
    }

    void ThreadHooker::HookLibs() {
        xhook_register("libart.so", "pthread_create", (void *) HookThreadCreate, nullptr);
        xhook_register("libart.so", "pthread_detach", (void *) HookThreadDetach, nullptr);
        xhook_register("libart.so", "pthread_join", (void *) HookThreadJoin, nullptr);
        xhook_register("libart.so", "pthread_exit", (void *) HookThreadExit, nullptr);

        xhook_refresh(1);
    }


    int ThreadHooker::HookThreadCreate(pthread_t *tidp, const pthread_attr_t *attr,
                                       void *(*start_rtn)(void *), void *arg) {
//        if (hookEnabled() && start_rtn != nullptr) {
        auto time = Util::CurrentTimeNs();
        threadhook::Log::info(thread_tag, "HookThreadCreate");
        auto *hook_arg = new StartRtnArg(arg, Util::CurrentTimeNs(), start_rtn);
        auto *thread_create_arg = hook_arg->thread_create_arg;
        void *thread = threadhook::CallStack::GetCurrentThread();
        if (thread != nullptr) {
            threadhook::CallStack::JavaStackTrace(thread,
                                                  hook_arg->thread_create_arg->java_stack);


//            if (threadhook::CallStack::canDumpNativeStack) {
                threadhook::CallStack::NativeStackTrace(thread_create_arg->pc,
                                                        threadhook::Constant::kMaxCallStackDepth,
                                                        thread_create_arg->native_stack);
//            }
        }

        xunwind_cfi_log(getpid(),gettid(), nullptr,"HookThreadCreate",ANDROID_LOG_DEBUG,"");

        //todo:
//        ThreadHolder::ReportToFlipper(Util::CurrentTimeNs(),item);

        thread_create_arg->stack_time = Util::CurrentTimeNs() - time;
        return pthread_create(tidp, attr,
                              reinterpret_cast<void *(*)(void *)>(HookThreadStart),
                              reinterpret_cast<void *>(hook_arg));
//        }
//        return pthread_create(tidp, attr, start_rtn, arg);
    }


    void ThreadHooker::HookThreadStart(void *arg) {
        threadhook::Log::info(thread_tag, "HookThreadStart");
        auto *hookArg = (StartRtnArg *) arg;
        pthread_attr_t attr;
        pthread_t self = pthread_self();
        int state = 0;
        if (pthread_getattr_np(self, &attr) == 0) {
            pthread_attr_getdetachstate(&attr, &state);
        }
        int tid = (int) syscall(SYS_gettid);

//这种方式，获取的线程名字长度不超过16字节，暂时无解
        char thread_name[16]{};
        prctl(PR_GET_NAME, thread_name);
//
        threadhook::Log::info(thread_tag, "HookThreadStart %p, %d, %d，%s", self, tid,
                              hookArg->thread_create_arg->stack_time,thread_name);
//        threadhook::Log::info(thread_tag, "HookThreadStart %p, %d, %d", self, tid,
//                              hookArg->thread_create_arg->stack_time);
        auto info = new HookAddInfo(tid, Util::CurrentTimeNs(), self,
                                    state == PTHREAD_CREATE_DETACHED,
                                    hookArg->thread_create_arg, thread_name);

        //测试栈回溯
        xunwind_cfi_log(getpid(),gettid(), nullptr,"HelloWorld",ANDROID_LOG_DEBUG,"");
        // java stack
        std::vector<std::string> splits =
                threadhook::Util::Split(hookArg->thread_create_arg->java_stack.str(), '\n');
        for (auto &split: splits) {
            if (split.empty()) continue;
            threadhook::Log::info(thread_tag, "HookThreadStart:%s",split.c_str());
        }

        sHookLooper->post(ACTION_ADD_THREAD, info);
        void *(*start_rtn)(void *) = hookArg->start_rtn;
        void *routine_arg = hookArg->arg;
        delete hookArg;
        start_rtn(routine_arg);
    }

    int ThreadHooker::HookThreadDetach(pthread_t t) {
//        if (!hookEnabled()) return pthread_detach(t);

        int c_tid = (int) syscall(SYS_gettid);
        threadhook::Log::info(thread_tag, "HookThreadDetach c_tid:%0x", c_tid);

        auto info = new HookInfo(t, Util::CurrentTimeNs());
        sHookLooper->post(ACTION_DETACH_THREAD, info);
        return pthread_detach(t);
    }


    int ThreadHooker::HookThreadJoin(pthread_t t, void **return_value) {
//        if (!hookEnabled()) return pthread_join(t, return_value);

        int c_tid = (int) syscall(SYS_gettid);
        threadhook::Log::info(thread_tag, "HookThreadJoin c_tid:%0x", c_tid);

        auto info = new HookInfo(t, Util::CurrentTimeNs());
        sHookLooper->post(ACTION_JOIN_THREAD, info);
        return pthread_join(t, return_value);
    }

    void ThreadHooker::HookThreadExit(void *return_value) {
//        if (!hookEnabled()) pthread_exit(return_value);

        threadhook::Log::info(thread_tag, "HookThreadExit");
        int tid = (int) syscall(SYS_gettid);
        char thread_name[16]{};
        prctl(PR_GET_NAME, thread_name);
        auto info =
                new HookExitInfo(pthread_self(), tid, thread_name, Util::CurrentTimeNs());
        sHookLooper->post(ACTION_EXIT_THREAD, info);
        pthread_exit(return_value);
    }


}