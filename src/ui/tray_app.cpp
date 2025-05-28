/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-23 19:25:31
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 23:38:53
 * @FilePath: \GameOptimizerPro\src\ui\tray_app.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "ui/tray_app.h"

TrayApp::TrayApp(QObject *parent)
    : QObject(parent),
      m_mainWindow(nullptr),
      m_trayIcon(nullptr),
      m_trayMenu(nullptr)
{
  try
  {
    // 使用 emplace_back 初始化
    m_actions.emplace_back(ActionType::ShowMain, nullptr, [this]()
                           { showMainWindow(); });
    m_actions.emplace_back(ActionType::OpenLog, nullptr, [this]()
                           { openLog(); });
    m_actions.emplace_back(ActionType::About, nullptr, [this]()
                           { showAbout(); });
    m_actions.emplace_back(ActionType::Quit, nullptr, [this]()
                           { quit(); });

    initTrayApp();
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("TrayApp constructor failed: " + std::string(e.what()));
  }
}

void TrayApp::initTrayApp()
{
  try
  {
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
      LOG_ERROR("System tray not available");
      return;
    }

    // 创建系统托盘图标
    m_trayIcon = std::make_unique<QSystemTrayIcon>(this);
    if (!m_trayIcon)
    {
      LOG_ERROR("Failed to create tray icon");
      return;
    }
    m_trayIcon->setIcon(QIcon(":/icons/app.png"));
    m_trayIcon->setToolTip("鱼腥味的游戏优化工具箱");

    // 创建托盘菜单
    m_trayMenu = std::make_unique<QMenu>();
    if (!m_trayMenu)
    {
      LOG_ERROR("Failed to create tray menu");
      return;
    }

    // 添加菜单项
    for (auto &action : m_actions)
    {
      // 创建菜单项
      action.action = std::make_unique<QAction>(getActionName(action.type), this);
      // 连接信号槽
      connect(action.action.get(), &QAction::triggered, this, action.callback);
      if (action.type == ActionType::Quit)
      {
        m_trayMenu->addSeparator();
      }
      // 添加菜单项到托盘菜单
      m_trayMenu->addAction(action.action.get());
    }

    // 设置托盘菜单
    m_trayIcon->setContextMenu(m_trayMenu.get());

    // 连接托盘图标点击事件
    QObject::connect(m_trayIcon.get(), &QSystemTrayIcon::activated,
                     [this](QSystemTrayIcon::ActivationReason reason)
                     {
                       if (reason == QSystemTrayIcon::DoubleClick)
                       {
                         showMainWindow();
                       }
                     });

    // 显示托盘图标
    m_trayIcon->show();

    // 创建主窗口但不显示
    m_mainWindow = std::make_unique<MainWnd>(nullptr, m_trayIcon.get());
    // 默认隐藏主窗口
    m_mainWindow->hide();
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("TrayApp init failed: " + std::string(e.what()));
  }
}

QString TrayApp::getActionName(ActionType type)
{
  switch (type)
  {
  case ActionType::ShowMain:
    return "显示主窗口";
  case ActionType::OpenLog:
    return "打开日志";
  case ActionType::About:
    return "关于";
  case ActionType::Quit:
    return "退出";
  default:
    return "未知";
  }
}

void TrayApp::showMainWindow()
{
  if (m_mainWindow)
  {
    m_mainWindow->show();
    m_mainWindow->activateWindow();
  }
}

void TrayApp::openLog()
{
  // 获取当前程序所在目录
  wchar_t currentPath[MAX_PATH];
  GetModuleFileNameW(NULL, currentPath, MAX_PATH);
  // 移除文件名，保留目录
  PathRemoveFileSpecW(currentPath);
  std::wstring logFilePath = std::wstring(currentPath) + L"/" + Logging::getLogFilePath();
  LOG_INFO(L"Opening log file: " + logFilePath);
  DWORD fileAttributes = GetFileAttributesW(logFilePath.c_str());
  if (fileAttributes != INVALID_FILE_ATTRIBUTES &&
      !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY))
  {
    // 文件存在，直接打开文件
    ShellExecuteW(NULL, L"open", logFilePath.c_str(), NULL, NULL, SW_SHOW);
  }
  else
  {
    // 文件不存在，打开所在目录
    std::wstring directoryPath;
    size_t lastSlashPos = logFilePath.find_last_of(L"\\/");

    if (lastSlashPos != std::wstring::npos)
    {
      directoryPath = logFilePath.substr(0, lastSlashPos);
    }
    else
    {
      // 如果没有找到路径分隔符，使用当前目录
      wchar_t currentDir[MAX_PATH];
      GetCurrentDirectoryW(MAX_PATH, currentDir);
      directoryPath = currentDir;
    }

    // 使用资源管理器打开目录
    ShellExecuteW(NULL, L"explore", directoryPath.c_str(), NULL, NULL, SW_SHOW);
  }
}

void TrayApp::showAbout()
{
  MessageBoxW(nullptr,
              L"鱼腥味的游戏优化工具箱\n"
              L"Game Optimizer Pro\n"
              L"version: 1.0.0.0\n"
              L"© 2025 by 鱼腥味, All Rights Reserved.",
              L"关于",
              MB_OK | MB_ICONINFORMATION);
}

void TrayApp::quit()
{
  QApplication::quit();
}

void TrayApp::cleanUp()
{
  // 在应用退出前执行清理操作
  m_mainWindow->cleanUp();
}

void TrayApp::ShowTrayMessage(const QString &title, const QString &message)
{
  if (m_trayIcon)
  {
    m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 5000);
  }
}