﻿# ThreadWatcherPlugin
监控线程创建的 Flipper 插件

![image](https://github.com/LXD312569496/thread-learning/blob/master/pic/demo.png)

# 集成
建议仅在 Debug buildType 下集成该插件，用于调试。

setting.gradle
```
pluginManagement {
    repositories {
        gradlePluginPortal()
        google()
        mavenCentral()
        maven { url 'https://jitpack.io' }
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        maven { url 'https://jitpack.io' }
    }
}
```

app build.gradle。
```
    implementation "com.github.LXD312569496.thread-learning:thread_hook:0.0.3"
    implementation "com.github.LXD312569496.thread-learning:thread_hook_flipper:0.0.3"
```
在 Application 中初始化，调用 ThreadCreateMonitor.start() 开始监控线程的创建过程。
```
class MyApplication: Application() {

    override fun attachBaseContext(base: Context?) {
        ThreadCreateMonitor.start()
        super.attachBaseContext(base)
    }
}
```


# 文章总结
链接：https://juejin.cn/post/7197980894362189884
