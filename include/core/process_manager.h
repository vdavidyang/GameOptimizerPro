/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:34:32
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-26 20:36:34
 * @FilePath: \GameOptimizerPro\include\core\process_manager.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <windows.h>
#include <Wbemidl.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <codecvt> // For std::wstring_convert (C++11/17, deprecated in C++17 but often available)
#include <locale>  // For std::wstring_convert
#include <vector>
#include <sstream>   // For wstring stream
#include <atlbase.h> // For CComPtr, CComModule
#include <atlcom.h>
#include <atomic>
#include <functional> // 包含 <functional> 头文件
#include <comdef.h>   // For _com_error

#pragma comment(lib, "wbemuuid.lib")

#include "log/logging.h"
#include "utils/system_utils.h"
#include "utils/event_sink.h"

// 前向声明 EventSink
class EventSink;

/**
 * @class ProcessManager
 * @brief 进程管理器类，负责监控和限制反作弊进程
 *
 * 管理 WMI 连接，异步监听指定进程的创建和销毁事件。
 * 使用 WMI 的异步事件通知机制，在单独的线程中监听进程事件，
 * 并通过可配置的回调函数通知用户代码。
 * @note 该类使用单例模式实现，确保全局只有一个实例
 */
class ProcessManager
{
public:
  // 定义回调函数类型别名，提高可读性
  using ProcessEventCallback = std::function<void(const std::wstring &, DWORD)>;
  using ErrorCallback = std::function<void(long)>;

  /**
   * @brief 构造函数。初始化 COM 和 WMI 连接。
   * @throw _com_error 如果 COM 或 WMI 初始化失败。
   */
  ProcessManager();

  /**
   * @brief 析构函数。停止监听并清理资源。
   */
  virtual ~ProcessManager();

  // 禁用拷贝构造和赋值
  ProcessManager(const ProcessManager &) = delete;
  ProcessManager &operator=(const ProcessManager &) = delete;

  /**
   * @brief 启动对指定进程列表的异步监听。
   *
   * 会创建一个新的监听线程，在该线程中初始化 COM (MTA), WMI 并注册事件。
   * @param processNames 要监听的进程名称列表 (例如 "notepad.exe")。
   * @return 如果成功启动监听线程则返回 true，否则返回 false。
   */
  bool startListening(const std::vector<std::string> &processNames);

  /**
   * @brief 停止所有 WMI 事件监听。
   *
   * 向监听线程发送停止信号，并等待线程结束。
   * COM/WMI 的反初始化和事件取消将在监听线程内部完成。
   */
  bool stopListening();

  /**
   * @brief 设置进程创建事件的回调函数。
   * @param callback 当进程被创建时调用的函数 (接受 std::wstring 进程名)。
   */
  void setOnProcessCreatedCallback(ProcessEventCallback callback);

  /**
   * @brief 设置进程销毁事件的回调函数。
   * @param callback 当进程被销毁时调用的函数 (接受 std::wstring 进程名)。
   */
  void setOnProcessDestroyedCallback(ProcessEventCallback callback);

  /**
   * @brief 设置 WMI 错误处理的回调函数。
   * @param callback 当发生 WMI 错误时调用的函数 (接受 HRESULT 错误码)。
   */
  void setOnErrorCallback(ErrorCallback callback);

  /**
   * @brief 检查当前是否正在监听。
   * @return bool 如果正在监听，返回 true。
   */
  bool isListening() const;

  /**
   * @brief: 限制反作弊进程
   * @param wstring &processName
   * @param DWORD pid
   * @return bool 是否限制成功
   */
  bool restrictAntiCheatProcess(const std::wstring &processName, DWORD pid);

  /**
   * @brief: 限制反作弊进程 PowerShell 版
   * @param wstring &processName
   * @param DWORD pid
   * @return bool 是否限制成功
   */
  bool restrictAntiCheatProcessPS(const std::wstring &processName, DWORD processId);

private:
  /**
   * @brief 监听线程的主函数。
   * 负责 COM 初始化、WMI 初始化、事件注册和消息循环。
   * @param processNames 要监听的进程名称列表，传入副本或确保生命周期
   */
  void runListenerLoop(std::vector<std::string> processNames);

  /**
   * @brief 在监听线程中初始化 COM (MTA)、WMI 服务连接和 EventSink。
   * @return HRESULT 成功时返回 S_OK，失败时返回错误代码。
   * @note 此方法在监听线程中被调用，确保 COM 在 MTA 模式下初始化。
   *
   * 此方法在监听线程 `runListenerLoop` 中被调用。
   */
  HRESULT initializeListener();

  /**
   * @brief 初始化 COM 库。
   * @return HRESULT 成功时返回 S_OK，失败时返回错误代码。
   * @note 该函数在监听线程中被调用，确保 COM 在 MTA 模式下初始化。
   */
  HRESULT initializeCOM();

  /**
   * @brief 初始化 WMI 连接。
   * @return HRESULT 成功时返回 S_OK，失败时返回错误代码。
   * @note 此方法会创建 IWbemLocator 和 IWbemServices 对象。
   */
  HRESULT initializeWMI();

  /**
   * @brief 创建 EventSink 实例和相应的 Stub Sink。
   * @return HRESULT 成功时返回 S_OK，失败时返回错误代码。
   * @note 此方法会创建一个新的 EventSink 对象，并将其与当前的 ProcessManager 关联。
   */
  HRESULT createEventSink();

  /**
   * @brief 为指定的进程列表注册 WMI 事件通知。
   * @param processNames 要监听的进程名称列表。
   * @return 如果所有事件都成功注册，则返回 true，否则返回 false。
   * @note 此函数应在监听线程中，在 WMI 和 EventSink 初始化成功后调用。
   *
   * 此处的返回值表示尝试注册的操作是否都“成功”发起了。
   * 实际的事件传递是异步的。
   */
  bool registerForEvents(const std::vector<std::string> &processNames);

  /**
   * @brief [内部] 实际执行WMI事件取消和COM反初始化的方法。
   *
   * 该方法应该在监听线程 (runListenerLoop) 内部被调用。
   */
  void cleanupListenerThread();

  /**
   * @brief 处理 WMI API 调用中的错误。
   * @param errorCode 错误代码。
   * @param errorMessage 描述错误上下文的消息。
   */
  void handleError(HRESULT errorCode, const std::wstring &errorMessage);

  /**
   * @brief [内部] 触发进程创建事件回调。
   *
   * 由 EventSink 调用。
   * @param processName 创建的进程名。
   */
  void triggerProcessCreatedCallback(const std::wstring &processName, DWORD processId);

  /**
   * @brief [内部] 触发进程销毁事件回调。
   *
   * 由 EventSink 调用。
   * @param processName 销毁的进程名。
   */
  void triggerProcessDestroyedCallback(const std::wstring &processName, DWORD processId);

  /**
   * @brief [内部] 触发错误处理回调。
   *
   * 由 EventSink 调用。
   * @param errorCode WMI 错误码 HRESULT。
   */
  void triggerErrorCallback(long errorCode);

  // --- WMI 和 COM 对象 ---
  CComPtr<IWbemLocator> m_pLoc = nullptr;             ///< WMI Locator 对象
  CComPtr<IWbemServices> m_pSvc = nullptr;            ///< WMI Services 对象
  CComPtr<IUnsecuredApartment> m_pUnsecApp = nullptr; ///< 用于创建 Stub Sink
  CComPtr<IWbemObjectSink> m_pStubSink = nullptr;     ///< 传递给 WMI 的 Sink Stub
  std::unique_ptr<EventSink> m_pEventSink = nullptr;  ///< 传递给 WMI 的 Event Sink 对象
  std::atomic<bool> m_stopGlobalRequested{false};     ///< 标志是否从外部（如信号处理器）请求了停止

  // --- 线程和同步 ---
  std::thread m_listenerThread;           ///< 后台监听线程
  HANDLE m_stopEvent = nullptr;           ///< 用于通知监听线程停止的事件
  std::atomic<bool> m_isListening{false}; ///< 当前是否正在监听的标志
  std::mutex m_callbackMutex;             ///< 保护回调函数调用的互斥锁
  std::mutex m_setMutex;                  ///< 用于保护共享资源的互斥锁 (例如启动/停止逻辑)
  std::condition_variable m_cv;           ///< 条件变量，用于线程间同步

  // --- 回调函数成员 ---
  ProcessEventCallback m_onProcessCreatedCallback = nullptr;   ///< 进程创建回调
  ProcessEventCallback m_onProcessDestroyedCallback = nullptr; ///< 进程销毁回调
  ErrorCallback m_onErrorCallback = nullptr;                   ///< 错误处理回调

  // 允许 EventSink 访问私有/保护成员 (例如回调函数)
  friend class EventSink;
};
// ATL Module，对于 CComObject 等是必需的
extern CComModule m_Module;
