/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-15 15:00:55
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-29 16:10:02
 * @FilePath: \GameOptimizerPro\src\main.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "log/logging.h"
#include "ui/tray_app.h"

int main(int argc, char *argv[])
{
    // 检查是否为管理员权限
    if (!isAdmin())
    {
        // 处理非管理员权限的情况
        // MessageBoxW(L"Restart as administrator.");
        requestAdminPrivileges();
        return 1;
    }

    // 初始化日志 Logging 是静态类, 初始化后其他类可以调用
    if (!Logging::initialize(L"logs/game_optimizer.log"))
    {
        // 处理日志初始化失败的情况，可能无法记录后续错误
        MessageBoxW(NULL, L"日志系统初始化失败!", L"严重错误", MB_ICONERROR | MB_OK);
        return 2;
    }
    LOG_INFO(L"游戏优化工具箱启动");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "GameOptimizerPro_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            app.installTranslator(&translator);
            break;
        }
    }

    // 创建托盘程序
    std::unique_ptr<TrayApp> trayApp = std::make_unique<TrayApp>(nullptr);

    // 退出程序时清理资源
    QCoreApplication::connect(qApp, &QCoreApplication::aboutToQuit, trayApp.get(), &TrayApp::cleanUp);

    return app.exec();
}
