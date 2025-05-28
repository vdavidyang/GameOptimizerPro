/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-03 18:08:49
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 01:30:49
 * @FilePath: \GameOptimizerPro\src\utils\event_sink.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "utils/event_sink.h"

// 静态创建函数实现
HRESULT EventSink::CreateInstance(ProcessManager *pMgr, EventSink **ppSink)
{
  if (!ppSink)
    return E_POINTER;
  *ppSink = nullptr;

  // 临时变量，指向具体实现类
  CComObject<EventSink> *pSinkImpl = nullptr;
  // 创建实现类的实例
  HRESULT hr = CComObject<EventSink>::CreateInstance(&pSinkImpl);
  if (SUCCEEDED(hr))
  {
    // pSinkImpl 的初始引用计数是 0

    // 设置 ProcessManager 指针
    pSinkImpl->SetProcessManager(pMgr);

    // 调用 AddRef() 使引用计数变为 1，符合 COM 返回规则 (返回 AddRef'd 的指针)
    pSinkImpl->AddRef();

    // 将实现类指针 (CComObject<EventSink>*) 赋值给基类指针 (EventSink*)
    // 这是安全的，因为 CComObject<EventSink> 继承自 EventSink
    *ppSink = pSinkImpl;
  }
  return hr;
}

void EventSink::SetProcessManager(ProcessManager *pMgr)
{
  m_pProcessManager = pMgr;
}

STDMETHODIMP EventSink::Indicate(long lObjectCount, IWbemClassObject **apObjArray)
{
  if (!m_pProcessManager)
  {
    // Should not happen if properly initialized
    LOG_ERROR(L"EventSink::Indicate - m_pProcessManager is null.");
    return WBEM_E_INVALID_PARAMETER;
  }

  // Check if the ProcessManager is still considered to be listening.
  // If ProcessManager requested a stop, it might be in the process of shutting down.
  // Calling back into it might be problematic or lead to race conditions if not handled carefully.
  // However, WMI might still deliver some final events.
  // For now, we assume ProcessManager handles calls евреn during its shutdown phase gracefully.

  HRESULT hr = S_OK;
  for (long i = 0; i < lObjectCount; i++)
  {
    // Current event object
    CComPtr<IWbemClassObject> pObj = apObjArray[i];
    if (!pObj)
    {
      continue;
    }

    // Get the class name of the event (e.g., __InstanceCreationEvent)
    // Using _variant_t for RAII
    _variant_t vtClassName;
    hr = pObj->Get(L"__CLASS", 0, &vtClassName, nullptr, nullptr);
    if (FAILED(hr) || vtClassName.vt != VT_BSTR)
    {
      // Report error
      m_pProcessManager->triggerErrorCallback(FAILED(hr) ? hr : WBEM_E_FAILED);
      // _variant_t destructor will handle this
      vtClassName.Clear();
      // Process next event if any
      continue;
    }

    // Get the TargetInstance from the event object
    _variant_t vtTargetInstance;
    hr = pObj->Get(L"TargetInstance", 0, &vtTargetInstance, nullptr, nullptr);
    if (FAILED(hr) || vtTargetInstance.vt != VT_UNKNOWN || vtTargetInstance.punkVal == nullptr)
    {
      m_pProcessManager->triggerErrorCallback(FAILED(hr) ? hr : WBEM_E_FAILED);
      vtTargetInstance.Clear();
      vtClassName.Clear();
      continue;
    }

    // QueryInterface for IWbemClassObject on the TargetInstance
    // Use CComPtr for automatic Release
    // The IUnknown from vtTargetInstance needs to be QI'd for IWbemClassObject
    CComPtr<IWbemClassObject> pTargetInst;
    hr = vtTargetInstance.punkVal->QueryInterface(IID_IWbemClassObject, (void **)&pTargetInst);

    // punkVal itself is AddRef'd by Get, and _variant_t will Release it.
    // pTargetInst, if successful, will also be AddRef'd. CComPtr handles its release.
    //  Clear original variant after QI or if QI fails
    vtTargetInstance.Clear();
    if (FAILED(hr) || !pTargetInst)
    {
      m_pProcessManager->triggerErrorCallback(FAILED(hr) ? hr : WBEM_E_FAILED);
      vtClassName.Clear();
      continue;
    }

    // Get the process name from the TargetInstance
    _variant_t vtProcName;
    std::wstring processName = L"";
    hr = pTargetInst->Get(L"Name", 0, &vtProcName, nullptr, nullptr);
    if (FAILED(hr) || vtProcName.vt != VT_BSTR)
    {
      m_pProcessManager->triggerErrorCallback(FAILED(hr) ? hr : WBEM_E_FAILED);
      vtProcName.Clear();
      // pTargetInst is released by CComPtr
      vtClassName.Clear();
      continue;
    }

    processName = vtProcName.bstrVal;
    // _variant_t destructor handles BSTR deallocation
    vtProcName.Clear();

    // Get the process pid from the TargetInstance
    _variant_t vtProcId;
    // 默认 PID 为 0
    DWORD procId = 0;
    hr = pTargetInst->Get(L"ProcessId", 0, &vtProcId, nullptr, nullptr);
    if (FAILED(hr))
    {
      // 获取 ProcessId 失败，PID 将保持为 0
      LOG_HRESULT("Get ProcessId failed", hr);
      m_pProcessManager->triggerErrorCallback(hr);
      vtProcId.Clear();
      // pTargetInst is released by CComPtr
      vtClassName.Clear();
    }

    if (vtProcId.vt == VT_I4) // ProcessId 通常是 VT_I4 (long)
    {
      procId = static_cast<DWORD>(vtProcId.lVal);
    }
    else if (vtProcId.vt == VT_UI4) // 也可能是 VT_UI4 (unsigned long)
    {
      procId = static_cast<DWORD>(vtProcId.ulVal);
    }
    else
    {
      // 如果类型不匹配，可以记录一个警告或错误，PID 将保持为 0
      LOG_HRESULT(L"ProcessId for " + processName + L" has unexpected VARIANT type: " + std::to_wstring(vtProcId.vt), WBEM_E_TYPE_MISMATCH);
      m_pProcessManager->triggerErrorCallback(WBEM_E_TYPE_MISMATCH);
    }

    // _variant_t destructor handles lVal/ulVal deallocation
    vtProcId.Clear();

    // 检查事件类型并调用相应的回调
    // Determine event type and call the appropriate ProcessManager trigger
    if (_wcsicmp(vtClassName.bstrVal, L"__InstanceCreationEvent") == 0)
    {
      m_pProcessManager->triggerProcessCreatedCallback(processName, procId);
    }
    else if (_wcsicmp(vtClassName.bstrVal, L"__InstanceDeletionEvent") == 0)
    {
      m_pProcessManager->triggerProcessDestroyedCallback(processName, procId);
    }
    // else: unhandled event type, could log if necessary

    // _variant_t destructor handles BSTR deallocation
    vtClassName.Clear();
    // pTargetInst is released by CComPtr destructor
  }

  // Always return S_NO_ERROR to WMI to continue receiving events,
  // unless a catastrophic, non-recoverable error specific to Indicate occurs.
  // Individual event processing errors are handled by calling triggerError.
  return WBEM_S_NO_ERROR;
}

STDMETHODIMP EventSink::SetStatus(
    /* [in] */ long lFlags,
    /* [in] */ HRESULT hResult,
    /* [in] */ BSTR strParam,
    /* [in] */ IWbemClassObject __RPC_FAR * /*pObjParam*/ // comment it because unused
)
// long lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject *pObjParam)
{
  /**
   * long lFlags:
   * WBEM_STATUS_COMPLETE: Final status, operation is complete (hResult has final code).
   * WBEM_STATUS_PROGRESS: Intermediate status (not typically used for these queries).
   * WBEM_STATUS_REQUIREMENTS: Related to security or DCOM negotiation.
   *
   * WBEM_STATUS_COMPLETE 表示操作完成 (成功或失败)
   * WBEM_STATUS_PROGRESS 报告操作进度 (通常不需要处理)
   *  WBEM_STATUS_REQUIREMENTS (不常用)
   */

  if (!m_pProcessManager)
  {
    // Should not happen
    return WBEM_S_NO_ERROR; // Or an error code if this state is critical
  }

  if (lFlags == WBEM_STATUS_COMPLETE)
  {
    LOG_HRESULT(L"EventSink::SetStatus - WBEM_STATUS_COMPLETE received", hResult);
    if (strParam)
    {
      std::wcout << L", Param: " << strParam;
    }
    std::wcout << std::endl;

    // hResult indicates the success or failure of the ExecNotificationQueryAsync operation.
    // If FAILED(hResult), it means the event subscription itself failed or was terminated.
    if (FAILED(hResult))
    {
      // This is a significant error; the query might have been cancelled or an error occurred.
      // If hResult is RPC_E_DISCONNECTED and we are shutting down, this might be expected.
      // Or if it's WBEM_E_CALL_CANCELLED.
      if (hResult == WBEM_E_CALL_CANCELLED)
      {
        // LOG_INFO(L"EventSink::SetStatus - Operation was cancelled (WBEM_E_CALL_CANCELLED).");
      }
      else
      {
        // Forward a more general error, or a specific one if hResult indicates a problem
        // that ProcessManager should know about beyond individual event errors.
        m_pProcessManager->triggerErrorCallback(hResult);
      }
    }
  }
  // else if (lFlags == WBEM_STATUS_PROGRESS) {
  //   std::wcout << L"EventSink::SetStatus - WBEM_STATUS_PROGRESS" << std::endl;
  // }
  // else if (lFlags == WBEM_STATUS_REQUIREMENTS) {
  //    std::wcout << L"EventSink::SetStatus - WBEM_STATUS_REQUIREMENTS" << std::endl;
  // }

  // if (m_pProcessManager && FAILED(hResult) && lFlags == WBEM_STATUS_COMPLETE)
  // {
  //   // 异步操作失败时报告错误
  //   // 注意：即使 hResult 成功 (S_OK)，SetStatus 也可能被调用表示完成
  //   m_pProcessManager->triggerError(hResult);
  //   if (strParam)
  //   {
  //     std::wcerr << L"WMI SetStatus Error Parameter: " << strParam << std::endl;
  //   }
  // }

  // Tell WMI we've handled the status.
  return WBEM_S_NO_ERROR;
}