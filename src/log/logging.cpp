/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:33:53
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-03 17:51:54
 * @FilePath: \GameOptimizerPro\src\logging\logging.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#include "log/logging.h"

// 定义静态变量
std::mutex Logging::m_mutex;
std::wofstream Logging::m_logFileStream;
bool Logging::m_isInitialized = false;
std::wstring Logging::m_logFilePath;

// 初始化日志系统
bool Logging::initialize(const std::wstring &logFilePath)
{
  if (m_isInitialized)
  {
    // 防止重复初始化
    LOG_WARN(L"Logging system already initialized.");
    return true;
  }

  try
  {
    // 提取目录路径
    std::filesystem::path pathObj(logFilePath);
    std::filesystem::path dirPath = pathObj.parent_path();

    // 检查目录是否存在，如果不存在则创建
    if (!dirPath.empty() && !std::filesystem::exists(dirPath))
    {
      if (!std::filesystem::create_directories(dirPath))
      {
        // 目录创建失败处理 (例如，记录到控制台或返回 false)
        std::wcerr << L"Failed to create log directory: " << dirPath << std::endl;
        MessageBoxW(NULL, L"无法创建日志目录", L"日志", MB_ICONINFORMATION | MB_OK);
        // 在这种情况下，我们可能无法写入日志文件本身来记录这个错误
        return false;
      }
    }

    // 设置日志文件路径
    m_logFilePath = logFilePath;

    // 直接使用 wstring 路径打开 (需要转换为窄字符)

    m_logFileStream.open(m_logFilePath.c_str(), std::ios::app | std::ios::out);

    if (!m_logFileStream.is_open())
    {
      // 文件打开失败处理
      std::wcerr << L"Failed to open log file: " << m_logFilePath << std::endl;
      MessageBoxW(NULL, L"无法打开日志文件", L"日志", MB_ICONINFORMATION | MB_OK);
      return false;
    }

    try
    {
      m_logFileStream.imbue(std::locale(""));
    }
    catch (const std::runtime_error &e)
    {
      // 如果设置locale失败，记录一个警告，但继续执行
      // 可以考虑回退到经典 "C" locale
      std::wcerr << L"Warning: Failed to set locale for log file. Error: "
                 << MultiByteToWide(e.what()) // 使用辅助函数转换异常消息
                 << L". Using default C locale." << std::endl;
      m_logFileStream.imbue(std::locale::classic());
    }

    m_isInitialized = true;
    LOG_INFO(L"Logging system initialized successfully. Log file: " + m_logFilePath);
    return true;
  }
  catch (const std::filesystem::filesystem_error &e)
  {
    std::wcerr << L"Filesystem error during logging initialization: " << e.what() << std::endl;
    return false;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Standard exception during logging initialization: " << e.what() << std::endl;
    return false;
  }
  catch (...)
  {
    std::cerr << "Unknown exception during logging initialization." << std::endl;
    return false;
  }
}

// 关闭日志系统
void Logging::shutdown()
{
  if (m_isInitialized && m_logFileStream.is_open())
  {
    LOG_INFO(L"Shutting down logging system.");
    m_logFileStream.close();
    m_isInitialized = false;
  }
}

bool Logging::ReopenLogFile()
{
  if (m_logFileStream.is_open())
  {
    return true; // 已打开
  }

  m_logFileStream.close(); // 确保首先完全关闭
  m_logFileStream.open(m_logFilePath, std::ios::app | std::ios::out);

  if (!m_logFileStream.is_open())
  {
    std::wcerr << L"Failed to reopen log file: " << m_logFilePath << std::endl;
    return false;
  }

  // 再次尝试设置区域设置
  try
  {
    m_logFileStream.imbue(std::locale(""));
  }
  catch (const std::runtime_error &)
  {
    std::wcerr << L"Warning: Failed to set system locale on reopen. Using C locale." << std::endl;
    // 回退
    m_logFileStream.imbue(std::locale::classic());
  }
  return true;
}
