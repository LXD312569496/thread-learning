//
// Created by Administrator on 2022/12/28.
//

#include <log.h>
#include "hook_looper.h"
#include "thread_holder.h"

namespace threadhook {
    const char *looper_tag = "thread-hook-looper";

    HookLooper::HookLooper() : looper() { this->holder = new threadhook::ThreadHolder(); }

    HookLooper::~HookLooper() { delete this->holder; }


    void HookLooper::handle(int what, void *data) {
        looper::handle(what, data);
        switch (what) {
            case ACTION_ADD_THREAD: {
                threadhook::Log::info(looper_tag, "AddThread");
                auto info = static_cast<HookAddInfo *>(data);
                holder->AddThread(info->tid, info->pthread, info->is_thread_detached,
                                  info->time, info->create_arg, info->thread_name);
                delete info;
                break;
            }
            case ACTION_JOIN_THREAD: {
                threadhook::Log::info(looper_tag, "JoinThread");
                auto info = static_cast<HookInfo *>(data);
                holder->JoinThread(info->thread_id);
                delete info;
                break;
            }
            case ACTION_DETACH_THREAD: {
                threadhook::Log::info(looper_tag, "DetachThread");
                auto info = static_cast<HookInfo *>(data);
                holder->DetachThread(info->thread_id);
                delete info;
                break;
            }
            case ACTION_EXIT_THREAD: {
                threadhook::Log::info(looper_tag, "ExitThread");
                auto info = static_cast<HookExitInfo *>(data);
                holder->ExitThread(info->thread_id, info->threadName, info->time);
                delete info;
                break;
            }
            case ACTION_REFRESH: {
                threadhook::Log::info(looper_tag, "Refresh");
                auto info = static_cast<SimpleHookInfo *>(data);
                holder->ReportThreadLeak(info->time);
                delete info;
                break;
            }
            default: {
            }
        }
    }

    void HookLooper::post(int what, void *data) { looper::post(what, data); }


    jstring HookLooper::getAllThread() {
        return holder->getAllThread();
    }

    jstring HookLooper::getThreadInfo(int tid) {
        return holder->getThreadInfo(tid);
    }
}