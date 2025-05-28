/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-23 19:26:23
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 00:37:19
 * @FilePath: \GameOptimizerPro\include\ui\tray_app.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>
#include <windows.h>
#include <shlobj.h>

#include "ui/mainwnd.h"
#include "log/logging.h"
#include "utils/system_utils.h"

/**
 * @enum ActionType
 * @brief 枚举类型，定义托盘图标菜单项的类型
 *
 */
enum class ActionType
{
  ShowMain,
  OpenLog,
  About,
  Quit
};

/**
 * @class TrayAction
 * @brief 封装托盘图标菜单项的信息
 */
class TrayAction
{
public:
  ActionType type;
  std::unique_ptr<QAction> action;
  std::function<void()> callback;

  // 移动构造函数
  TrayAction(ActionType type, std::unique_ptr<QAction> action, std::function<void()> callback)
      : type(std::move(type)), action(std::move(action)), callback(std::move(callback)) {}

  // 删除拷贝构造函数和拷贝赋值
  TrayAction(const TrayAction &) = delete;
  TrayAction &operator=(const TrayAction &) = delete;

  // 默认移动操作
  TrayAction(TrayAction &&) = default;
  TrayAction &operator=(TrayAction &&) = default;
};

/**
 * @class TrayApp
 * @brief 系统托盘图标管理类
 *
 * 负责管理系统托盘图标及其相关功能，包括：
 * 创建托盘图标和右键菜单
 * 处理托盘图标点击事件
 * 提供显示主窗口、查看日志、关于和退出功能
 */
class TrayApp : public QObject
{

  Q_OBJECT

public:
  /**
   * @brief 构造函数
   * @param parent 父对象指针
   */
  explicit TrayApp(QObject *parent = nullptr);

  /**
   * @brief 程序退出前的清理工作
   */
  void cleanUp();

  /**
   * @brief 显示托盘消息
   *
   * @param title 标题
   * @param message 消息内容
   */
  void ShowTrayMessage(const QString &title, const QString &message);

private slots:

  /**
   * @brief 显示主窗口
   *
   * 该函数首先检查主窗口是否已经创建，如果已经创建，则显示并激活主窗口；
   */
  void showMainWindow();

  /**
   * @brief 打开日志文件或日志文件所在目录
   *
   * 该函数首先检查日志文件是否存在。如果存在，则用默认程序打开该日志文件；
   * 如果不存在，则打开日志文件所在的目录。
   *
   * @param logFilePath 日志文件的完整路径
   */
  void openLog();

  /**
   * @brief 显示关于对话框
   *
   * 该函数显示关于对话框。
   */
  void showAbout();

  /**
   * @brief 退出应用
   *
   * 该函数调用QApplication的quit()函数退出应用。
   */
  void quit();

private:
  std::unique_ptr<MainWnd> m_mainWindow;       ///< 主窗口指针
  std::unique_ptr<QSystemTrayIcon> m_trayIcon; ///< 系统托盘图标指针
  std::unique_ptr<QMenu> m_trayMenu;           ///< 托盘右键菜单指针
  ///< 托盘右键菜单项指针
  std::vector<TrayAction> m_actions;

  /**
   * @brief 初始化托盘程序
   */
  void initTrayApp();

  /**
   * @brief 获取菜单项名称
   * @param type 菜单项类型
   */
  QString getActionName(ActionType type);
};