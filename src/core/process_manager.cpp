/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:34:24
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-27 17:49:15
 * @FilePath: \GameOptimizerPro\src\core\process_manager.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "core/process_manager.h"

ProcessManager::ProcessManager()
    : m_pLoc(nullptr),
      m_pSvc(nullptr),
      m_pUnsecApp(nullptr),
      m_pStubSink(nullptr),
      m_pEventSink(nullptr),
      m_isListening(false),
      m_stopGlobalRequested(false)
{
  // Manual-reset, initially non-signaled
  m_stopEvent = CreateEventW(
      NULL,  // Default security attributes
      TRUE,  // Manual-reset event: must be reset manually by ResetEvent
      FALSE, // Initial state is nonsignaled
      NULL   // Unnamed event object
  );
  if (m_stopEvent == nullptr)
  {
    // 这里记录一个严重的错误或抛出异常
    LOG_ERROR("Failed to create stop event. Error: " + std::to_string(GetLastError()));
    // 抛出一个自定义异常或设置一个错误状态
    throw std::runtime_error("Failed to create stop event for ProcessManager.");
  }
  // LOG_INFO("ProcessManager instance created. Stop event initialized.");
}

ProcessManager::~ProcessManager()
{
  // LOG_INFO("ProcessManager destructor called.");

  // 1. Stop listening operations and cancel WMI calls.
  // This will signal the thread, join it, and call CancelAsyncCall.
  // 确保在析构时如果还在监听则停止
  if (m_isListening.load())
  {
    // LOG_INFO("ProcessManager destructor: Listener was active, stopping it now.");
    // 这将等待线程结束并执行清理
    stopListening();
  }

  // 2. Close native handles like the stop event.
  if (m_stopEvent)
  {
    CloseHandle(m_stopEvent);
    m_stopEvent = nullptr;
    // LOG_INFO("Stop event handle closed in destructor.");
  }

  // CComPtr members (m_pLoc, m_pSvc, m_pUnsecApp, m_pStubSink) are automatically released.
  // m_pEventSink is manually released in cleanupListenerThread.
  // LOG_INFO("ProcessManager destructor finished.");
}

HRESULT ProcessManager::initializeCOM()
{
  // 以多线程单元（MTA）模式初始化COM
  // MTA 允许来自其他线程的调用，这对于WMI回调是必要的。
  // Qt通常初始化为STA，所以我们需要为这个线程显式设置为MTA。
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(hr))
  {
    if (hr == RPC_E_CHANGED_MODE)
    {
      // 如果 COM 已经被初始化为不同的模式 (例如 STA by Qt),
      // 在这个线程中以 MTA 再次初始化通常是可以的，COM 会为这个线程维护正确的状态。
      // 但如果严格要求整个进程只有一个 COM 初始化模式，则需要更复杂的处理。
      // 对于 WMI 异步回调，通常建议回调接口所在的线程是 MTA。
      LOG_HRESULT(L"COM already initialized with a different concurrency model. This thread will use MTA.", hr);
    }
    else
    {
      LOG_HRESULT(L"CoInitializeEx failed in initializeCOM", hr);
      handleError(hr, L"CoInitializeEx failed in initializeCOM");
      return hr;
    }
  }
  else if (hr == S_FALSE)
  {
    // S_FALSE 表示 COM 已经在此线程上初始化，这也是可接受的。
    std::cout << "COM library was already initialized on this thread (MTA)." << std::endl;
    LOG_INFO("COM library was already initialized on this thread (MTA).");
  }

  // 为WMI设置通用COM安全性
  // 这允许WMI回调在没有模拟客户端安全级别的情况下进行。
  // 对于异步回调，这是推荐的做法。
  hr = CoInitializeSecurity(
      nullptr,
      -1,                          // COM negotiates authentication service
      nullptr,                     // Authentication services
      nullptr,                     // ServerPrincipalName
      RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
      RPC_C_IMP_LEVEL_IMPERSONATE, // Default impersonation
      nullptr,                     // Authentication info
      EOAC_NONE,                   // Additional capabilities
      nullptr                      // Reserved
  );

  // RPC_E_TOO_LATE 表示安全已被设置，可以忽略
  if (FAILED(hr) && hr != RPC_E_TOO_LATE)
  {
    LOG_HRESULT(L"CoInitializeSecurity failed in initializeCOM", hr);
    handleError(hr, L"CoInitializeSecurity failed in initializeCOM");
    // 即使 CoInitializeSecurity 失败 (除了 RPC_E_TOO_LATE)，
    // CoUninitialize 仍应在线程退出时调用（如果 CoInitializeEx 成功）。
    // 所以这里不立即 CoUninitialize。
    return hr;
  }
  if (hr == RPC_E_TOO_LATE)
  {
    // LOG_INFO("CoInitializeSecurity has already been called (RPC_E_TOO_LATE). Continuing.");
  }

  // 表示COM和安全性设置成功或已存在
  return S_OK;
}

HRESULT ProcessManager::initializeWMI()
{
  HRESULT hr;

  // 1. 创建 IWbemLocator 实例 / Create IWbemLocator instance
  hr = CoCreateInstance(
      CLSID_WbemLocator,
      nullptr,
      CLSCTX_INPROC_SERVER,
      IID_IWbemLocator,
      (LPVOID *)&m_pLoc);
  if (FAILED(hr))
  {
    LOG_HRESULT(L"Failed to create IWbemLocator object", hr);
    handleError(hr, L"Failed to create IWbemLocator object");
    return hr;
  }

  // 2. 连接到 WMI 的 ROOT\\CIMV2 命名空间 / Connect to WMI namespace ROOT\\CIMV2
  // IWbemLocator::ConnectServer 方法用于连接到指定的 WMI 命名空间。
  // The IWbemLocator::ConnectServer method is used to connect to the specified WMI namespace.
  // 使用 BSTR 进行字符串转换 / Use BSTR for string conversion
  _bstr_t bstrNamespace = L"ROOT\\CIMV2";
  hr = m_pLoc->ConnectServer(
      bstrNamespace, // WMI namespace
      nullptr,       // User name (null for current user)
      nullptr,       // User password (null for current user)
      nullptr,       // Locale (null for current locale)
      0,             // Security flags
      nullptr,       // Authority (null for current domain)
      nullptr,       // IWbemContext
      &m_pSvc        // Receives IWbemServices proxy
  );
  if (FAILED(hr))
  {
    LOG_HRESULT(L"Could not connect to WMI namespace ROOT\\CIMV2", hr);
    handleError(hr, L"Could not connect to WMI namespace ROOT\\CIMV2");
    // 清理已创建的 locator
    m_pLoc.Release();
    return hr;
  }
  LOG_INFO(L"Connected to ROOT\\CIMV2 WMI namespace.");

  // 3. 设置 IWbemServices 代理的安全级别
  // 这允许 WMI 服务回调此进程。
  hr = CoSetProxyBlanket(
      m_pSvc,                      // IWbemServices proxy
      RPC_C_AUTHN_WINNT,           // Authentication service
      RPC_C_AUTHZ_NONE,            // Authorization service
      nullptr,                     // Server principal name
      RPC_C_AUTHN_LEVEL_CALL,      // Authentication level
      RPC_C_IMP_LEVEL_IMPERSONATE, // Impersonation level
      nullptr,                     // Client identity
      EOAC_NONE                    // Capability flags
  );
  if (FAILED(hr))
  {
    LOG_HRESULT(L"Could not set proxy blanket", hr);
    handleError(hr, L"Could not set proxy blanket");
    // 清理 service proxy
    m_pSvc.Release();
    // 清理 locator
    m_pLoc.Release();
    return hr;
  }
  // LOG_INFO("WMI services proxy blanket set.");
  return S_OK;
}

HRESULT ProcessManager::createEventSink()
{
  HRESULT hr;

  // 1. 创建 EventSink 的实例
  // EventSink::CreateInstance 是我们自定义的静态工厂方法
  // 它内部处理了 CComObject<EventSink>::CreateInstance 和 AddRef
  // 使用临时指针来创建 EventSink 实例
  std::unique_ptr<EventSink> tempSink;
  EventSink *rawPtr = nullptr;
  hr = EventSink::CreateInstance(this, &rawPtr);

  if (FAILED(hr) || !rawPtr)
  {
    m_pSvc = nullptr;
    m_pLoc = nullptr;
    LOG_HRESULT(L"EventSink::CreateInstance failed", hr);
    handleError(hr, L"EventSink::CreateInstance failed");
    // 确保返回一个失败的HRESULT
    return FAILED(hr) ? hr : E_FAIL;
  }

  // 将原始指针赋值给智能指针，转移所有权到成员变量
  tempSink.reset(rawPtr);
  m_pEventSink = std::move(tempSink);

  // m_pEventSink 的引用计数现在是 1 (由 CreateInstance 保证)
  // m_pEventSink is now AddRef'd by CreateInstance
  LOG_INFO(L"EventSink instance created.");

  // 2. 创建 IUnsecuredApartment 实例
  // IUnsecuredApartment 用于创建 "stub" sink，以便从 WMI (可能在不同线程)安全地回调我们的 EventSink。
  // 这是推荐的做法，尤其是在使用 MTA 时，尽管 MTA 本身设计为能处理跨线程调用，
  // 但使用 IUnsecuredApartment 是更通用的 WMI 异步回调模式。
  hr = CoCreateInstance(CLSID_UnsecuredApartment,
                        nullptr,
                        CLSCTX_LOCAL_SERVER,
                        IID_IUnsecuredApartment,
                        (void **)&m_pUnsecApp);
  if (FAILED(hr))
  {
    LOG_HRESULT(L"Failed to create IUnsecuredApartment instance", hr);
    handleError(hr, L"Failed to create IUnsecuredApartment instance");
    // EventSink 创建成功，但后续失败，需要释放
    // // m_pEventSink->Release();
    // // m_pEventSink = nullptr;
    m_pEventSink.reset();
    m_pSvc = nullptr;
    m_pLoc = nullptr;
    return hr;
  }

  // 3. 使用 IUnsecuredApartment 创建 Stub Sink
  // m_pEventSink 是我们 IWbemObjectSink 的实际实现
  // m_pStubSink 将是 WMI 实际调用的代理 sink
  IUnknown *pSinkUnk = nullptr;
  // 获取 EventSink 的 IUnknown
  m_pEventSink->QueryInterface(IID_IUnknown, (void **)&pSinkUnk);
  if (!pSinkUnk)
  {
    // QueryInterface 应该会 AddRef
    m_pUnsecApp.Release();
    // // m_pEventSink->Release();
    // // m_pEventSink = nullptr;
    m_pEventSink.reset();
    LOG_HRESULT(L"QueryInterface for IUnknown on EventSink failed", E_NOINTERFACE);
    handleError(E_NOINTERFACE, L"QueryInterface for IUnknown on EventSink failed");
    return E_NOINTERFACE;
  }

  // m_pUnsecApp->CreateObjectStub 会 AddRef pSinkUnk
  // Create the stub sink that WMI will call
  // The m_pEventSink (IWbemObjectSink*) is passed to CreateObjectStub.
  // The returned m_pStubSink is also an IWbemObjectSink* that WMI can safely call.
  hr = m_pUnsecApp->CreateObjectStub(pSinkUnk, (IUnknown **)&m_pStubSink);
  // 我们在这里 Release 通过 QueryInterface 获得的 pSinkUnk 引用
  // 因为 CreateObjectStub 已经 AddRef 了它，并且 m_pStubSink (CComPtr) 会管理 stub 的生命周期，
  // stub 又会管理原始 pSinkUnk (即 m_pEventSink)的生命周期。

  // CreateObjectStub 会 AddRef pSinkUnk，所以这里我们 Release 我们 QI 得到的引用
  pSinkUnk->Release();

  if (FAILED(hr))
  {
    // CComPtr releases it
    m_pUnsecApp = nullptr;
    // // m_pEventSink->Release();
    // // m_pEventSink = nullptr;
    m_pEventSink.reset();
    m_pSvc = nullptr;
    m_pLoc = nullptr;
    LOG_HRESULT(L"Failed to create stub sink using IUnsecuredApartment", hr);
    handleError(hr, L"Failed to create stub sink using IUnsecuredApartment");
    return hr;
  }
  // m_pStubSink 的引用计数现在由 CreateObjectStub 管理，通常是1
  // m_pStubSink is now AddRef'd by CreateObjectStub, and CComPtr will manage its lifetime.

  // LOG_INFO(L"EventSink and StubSink created successfully.");
  return S_OK;
}

HRESULT ProcessManager::initializeListener()
{
  HRESULT hr = S_OK;

  // 1. Initialize COM
  hr = initializeCOM();
  if (FAILED(hr))
  {
    LOG_ERROR("ProcessManager: COM initialization failed in listener thread.");
    return hr;
  }

  LOG_INFO(L"ProcessManager: COM initialized successfully in listener thread (MTA).");

  // 2. Initialize WMI Locator and Service (m_pLoc, m_pSvc)
  // These are CComPtr, so they manage their own AddRef/Release for assignment.
  hr = initializeWMI();
  if (FAILED(hr))
  {
    // 此处不需要 CoUninitialize，它将在 runListenerLoop 的末尾统一处理
    LOG_ERROR("ProcessManager: WMI initialization failed in listener thread.");
    return hr;
  }
  // LOG_INFO(L"ProcessManager: WMI initialized successfully.")

  // 3. Create Event Sink (m_pEventSink and m_pStubSink)
  // Create the CComObject<EventSink>
  // EventSink::CreateInstance handles AddRef for the returned pointer
  hr = createEventSink();
  if (FAILED(hr))
  {
    LOG_HRESULT(L"ProcessManager: EventSink creation failed in listener thread", hr);
    return hr;
  }
  // LOG_INFO(L"ProcessManager: EventSink created successfully.");

  return hr;
}

bool ProcessManager::registerForEvents(const std::vector<std::string> &processNames)
{
  if (!m_pSvc || !m_pStubSink)
  {
    LOG_ERROR(L"WMI service or stub sink is not initialized. Cannot register for events.");
    return false;
  }

  HRESULT hr;
  bool allRegistered = true;

  // 通用查询语言
  _bstr_t bstrQueryLanguage = L"WQL";

  for (const auto &procName : processNames)
  {
    std::wstring procNameWs = MultiByteToWide(procName);
    if (procNameWs.empty() && !procName.empty())
    {
      LOG_ERROR(L"Failed to convert process name to wstring: " + procNameWs);
      allRegistered = false;
      continue;
    }

    // 1. 注册进程创建事件
    std::wstringstream wssCreate;
    wssCreate << L"SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.Name='" << procNameWs << L"'";
    _bstr_t bstrCreateQuery = wssCreate.str().c_str();

    hr = m_pSvc->ExecNotificationQueryAsync(
        bstrQueryLanguage,
        bstrCreateQuery,
        WBEM_FLAG_SEND_STATUS, // 可选，用于接收 SetStatus 调用
        nullptr,               // Context
        m_pStubSink            // 事件接收器 (stub)
    );
    if (FAILED(hr))
    {
      handleError(hr, L"ExecNotificationQueryAsync for process creation failed for: " + procNameWs);
      LOG_HRESULT(L"ProcessManager: ExecNotificationQueryAsync for process creation failed in listener thread", hr);
      allRegistered = false;
    }
    else
    {
      LOG_INFO(L"Successfully registered for CREATION events for: " + procNameWs);
    }

    // 2. 注册进程销毁事件
    std::wstringstream wssDelete;
    wssDelete << L"SELECT * FROM __InstanceDeletionEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.Name='" << procNameWs << L"'";
    _bstr_t bstrDeleteQuery = wssDelete.str().c_str();

    hr = m_pSvc->ExecNotificationQueryAsync(
        bstrQueryLanguage,
        bstrDeleteQuery,
        WBEM_FLAG_SEND_STATUS,
        nullptr,
        m_pStubSink);
    if (FAILED(hr))
    {
      handleError(hr, L"ExecNotificationQueryAsync for process deletion failed for: " + procNameWs);
      LOG_HRESULT(L"ProcessManager: ExecNotificationQueryAsync for process deletion failed in listener thread", hr);
      allRegistered = false;
    }
    else
    {
      LOG_INFO(L"Successfully registered for DELETION events for: " + procNameWs);
    }
  }
  return allRegistered;
}

void ProcessManager::runListenerLoop(std::vector<std::string> processNames)
{

  bool comInitializedInThisThread = false;

  // 初始化COM (MTA), WMI, EventSink
  if (FAILED(initializeListener()))
  {
    LOG_ERROR("Failed to initialize listener components (COM/WMI/EventSink).");

    // Ensure m_isListening is false if initialization failed
    m_isListening = false;
    // cleanupListenerThread will handle partial resource cleanup based on what was initialized
    // Call it to ensure any partially acquired resources are released.
    cleanupListenerThread();

    // 使用一个通用的失败代码触发错误回调或记录
    triggerErrorCallback(E_FAIL);
  }

  // 标记COM在此线程中初始化成功
  comInitializedInThisThread = true;
  LOG_INFO(L"Listener initialization successful. Registering for events...");

  // 注册事件
  if (!registerForEvents(processNames))
  {
    LOG_ERROR(L"Failed to register for WMI events.");
    // 触发错误回调记录,
    triggerErrorCallback(E_FAIL);
  }

  m_isListening = true;
  LOG_INFO(L"Event registration successful. Listener loop started.");

  // 通知等待线程
  m_cv.notify_one();

  // 进入等待状态，直到 m_stopEvent 被触发
  // CoWaitForMultipleHandles 允许 COM 消息泵继续运行，以便接收回调
  HRESULT hr = S_OK;
  DWORD waitResult;
  // 只有一个等待句柄
  HANDLE handles[] = {m_stopEvent};

  // 使用 m_isListening 作为循环条件
  while (m_isListening.load())
  {
    // 等待停止事件，超时时间设为1秒，以便定期检查 m_isListening
    // 这样，即使外部没有正确设置 m_stopEvent，如果 m_isListening 变为 false，循环也会退出
    // 同时，COM 调用可以在此期间被处理
    hr = CoWaitForMultipleHandles(
        COWAIT_DISPATCH_CALLS | COWAIT_DISPATCH_WINDOW_MESSAGES, // Flags: COWAIT_NONE (or other flags if needed)
                                                                 // COWAIT_DISPATCH_CALLS | COWAIT_DISPATCH_WINDOW_MESSAGES (如果需要处理窗口消息)
                                                                 // 对于纯粹的WMI回调，通常不需要复杂的标志，COM会自动处理。
                                                                 // 0 (COWAIT_NONE) 通常意味着等待，同时允许COM调用。
        5000,                                                    // Timeout in milliseconds
        1,                                                       // Number of handles
        handles,                                                 // Array of handles
        &waitResult                                              // Index of signaled handle (not strictly needed for one handle)
    );

    if (hr == RPC_S_CALLPENDING)
    {
      // COM call is pending, continue to allow it to be processed
      // This can happen if COM needs to process outgoing calls while waiting
      continue;
    }
    else if (hr == RPC_E_DISCONNECTED)
    {
      // 远程对象断开时重建连接 / remote object disconnected
      LOG_ERROR(L"Remote object disconnected. Attempting to reconnect...");
      // 这里可以尝试重新连接或处理断开连接的逻辑
      // 退出循环，可能需要重新初始化
      break;
    }

    if (waitResult == WAIT_OBJECT_0)
    {
      // m_stopEvent was signaled
      // LOG_INFO("Stop event received in listener loop.");
      break;
    }
    else if (waitResult == WAIT_TIMEOUT)
    {
      // Timeout, loop again to check m_isListening and process COM messages
      continue;
    }
    else
    {
      // Some other error or unexpected result from CoWaitForMultipleHandles
      long lastError = GetLastError();
      _com_error err(HRESULT_FROM_WIN32(waitResult));
      // 通知错误回调
      triggerErrorCallback(static_cast<long>(lastError));
      break;
    }
  }
  // 在监听线程结束前取消订阅并释放资源
  cleanupListenerThread();

  if (comInitializedInThisThread)
  {
    // 仅当此线程成功初始化COM时才反初始化
    CoUninitialize();
    // LOG_INFO("COM uninitialized in listener thread.");
  }

  // LOG_INFO("Listener thread finished.");
  // 确保状态更新
  m_isListening = false;
}

bool ProcessManager::startListening(const std::vector<std::string> &processNames)
{
  if (m_isListening.load() && m_listenerThread.joinable())
  {
    LOG_ERROR("Already listening. Please stop the current listener first.");
    return false;
  }

  if (!m_stopEvent)
  {
    LOG_ERROR("Stop event is null. Cannot start listener.");
    return false;
  }
  // 确保事件在开始时是非信号状态
  ResetEvent(m_stopEvent);
  // Reset global stop flag
  m_stopGlobalRequested = false;

  // 将 processNames 复制一份传递给线程，以避免生命周期问题
  // 或者确保 processNames 的生命周期超过线程
  // std::thread 的构造函数会复制或移动参数
  try
  {
    // Prevent race conditions with startListening
    std::unique_lock<std::mutex> lock(m_setMutex);
    // 使用 lambda 捕获 this 指针和 processNames (通过值传递以确保生命周期)
    // Capture processNames by value
    m_listenerThread = std::thread([this, processNamesCopy = processNames]()
                                   { this->runListenerLoop(processNamesCopy); });
    // 等待条件变量通知，让线程有机会启动并设置 m_isListening，或者超时
    if (!m_cv.wait_for(lock, std::chrono::seconds(5),
                       [this]
                       { return m_isListening.load(); }))
    {
      LOG_ERROR("Failed to start listener thread: Timed out waiting for listener to start.");
      m_isListening = false;
      return false;
    }
  }
  catch (const std::system_error &e)
  {
    LOG_ERROR("Failed to start listener thread: " + std::string(e.what()));
    m_isListening = false;
    return false;
  }

  // while (!m_isListening.load())
  // {
  //   // 等待条件变量通知，直到 m_isListening 被设置为 true
  //   // 或者超时
  //   std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // }

  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // 返回实际的监听状态
  return m_isListening.load();
}

bool ProcessManager::stopListening()
{
  // Prevent race conditions with startListening
  std::lock_guard<std::mutex> lock(m_setMutex);

  // 如果监听器没有启动或线程不可连接，直接返回
  // 这可以防止多次调用 stopListening 导致的错误
  // 这可能是因为监听器已经停止或从未成功启动
  // 这里可以选择记录日志或抛出异常
  if (!m_isListening.load() || !(m_listenerThread.joinable()))
  {
    LOG_ERROR("Listener is not active or thread not joinable.");
    return true;
  }
  // Indicate a stop has been requested.
  m_stopGlobalRequested = true;

  // LOG_INFO("Requesting listener thread to stop...");

  // 通知监听线程停止
  if (m_stopEvent)
  {
    SetEvent(m_stopEvent);
  }

  // ! 这里不要使用 m_isListening = false; 有可能会导致死锁
  // ! 因为在监听线程中，m_isListening 是在 CoWaitForMultipleHandles 循环中检查的
  // m_isListening = false;

  if (m_listenerThread.joinable())
  {
    try
    {
      // 等待监听线程结束
      m_listenerThread.join();
      // LOG_INFO("Listener thread joined successfully.");
    }
    catch (const std::system_error &e)
    {
      LOG_ERROR("Error joining listener thread: " + std::string(e.what()));
      // 即使 join 失败，也应继续尝试清理
    }
  }
  else
  {
    // LOG_INFO("Listener thread was not joinable (perhaps never started or already finished).");
  }

  // LOG_INFO(" Stop process complete.");

  return true;
}

void ProcessManager::cleanupListenerThread()
{
  // LOG_INFO("Starting cleanup...");

  // 1. 取消 WMI 异步调用 / Cancel WMI asynchronous calls
  // 这个操作必须在创建异步调用的同一个线程中执行
  // This should be done first, and in the same thread that called ExecNotificationQueryAsync.

  // Check if IWbemServices and the stub sink are valid
  if (m_pSvc && m_pStubSink)
  {
    HRESULT hr = m_pSvc->CancelAsyncCall(m_pStubSink);
    if (FAILED(hr))
    {
      // RPC_E_DISCONNECTED can happen if the WMI service has already terminated or
      // if COM was uninitialized prematurely for this thread.
      // WBEM_E_INVALID_OPERATION if there were no calls to cancel.
      if (hr == RPC_E_DISCONNECTED)
      {
        LOG_ERROR("WMI Warning: m_pSvc->CancelAsyncCall failed because the object was disconnected (RPC_E_DISCONNECTED). This might be okay if the listener is already shutting down.");
      }
      else if (hr == WBEM_E_INVALID_OPERATION)
      {
        // LOG_INFO("WMI Info: CancelAsyncCall returned WBEM_E_INVALID_OPERATION (no active calls to cancel).");
      }
      else
      {
        _com_error err(hr);
        LOG_HRESULT("WMI Error: m_pSvc->CancelAsyncCall failed.", hr);
        // 即使失败，也继续尝试释放其他资源
      }
    }
    else
    {
      // LOG_INFO("WMI asynchronous calls cancelled successfully via m_pStubSink.");
    }
  }
  else
  {
    // LOG_INFO("IWbemServices or m_pStubSink is null, skipping CancelAsyncCall.");
  }

  // 2. 释放 StubSink
  // m_pStubSink 是通过 IUnsecuredApartment 创建的
  // Release the stub sink (CComPtr will handle this, but explicit Release is fine for clarity if desired)
  // m_pStubSink 是 m_pEventSink 的一个包装器（通过 IUnsecuredApartment 创建）
  if (m_pStubSink)
  {
    // m_pStubSink 的 Release 由 CComPtr 自动处理
    // CComPtr sets itself to nullptr on Release()
    m_pStubSink.Release();
    // LOG_INFO("StubSink (m_pStubSink) released.");
  }

  // 3. 释放 IUnsecuredApartment
  if (m_pUnsecApp)
  {
    // m_pUnsecApp 的 Release 由 CComPtr 自动处理
    m_pUnsecApp.Release();
    // LOG_INFO("IUnsecuredApartment (m_pUnsecApp) released.");
  }

  // 4. 释放我们实际的 EventSink 对象 / Release our actual EventSink object (m_pEventSink)
  // 我们需要释放我们通过 CreateInstance 创建的原始 m_pEventSink
  // This was created by EventSink::CreateInstance and AddRef'd. We must Release it.
  if (m_pEventSink)
  {
    // m_pEventSink 是一个 std::unique_ptr<EventSink>，它会自动调用 delete
    // 但如果我们在这里手动释放，确保它不会被重复释放
    // 这将调用 EventSink 的析构函数
    // 注意：如果我们在这里使用 Release()，则不需要 std::unique_ptr 的 reset()
    // 但为了保持一致性和清晰性，我们使用 reset() 来释放它。
    // EventSink 是通过 CComObject<EventSink>::CreateInstance 创建的，
    // 我们持有它的一个 AddRef'd 指针。我们需要调用 Release。
    // // ULONG refCount = m_pEventSink->Release();
    // // Important: Set to nullptr after release
    // // m_pEventSink = nullptr;
    // 因为我们在创建时使用了 std::unique_ptr<EventSink>，所以这需要使用 reset 来释放
    m_pEventSink.reset();
    // LOG_INFO("Actual EventSink (m_pEventSink) released.");
  }

  // 5. 释放 WMI 服务 / Release IWbemServices (m_pSvc) (CComPtr will handle this)
  if (m_pSvc)
  {
    // m_pSvc 的 Release 由 CComPtr 自动处理
    m_pSvc.Release();
    // LOG_INFO("IWbemServices (m_pSvc) released.");
  }

  // 6. 释放 WMI 定位器 / Release IWbemLocator (m_pLoc) (CComPtr will handle this)
  if (m_pLoc)
  {
    // m_pLoc 的 Release 由 CComPtr 自动处理
    m_pLoc.Release();
    // LOG_INFO("IWbemLocator (m_pLoc) released.");
  }

  // LOG_INFO("Cleanup finished.");
}

void ProcessManager::handleError(HRESULT errorCode, const std::wstring &errorMessage)
{
  // 如果错误是 RPC_E_DISCONNECTED，并且我们正在停止，那么这可能是预期的。
  // If a stop is requested globally (e.g. Ctrl+C or stopListening called)
  // and the error is RPC_E_DISCONNECTED, it's likely due to the shutdown process.
  if (errorCode == RPC_E_DISCONNECTED && m_stopGlobalRequested.load())
  {
    LOG_WARN("WMI Disconnected (RPC_E_DISCONNECTED) during shutdown process. This is likely expected.");
    // Optionally, call the user's error callback anyway, or just log and return.
    // For now, just log and return to avoid redundant error spam during shutdown.
    if (m_onErrorCallback)
    {
      // Decide if user needs to see this during shutdown
      m_onErrorCallback(errorCode);
    }
    return;
  }

  _com_error err(errorCode);
  LOG_HRESULT(L"ErrorMessage: " + errorMessage + L"WMI Error", errorCode);

  // 如果用户设置了错误回调则触发 / Trigger the user's error callback if set
  // 加锁以保护回调调用，如果回调可能来自不同线程（虽然这里主要在监听线程内）/ Protect callback access
  std::lock_guard<std::mutex> lock(m_callbackMutex);
  if (m_onErrorCallback)
  {
    try
    {
      m_onErrorCallback(static_cast<long>(errorCode));
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("Exception in onErrorCallback: " + std::string(e.what()));
    }
    catch (...)
    {
      LOG_ERROR("Unknown exception in onErrorCallback.");
    }
  }

  // 这里可以根据错误代码决定是否需要停止监听或执行其他恢复操作
  // 例如，某些严重的错误可能意味着无法继续监听
  if (errorCode == WBEM_E_TRANSPORT_FAILURE || errorCode == WBEM_E_CRITICAL_ERROR)
  {
    LOG_ERROR("Critical WMI error, attempting to stop listener...");
    // ! 不能直接调用 stopListening() 因为这可能导致死锁
    // 应该设置一个标志，让主循环来处理停止
    if (m_stopEvent)
    {
      SetEvent(m_stopEvent);
    }
    // 或者直接设置 m_isListening 为 false
    // ! 这可能会导致死锁，因为 stopListening 可能在等待
    // m_isListening = false;
  }
}

void ProcessManager::setOnProcessCreatedCallback(ProcessEventCallback callback)
{
  // 使用 std::move 提高效率，避免不必要的拷贝
  m_onProcessCreatedCallback = std::move(callback);
}

void ProcessManager::setOnProcessDestroyedCallback(ProcessEventCallback callback)
{
  // 使用 std::move 提高效率，避免不必要的拷贝
  m_onProcessDestroyedCallback = std::move(callback);
}

void ProcessManager::setOnErrorCallback(ErrorCallback callback)
{
  // 使用 std::move 提高效率，避免不必要的拷贝
  m_onErrorCallback = std::move(callback);
}

bool ProcessManager::isListening() const
{
  return m_isListening;
}

// --- 回调触发方法 ---
// 这些方法由 EventSink 调用，因此需要是线程安全的，这里加锁保证安全。
// 通常，EventSink 的 Indicate 方法在 WMI 的某个线程上被调用。

void ProcessManager::triggerProcessCreatedCallback(const std::wstring &processName, DWORD processId)
{
  // 保护回调
  std::lock_guard<std::mutex> lock(m_callbackMutex);
  if (m_onProcessCreatedCallback)
  {
    // 检查回调是否已设置
    try
    {
      m_onProcessCreatedCallback(processName, processId);
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("Exception in onProcessCreated callback: " + std::string(e.what()));
    }
    catch (...)
    {
      LOG_ERROR("Unknown exception in onProcessCreated callback.");
    }
  }
  else
  {
    // 如果没有设置回调，可以提供默认行为（例如打印到控制台）
    LOG_INFO(L"[Default] Process CREATED: " + processName);
  }
}

void ProcessManager::triggerProcessDestroyedCallback(const std::wstring &processName, DWORD processId)
{
  // 保护回调
  std::lock_guard<std::mutex> lock(m_callbackMutex);
  if (m_onProcessDestroyedCallback)
  {
    try
    {
      m_onProcessDestroyedCallback(processName, processId);
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("Exception in onProcessDestroyed callback: " + std::string(e.what()));
    }
    catch (...)
    {
      LOG_ERROR("Unknown exception in onProcessDestroyed callback.");
    }
  }
  else
  {
    LOG_INFO(L"[Default] Process DESTROYED: " + processName);
  }
}

void ProcessManager::triggerErrorCallback(long errorCode)
{
  // 保护回调
  std::lock_guard<std::mutex> lock(m_callbackMutex);
  if (m_onErrorCallback)
  {
    try
    {
      m_onErrorCallback(errorCode);
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("Exception in onError callback: " + std::string(e.what()));
    }
    catch (...)
    {
      LOG_ERROR("Unknown exception in onError callback.");
    }
  }
  else
  {
    // 默认错误处理
    _com_error err(errorCode);
    LOG_ERROR(L"[Default ERROR] WMI Error: HRESULT=0x" + std::to_wstring(errorCode) +
              L", Message: " + std::wstring(err.ErrorMessage()));
  }
}

bool ProcessManager::restrictAntiCheatProcess(const std::wstring &processName, DWORD processId)
{
  // 检查PID是否为0
  if (processId == 0)
  {
    LOG_ERROR(L"进程ID无效: " + processName);
    return false;
  }

  try
  {
    // 打开进程
    HANDLE hProcess = OpenProcess(
        PROCESS_SET_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        processId);
    if (hProcess)
    {
      // 优先级设置
      bool prioritySet = false;
      // 亲和性设置
      bool affinitySet = false;

      // 设置低优先级
      if (SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS))
      {
        prioritySet = true;
        LOG_INFO(L"设置进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 优先级为低");
      }
      else
      {
        LOG_HRESULT(L"设置进程 " + processName + L" PID " + std::to_wstring(processId) + L" 优先级失败", GetLastError());
      }

      // 绑定到最后一个逻辑处理器
      SYSTEM_INFO sysInfo;
      GetSystemInfo(&sysInfo);
      if (sysInfo.dwNumberOfProcessors > 0)
      {
        DWORD_PTR lastCoreMask = (DWORD_PTR)1 << (sysInfo.dwNumberOfProcessors - 1);
        if (SetProcessAffinityMask(hProcess, lastCoreMask))
        {
          affinitySet = true;
          LOG_INFO(L"设置进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 亲和性为最后一个核心: " + std::to_wstring(lastCoreMask));
        }
        else
        {
          LOG_HRESULT(L"设置进程" + processName + L" PID: " + std::to_wstring(processId) + L" 亲和性失败", GetLastError());
        }
      }
      else
      {
        LOG_WARN(L"无法获取处理器数量，无法设置进程亲和性");
      }

      CloseHandle(hProcess);
      // 两个都设置成功才返回 true
      return prioritySet && affinitySet;
    }
    else
    {
      LOG_HRESULT(L"无法打开进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 进行限制", GetLastError());
    }
  }
  catch (const std::exception &e)
  {
    LOG_ERROR(L"限制进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 失败：" + MultiByteToWide(e.what()));
  }
  catch (...)
  {
    LOG_ERROR(L"限制进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 失败");
  }

  return false;
}

bool ProcessManager::restrictAntiCheatProcessPS(const std::wstring &processName, DWORD processId)
{
  // 检查PID是否为0
  if (processId == 0)
  {
    LOG_ERROR(L"进程ID无效: " + processName);
    return false;
  }

  try
  {
    // 获取最后一个逻辑处理器
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors < 0)
    {
      LOG_WARN(L"无法获取处理器数量，无法设置进程亲和性");
      return false;
    }

    DWORD_PTR lastCoreMask = (DWORD_PTR)1 << (sysInfo.dwNumberOfProcessors - 1);

    // 去除processName的进程名 .exe 后缀
    std::wstring processNameWithoutExt = processName.substr(0, processName.find_last_of(L"."));

    if (SetProcessPriorityAndAffinity(processNameWithoutExt, L"Idle", lastCoreMask))
    {
      LOG_INFO(L"成功设置进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 的优先级和亲和性");
      return true;
    }
    else
    {
      LOG_ERROR(L"设置进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 的优先级和亲和性失败");
      return false;
    }
  }
  catch (const std::exception &e)
  {
    LOG_ERROR(L"限制进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 失败：" + MultiByteToWide(e.what()));
  }
  catch (...)
  {
    LOG_ERROR(L"限制进程 " + processName + L" PID: " + std::to_wstring(processId) + L" 失败");
  }

  return false;
}