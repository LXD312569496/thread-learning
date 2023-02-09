/*
 * Copyright (c) 2021. Kwai, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Created by shenvsv on 2021.
 *
 */

#ifndef APM_THREAD_HOOK_H
#define APM_THREAD_HOOK_H

#include <android/dlext.h>
#include <jni.h>
#include <pthread.h>
#include <sys/types.h>
#include <set>
#include "iosfwd"

namespace threadhook {

    class ThreadHooker {
    public:
        static void Stop();

        static void Start();

    private:
        static void HookThreadStart(void *arg);

        static int HookThreadCreate(pthread_t *tidp, const pthread_attr_t *attr,
                                    void *(*start_rtn)(void *), void *arg);

        static int HookThreadJoin(pthread_t t, void **return_value);

        static int HookThreadDetach(pthread_t t);

        static void HookThreadExit(void *return_value);

//        static bool RegisterSo(const std::string &lib, int source);

        static void InitHook();

        static void DlopenCallback(std::set<std::string> &libs, int source,
                                   std::string &sourcelib);

        static void HookLibs();

        static bool hookEnabled();
    };

}

#endif  // APM_THREAD_HOOK_H
