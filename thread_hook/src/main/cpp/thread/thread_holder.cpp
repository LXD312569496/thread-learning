//
// Created by Administrator on 2022/12/29.
//

#include <log.h>
#include <util.h>
#include <xunwind.h>
#include "thread_holder.h"
#include "../thread_hook.h"
#include "callstack.h"

namespace threadhook {
    const char *holder_tag = "thread-holder";

    void
    ThreadHolder::AddThread(int tid, pthread_t pthread, bool isThreadDetached, int64_t start_time,
                            ThreadCreateArg *create_arg, std::string &thread_name) {

        bool valid = threadMap.count(pthread) > 0;
        if (valid) return;

        threadhook::Log::info(holder_tag, "AddThread tid:%d pthread_t:%p", tid, pthread);
        auto &item = threadMap[pthread];
        item.Clear();
        item.thread_internal_id = pthread;
        item.thread_detached = isThreadDetached;
        item.startTime = start_time;
        item.create_time = create_arg->time;
        item.id = tid;
        item.name.assign(thread_name);
        std::string &stack = item.create_call_stack;
        stack.assign("");
        try {
            // native stack
//            int ignoreLines = 0;
//            for (int index = 0; index < threadhook::Constant::kMaxCallStackDepth; ++index) {
//                uintptr_t p = create_arg->pc[index];
//                if (p == 0) continue;
//                 threadhook::Log::info(holder_tag, "unwind native callstack #%d pc%p", index,
//                 p);
//                std::string line = threadhook::CallStack::SymbolizePc(p, index - ignoreLines);
//                if (line.empty()) {
//                    ignoreLines++;
//                } else {
//                    line.append("\n");
//                    stack.append(line);
//                }
//            }
//            std::vector<std::string> splitNatives =
//                    threadhook::Util::Split(test, '\n');

//            Log::info("test", "native_stack:%d", splitNatives.size());
////            Log::info("native_stack", create_arg->native_stack.c_str());
//
//
////            int index=0;
//            for (const auto &split: splitNatives) {
//                if (split.empty()) continue;
////                if (index >= threadhook::Constant::kMaxCallStackDepth) break;
////                index++;
//                std::string line;
//                line.append("#");
//                line.append(split);
//                line.append("\n");
//                stack.append(line);
//                Log::info("native_stack", line.c_str());
//            }
//            if (threadhook::CallStack::canDumpNativeStack) {
                stack.append(create_arg->native_stack);
//            }
            // java stack
            std::vector<std::string> splits =
                    threadhook::Util::Split(create_arg->java_stack.str(), '\n');
            for (const auto &split: splits) {
                if (split.empty()) continue;
                std::string line;
                line.append("#");
                line.append(split);
                line.append("\n");
                stack.append(line);
            }
            //空白堆栈，去掉##
            if (stack.size() == 3) stack.assign("");
        } catch (const std::bad_alloc &) {
            stack.assign("error:bad_alloc");
        }
        delete create_arg;


        ThreadHolder::ReportToFlipper(Util::CurrentTimeNs(), item);
        threadhook::Log::info(holder_tag, "AddThread finish");
    }


    void ThreadHolder::JoinThread(pthread_t threadId) {
        bool valid = threadMap.count(threadId) > 0;
        threadhook::Log::info(holder_tag, "JoinThread tid:%p", threadId);
        if (valid) {
            threadMap[threadId].thread_detached = true;
        } else {
            leakThreadMap.erase(threadId);
        }
    }

    void ThreadHolder::ExitThread(pthread_t threadId, std::string &threadName,
                                  long long int time) {
        bool valid = threadMap.count(threadId) > 0;
        if (!valid) return;
        auto &item = threadMap[threadId];
        threadhook::Log::info(holder_tag, "ExitThread tid:%p name:%s", threadId,
                              item.name.c_str());

        item.exitTime = time;
        item.name.assign(threadName);
        if (!item.thread_detached) {
            // 泄露了
            threadhook::Log::error(holder_tag,
                                   "Exited thread Leak! Not joined or detached!\n tid:%p",
                                   threadId);
            leakThreadMap[threadId] = item;
        }
        threadMap.erase(threadId);
        threadhook::Log::info(holder_tag, "ExitThread finish");
    }

    void ThreadHolder::DetachThread(pthread_t threadId) {
        bool valid = threadMap.count(threadId) > 0;
        threadhook::Log::info(holder_tag, "DetachThread tid:%p", threadId);
        if (valid) {
            threadMap[threadId].thread_detached = true;
        } else {
            leakThreadMap.erase(threadId);
        }
    }

    jstring ThreadHolder::getAllThread() {
        int needReport{};
        const char *type = "detach_leak";
//        auto delay = threadLeakDelay * 1000000LL;  // ms -> ns
        rapidjson::StringBuffer jsonBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);
        writer.StartObject();

        writer.Key("allThreads");
        writer.StartArray();

        for (auto &item: threadMap) {
            WriteThreadJson(writer, item.second);
        }
        writer.EndArray();


        writer.Key("leakThreads");
        writer.StartArray();
        for (auto &item: leakThreadMap) {
            WriteThreadJson(writer, item.second);
        }

        writer.EndArray();

        writer.EndObject();
        threadhook::Log::info(holder_tag, "ReportThreadLeak %d", needReport);

        JNIEnv *env = GetEnv(true);
        jstring string_value = env->NewStringUTF(jsonBuf.GetString());
        return string_value;
    }

    jstring ThreadHolder::getThreadInfo(int tid) {
        rapidjson::StringBuffer jsonBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);

        for (auto &item: threadMap) {
            if (item.second.id == tid) {
                WriteThreadJson(writer, item.second);
            }
        }

        JNIEnv *env = GetEnv(true);
        jstring string_value = env->NewStringUTF(jsonBuf.GetString());
        return string_value;
    }

    void ThreadHolder::ReportThreadLeak(long long time) {
        int needReport{};
        const char *type = "detach_leak";
//        auto delay = threadLeakDelay * 1000000LL;  // ms -> ns
        rapidjson::StringBuffer jsonBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);
        writer.StartObject();

        writer.Key("leakType");
        writer.String(type);

        writer.Key("threads");
        writer.StartArray();

        for (auto &item: threadMap) {
            WriteThreadJson(writer, item.second);
        }


//        for (auto &item : leakThreadMap) {
//            if (item.second.exitTime + delay < time && !item.second.thread_reported) {
//                threadhook::Log::info(holder_tag, "ReportThreadLeak %ld, %ld, %ld",
//                                item.second.exitTime, time, delay);
//                needReport++;
//                item.second.thread_reported = true;
//                WriteThreadJson(writer, item.second);
//            }
//        }
        writer.EndArray();
        writer.EndObject();
        threadhook::Log::info(holder_tag, "ReportThreadLeak %d", needReport);
        if (needReport) {
//            JavaCallback(jsonBuf.GetString());
            // clean up
//            auto it = leakThreadMap.begin();
//            for (; it != leakThreadMap.end();) {
//                if (it->second.thread_reported) {
//                    leakThreadMap.erase(it++);
//                } else {
//                    it++;
//                }
//            }
        }
    }


    //写入单个thread数据
    void ThreadHolder::WriteThreadJson(rapidjson::Writer<rapidjson::StringBuffer> &writer,
                                       ThreadItem &thread_item) {
        writer.StartObject();

        writer.Key("tid");
        writer.Uint(thread_item.id);

        writer.Key("interal_id");
        writer.Uint(thread_item.thread_internal_id);

        writer.Key("createTime");
        writer.Int64(thread_item.create_time);

        writer.Key("startTime");
        writer.Int64(thread_item.startTime);

        writer.Key("endTime");
        writer.Int64(thread_item.exitTime);

        writer.Key("name");
        writer.String(thread_item.name.c_str());

        // 这里先注释掉，确认一下是不是这里的转换有问题，是的话，再处理
        writer.Key("createCallStack");
        auto stack = thread_item.create_call_stack.c_str();
        writer.String(stack);

        writer.EndObject();
    }


    void ThreadHolder::ReportToFlipper(long long time, ThreadItem threadItem) {
        const char *type = "detach_leak";
//        auto delay = threadLeakDelay * 1000000LL;  // ms -> ns
        rapidjson::StringBuffer jsonBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);
//        writer.StartObject();

        WriteThreadJson(writer, threadItem);

//        writer.EndObject();

        threadhook::FlipperCallBack(jsonBuf.GetString());
    }

}