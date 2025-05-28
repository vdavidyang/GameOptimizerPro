/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-03 17:48:25
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-13 00:27:40
 * @FilePath: \GameOptimizerPro\include\utils\event_sink.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <Wbemidl.h>
#include <atlbase.h> // For CComObjectRootEx, CComCoClass
#include <atlcom.h>  // For CComPtr
#include <string>
#include <functional>
#include <vector>
#include <iostream> // For demo output
#include <comdef.h> // For _variant_t, _bstr_t
#include <iostream>

#include "core/process_manager.h"

// 前向声明 ProcessManager 类
class ProcessManager;

/**
 * @class EventSink
 * @brief 实现 IWbemObjectSink 接口以异步接收 WMI 事件通知。
 *
 * 该类处理来自 WMI 的事件，并调用 ProcessManager 中的回调函数。
 */
class EventSink : public CComObjectRootEx<CComMultiThreadModel>, // 支持多线程
                  public IWbemObjectSink
{
public:
  // ATL COM 映射表
  BEGIN_COM_MAP(EventSink)
  COM_INTERFACE_ENTRY(IWbemObjectSink)
  END_COM_MAP()

  // 使用 CComObject 创建 EventSink 实例的静态方法
  static HRESULT CreateInstance(ProcessManager *pMgr, EventSink **ppSink);

  /**
   * @brief 设置指向 ProcessManager 实例的指针。
   * @param pMgr 指向 ProcessManager 对象的指针。
   */
  void SetProcessManager(ProcessManager *pMgr);

  // --- IWbemObjectSink methods ---

  /**
   * @brief WMI 调用此方法来传递事件。
   * @param lObjectCount 数组中的对象数量。
   * @param apObjArray 指向 IWbemClassObject 指针数组的指针，每个指针代表一个事件。
   * @return HRESULT 状态码。
   */
  STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject **apObjArray);

  /**
   * @brief WMI 调用此方法来指示异步操作的状态。
   * @param lFlags 指示状态的标志 (WBEM_STATUS_COMPLETE, WBEM_STATUS_PROGRESS, WBEM_STATUS_REQUIREMENTS)。
   * @param hResult 操作的 HRESULT 结果。
   * @param strParam 描述错误的字符串 (如果 hResult 不是 WBEM_S_NO_ERROR)。
   * @param pObjParam 包含更详细错误信息的 IWbemClassObject 对象 (如果可用)。
   * @return HRESULT 状态码。
   */
  STDMETHOD(SetStatus)(long lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject *pObjParam);

private:
  // 指向 ProcessManager 的指针
  ProcessManager *m_pProcessManager = nullptr;
};
