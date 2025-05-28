这是一个游戏优化工具箱项目，通过修改Windows上的一些设置或设置进程优先级的方式来提升游戏体验，减少游戏卡顿。

## 开源协议

GPL 3.0

## 程序技术栈

* 开发语言：C++
* 框架：Qt(6.8.3)
* 构建工具链：MSVC(Qt-6.8.3-msvc2022_64)
* 构建工具：CMAKE、Ninja
* 支持平台：Windows
* 安装包构建工具：NSIS

## 程序功能

* 游戏优化（设置特定游戏进程优先级和I/O为高）
* 自动限制反作弊进程（在监测到反作弊进程启动时自动设置进程优先级为低，并将CPU亲和性绑定到最后一个核）
* 电源计划优化（优化电源调度，发挥最佳性能）
* 限制后台活动（在游戏时降低后台活动资源占比）
* 网络延迟优化（禁用Nagle 算法，降低网络延迟）
* 系统调度优化（提升系统对游戏进程的调度优先级）
* 系统服务优化（禁止非必要系统服务）

## 项目结构

```
GameOptimizerPro/
├── build_all.bat # 构建脚本
├── cmake/
│   └── version.h.in # CMake版本文件
├── CMakeLists.txt # CMake配置文件
├── config/ # 程序配置文件
│   └── config.json
├── GameOptimizerPro.rc # 程序资源文件
├── include/ # 程序头文件
│   ├── config/ # 配置实体类
│   │   ├── app_config.h
│   │   ├── optimism_config.h
│   │   ├── power_plan.h
│   │   ├── process_config.h
│   │   ├── process_info.h
│   │   └── system_info.h
│   ├── core/ # 核心代码
│   │   ├── application.h # 应用类（管理配置类和优化器类）
│   │   ├── config_manager.h # 配置管理类
│   │   ├── optimizer.h # 优化器类（管理各类优化操作）
│   │   ├── power_manager.h # 电源计划管理类
│   │   ├── process_manager.h # 进程管理类（使用`IWbemServices::ExecNotificationQueryAsync`异步方法订阅进程的创建和销毁事件）
│   │   ├── registry_manager.h # 注册表管理类（创建、删除注册表项，修改注册表值等）
│   │   ├── service_manager.h # 系统服务管理类
│   ├── log/
│   │   └── logging.h # 日志类
│   ├── ui/
│   │   ├── components/
│   │   │   └── switchbutton.h # 自定义switchbutton组件
│   │   ├── mainwnd.h # 主窗口
│   │   └── tray_app.h # 托盘类
│   └── utils/
│       ├── event_sink.h # WMI EventSink类
│       ├── registry_key.h # 注册表数据结构
│       └── system_utils.h # 工具函数
├── lib/ # 库文件（自定义组件等）
├── LICENSE
├── project_tree.txt
├── resources/ # 资源文件（图片、qrc文件等）
├── src/
│   ├── config/
│   │   ├── app_config.cpp
│   │   ├── optimism_config.cpp
│   │   ├── power_plan.cpp
│   │   ├── process_config.cpp
│   │   ├── process_info.cpp
│   │   └── system_info.cpp
│   ├── core/
│   │   ├── application.cpp
│   │   ├── config_manager.cpp
│   │   ├── optimizer.cpp
│   │   ├── power_manager.cpp
│   │   ├── process_manager.cpp
│   │   ├── registry_manager.cpp
│   │   ├── service_manager.cpp
│   ├── log/
│   │   └── logging.cpp
│   ├── main.cpp
│   ├── ui/
│   │   ├── mainwnd.cpp
│   │   ├── mainwnd.ui
│   │   └── tray_app.cpp
│   └── utils/
│       ├── event_sink.cpp
│       └── system_utils.cpp
└── translations/
    └── GameOptimizerPro_zh_CN.ts
```

## 程序依赖关系

* TrayApp
  * MainWnd
    * Application
      * ConfigManager
      * Optimizer
        * PowerManager
        * ProcessManager
          * EventSink
        * RegistryManager
          * RegistryKey
        * ServiceManager

其中`logging`和`system_utils`在各个文件都有调用