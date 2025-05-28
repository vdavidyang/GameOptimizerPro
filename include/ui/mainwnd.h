/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-15 14:58:22
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-29 01:22:47
 * @FilePath: \GameOptimizerPro\include\ui\mainwnd.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QString>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QSystemTrayIcon>

#include "ui/components/switchbutton.h"
#include "log/logging.h"
#include "core/application.h"

#include "version.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWnd;
}
QT_END_NAMESPACE

class MainWnd : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief: 构造函数
     * @param QWidget *parent
     */
    explicit MainWnd(QWidget *parent = nullptr, QSystemTrayIcon *trayIcon = nullptr);
    ~MainWnd();

    /**
     * @brief 清理资源
     */
    void cleanUp();

private:
    std::unique_ptr<Ui::MainWnd> m_mainWindow{nullptr};
    std::unique_ptr<Application> m_application{nullptr};
    QSystemTrayIcon *m_trayIcon = nullptr;

    std::wstring m_configPath = L"config/config.json";

    // 创建一个map，保存所有需要设置的开关按钮和对应的设置方法
    std::map<SwitchButton *, std::function<bool(bool)>> m_switchButtonMap;

    void initUI();

    /**
     * @brief: 从配置文件加载游戏进程
     * @param std::vector<ProcessInfo> &gameProcesses
     */
    void loadGameProcessFromConfig(const std::vector<ProcessInfo> &gameProcesses);

    /**
     * @brief: 开关按钮点击事件
     */
    void on_switchButton_clicked();

    /**
     * @brief: 在表格中插入一行
     * @param QTableWidget *tableWidget
     * @param QString &gameName
     */
    void insertRowtoTableWidget(QTableWidget *tableWidget, const QString &gameName, const bool &status);

protected:
    /**
     * @brief 重写关闭事件，实现最小化到托盘而不是退出
     * @param event 关闭事件对象
     */
    void closeEvent(QCloseEvent *event) override
    {
        // 忽略关闭事件，改为隐藏窗口
        hide();
        event->ignore();
    }
};