/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:33:47
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-26 00:01:21
 * @FilePath: \GameOptimizerPro\include\log\logging.h
 * @Description: Logging class supporting different string types for messages and function names.
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once
#include <string>
#include <fstream>
#include <windows.h>
#include <locale>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <mutex>
#include <type_traits>
#include <cstring>
#include <comdef.h> // For _com_error

#include "core/config_manager.h"
#include "utils/system_utils.h"

#if defined(_MSC_VER)
#define LOG_FUNC_NAME __FUNCTIONW__ // MSVC provides __FUNCTIONW__ as const wchar_t*
#else
// GCC/Clang provide __func__ as const char* (UTF-8 usually)
// We need to convert it. Using a helper function directly in the macro.
#define LOG_FUNC_NAME MultiByteToWide(__func__)
#endif

#define LOG_INFO(msg) Logging::log(Logging::LogLevel::LOG_INFO, (msg), LOG_FUNC_NAME, __LINE__)
#define LOG_WARN(msg) Logging::log(Logging::LogLevel::LOG_WARNING, (msg), LOG_FUNC_NAME, __LINE__)
#define LOG_ERROR(msg) Logging::log(Logging::LogLevel::LOG_ERROR, (msg), LOG_FUNC_NAME, __LINE__)
#define LOG_DEBUG(msg) Logging::log(Logging::LogLevel::LOG_DEBUG, (msg), LOG_FUNC_NAME, __LINE__)

#define LOG_HRESULT(msg, hr) Logging::log(Logging::LogLevel::LOG_ERROR, (msg), LOG_FUNC_NAME, __LINE__, hr)

/**
 * @class Logging
 * @brief 日志类，负责初始化、关闭和记录日志
 * @note 该类使用静态类实现，确保全局可用
 * @note 日志级别包括 INFO、WARNING、ERROR 和 DEBUG
 * @note 支持多种字符串类型的消息和函数名
 * @note 日志文件路径在初始化时设置，默认路径为当前目录下的 log/game_optimizer.log
 */
class Logging
{
public:
  enum LogLevel
  {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
  };

  static bool initialize(const std::wstring &logFilePath);
  static void shutdown();

  /**
   * @brief 日志记录函数（模板）.
   * @tparam MsgType 日志消息类型 (e.g., std::string, std::wstring, const char*, etc.).
   * @tparam FuncNameType 函数名类型 (e.g., std::wstring, const wchar_t*).
   */
  template <typename MsgType, typename FuncNameType>
  static void log(
      LogLevel level,
      const MsgType &message,
      const FuncNameType &functionName,
      int line,
      HRESULT hr = S_OK)
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_isInitialized)
    {
      // 如果日志未初始化，输出到标准错误流
      std::wcerr << L"Log attempt before initialization." << std::endl;
      return;
    }

    // 确保日志流已打开且准备就绪
    if (!m_logFileStream.is_open() || m_logFileStream.fail())
    {
      if (!ReopenLogFile())
      {
        // 尝试重新打开文件
        // 如果重新打开失败，无法记录日志。输出到标准错误流可能是一个选项。
        std::wcerr << L"Cannot write to log file: " << m_logFilePath << std::endl;
        return;
      }
    }

    try
    {
      // 使用 GetSystemTime 获取 UTC 时间
      SYSTEMTIME st;
      GetLocalTime(&st);

      // 确定日志级别字符串
      const wchar_t *levelStr = L"UNKNOWN";
      switch (level)
      {
      // 添加空格以对齐
      case LogLevel::LOG_INFO:
        levelStr = L"INFO";
        break;
      case LogLevel::LOG_WARNING:
        levelStr = L"WARN";
        break;
      case LogLevel::LOG_ERROR:
        levelStr = L"ERROR";
        break;
      case LogLevel::LOG_DEBUG:
        levelStr = L"DEBUG";
        break;
      }

      // 将消息和函数名转换为 wstring 类型
      std::wstring wideMessage = GetWideString(message);
      std::wstring wideFuncName = GetWideString(functionName);

      // 格式化并写入日志条目
      // [YYYY-MM-DD HH:MM:SS.ms][LEVEL][Function:Line] Message (HRESULT: 0x...)
      wchar_t timeBuf[64];
      swprintf_s(timeBuf, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
                 st.wYear, st.wMonth, st.wDay,
                 st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

      m_logFileStream << L"[" << timeBuf << L"]"
                      << L"[" << levelStr << L"]"
                      << L"[" << wideFuncName << L":" << line << L"] "
                      << wideMessage;

      // 如果它是错误代码，附加 HRESULT 和描述
      if (FAILED(hr))
      {
        // 将 HRESULT 转换为宽字符串
        // 使用 swprintf_s 格式化 HRESULT
        // 0x00000000 是一个有效的 HRESULT，表示成功
        // 但我们只在失败时附加它
        wchar_t hrBuf[20];
        swprintf_s(hrBuf, L"0x%08X", static_cast<unsigned int>(hr));
        _com_error err(hr);
        // 将错误消息转换为宽字符串
        std::wstring hrMessage = GetWideString(err.ErrorMessage());
        // 附加 HRESULT 和描述
        m_logFileStream << L" (HRESULT: " << hrBuf << L", Description: " << hrMessage << L")";
      }

      // 结束日志条目
      m_logFileStream << std::endl;

      // 不需要显式刷新，因为 std::endl 默认使用 unitbuf，
      // 但刷新确保立即写入，特别是在错误/崩溃时。
      m_logFileStream.flush();
    }
    catch (const std::ios_base::failure &e)
    {
      // 处理写入/刷新期间的潜在流错误
      std::cerr << "Log stream I/O error: " << e.what() << std::endl;
      // 考虑关闭流或标记日志记录器失败以防止循环
      m_logFileStream.close();
    }
    catch (const std::exception &e)
    {
      // 捕获格式化或转换期间的潜在标准异常
      std::cerr << "Standard exception during logging: " << e.what() << std::endl;
    }
    catch (...)
    {
      // 捕获任何其他未知异常
      std::cerr << "Unknown exception during logging." << std::endl;
    }
  }

  static std::wstring getLogFilePath()
  {
    return m_logFilePath;
  }

private:
  // --- 私有成员 ---
  static std::mutex m_mutex;
  static std::wofstream m_logFileStream;
  static bool m_isInitialized;
  static std::wstring m_logFilePath;

  // --- 私有帮助函数 ---

  /**
   * @brief: 将各种字符串类型转换为 std::wstring 用于日志记录。
   * @tparam T 输入类型
   * @tparam input 输入字符串
   * @return 转换后的 std::wstring
   */
  template <typename T>
  static std::wstring GetWideString(const T &input)
  {
    // 处理 const/ref 限定符
    using DecayedT = std::decay_t<T>;

    // 如果 T 已经是 std::wstring
    if constexpr (std::is_same_v<DecayedT, std::wstring>)
    {
      return input;
    }
    // 如果 T 是 const wchar_t*
    else if constexpr (std::is_convertible_v<DecayedT, const wchar_t *>)
    {
      // 隐式转换在这里有效
      const wchar_t *ptr = input;
      // 处理空指针
      return std::wstring(ptr ? ptr : L"");
    }
    // 如果 T 是 std::string（假设 UTF-8）
    else if constexpr (std::is_same_v<DecayedT, std::string>)
    {
      // UTF-8 转换为 UTF-16
      return MultiByteToWide(input.c_str());
    }
    // 如果 T 是 const char*（假设 UTF-8）
    else if constexpr (std::is_convertible_v<DecayedT, const char *>)
    {
      // 隐式转换
      const char *ptr = input;
      // UTF-8 转换为 UTF-16
      return MultiByteToWide(ptr);
    }
    // 其他类型（例如数字）的回退 - 使用 wstringstream
    else
    {
      try
      {
        std::wstringstream wss;
        // 尝试流插入
        wss << input;
        if (wss.fail())
        {
          return L"[Stream Conversion Failed]";
        }
        return wss.str();
      }
      catch (...)
      {
        // 捕获流插入期间的潜在异常
        return L"[Conversion Exception]";
      }
    }
  }

  /**
   * @brief: 尝试重新打开日志文件，如果它已关闭或失败。
   * @return {bool} 如果文件打开且准备就绪，返回 true，否则返回 false。
   */
  static bool ReopenLogFile();

  // 防止实例化 - 静态类
  Logging() = delete;
  Logging(const Logging &) = delete;
  Logging &operator=(const Logging &) = delete;
};