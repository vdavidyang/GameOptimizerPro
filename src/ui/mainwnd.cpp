/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-15 14:58:54
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-29 01:24:06
 * @FilePath: \GameOptimizerPro\src\ui\mainwnd.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "ui/mainwnd.h"
#include "./ui_mainwnd.h"

MainWnd::MainWnd(QWidget *parent, QSystemTrayIcon *trayIcon)
    : QWidget(parent), m_trayIcon(trayIcon)
{
    try
    {
        m_mainWindow = std::make_unique<Ui::MainWnd>();
        m_mainWindow->setupUi(this);
        m_application = std::make_unique<Application>(m_configPath, trayIcon);
        initUI();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("初始化主窗口失败: " + std::string(e.what()));
    }
}

MainWnd::~MainWnd()
{
    // delete ui;
}

void MainWnd::initUI()
{
    // 设置窗口标题
    QWidget::setWindowTitle(QStringLiteral("鱼腥味的优化工具箱 V") + QString::fromUtf8(PROJECT_VERSION));

    // 设置表头均分宽度（Stretch 模式）
    // m_mainWindow->tableWidget_Game->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 禁止用户调整列宽
    m_mainWindow->tableWidget_GameProcess->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_mainWindow->tableWidget_GameProcess->horizontalHeader()->setSectionsMovable(false);
    // 设置每列宽度比例 游戏名称列占70%，优化状态列占30%
    int totalWidth = m_mainWindow->tableWidget_GameProcess->width();
    m_mainWindow->tableWidget_GameProcess->setColumnWidth(0, totalWidth * 0.7);
    m_mainWindow->tableWidget_GameProcess->setColumnWidth(1, totalWidth * 0.3);

    // 从配置文件加载游戏进程
    loadGameProcessFromConfig(m_application->getCurrentConfig().processConfig.gameProcessList);

    // 初始化开关按钮的映射
    m_switchButtonMap[m_mainWindow->switchButton_SetAutoStartup] = [this](bool checked)
    { return m_application->setAutoStartup(checked); };
    m_switchButtonMap[m_mainWindow->switchButton_SetAutoLimitAntiCheat] = [this](bool checked)
    { return m_application->setAutoLimitAntiCheat(checked); };
    m_switchButtonMap[m_mainWindow->switchButton_SetGameOptimizePowerPlan] = [this](bool checked)
    { return m_application->setGameOptimizePowerPlan(checked); };
    m_switchButtonMap[m_mainWindow->switchButton_SetPowerPlanLock] = [this](bool checked)
    { return m_application->setPowerPlanLock(checked); };
    m_switchButtonMap[m_mainWindow->switchButton_SetLimitBackgroundActivity] = [this](bool checked)
    { return m_application->setBackgroundActivityLimit(checked); };
    m_switchButtonMap[m_mainWindow->switchButton_SetNetworkDelayOptimization] = [this](bool checked)
    { return m_application->setOptimizeNetworkDelay(checked); };
    m_switchButtonMap[m_mainWindow->switchButton_SetSystemSchedulerOptimization] = [this](bool checked)
    { return m_application->setSystemSchedulerOptimization(checked); };
    m_switchButtonMap[m_mainWindow->switchButton_SetSystemServiceOptimization] = [this](bool checked)
    { return m_application->setSystemServiceOptimization(checked); };

    // 设置开关按钮的初始状态
    m_mainWindow->switchButton_SetAutoStartup->setChecked(m_application->getCurrentConfig().optimismConfig.autoStartUp);

    // 如果自动限制反作弊被启用, 则启动自动限制反作弊
    if (m_application->getCurrentConfig().optimismConfig.autoLimitAntiCheat)
    {
        m_application->setAutoLimitAntiCheat(true);
    }
    m_mainWindow->switchButton_SetAutoLimitAntiCheat->setChecked(m_application->getCurrentConfig().optimismConfig.autoLimitAntiCheat);

    m_mainWindow->switchButton_SetGameOptimizePowerPlan->setChecked(m_application->getCurrentConfig().optimismConfig.powerPlan.optimizePowerPlan);
    m_mainWindow->switchButton_SetPowerPlanLock->setChecked(m_application->getCurrentConfig().optimismConfig.powerPlan.lockPowerPlan);
    m_mainWindow->switchButton_SetLimitBackgroundActivity->setChecked(m_application->getCurrentConfig().optimismConfig.limitBackgroundActivity);
    m_mainWindow->switchButton_SetNetworkDelayOptimization->setChecked(m_application->getCurrentConfig().optimismConfig.optimizeNetworkDelay);
    m_mainWindow->switchButton_SetSystemSchedulerOptimization->setChecked(m_application->getCurrentConfig().optimismConfig.optimizeSystemScheduling);
    m_mainWindow->switchButton_SetSystemServiceOptimization->setChecked(m_application->getCurrentConfig().optimismConfig.optimizeSystemService);

    // 连接开关按钮的点击信号到槽函数
    connect(m_mainWindow->switchButton_SetAutoStartup, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
    connect(m_mainWindow->switchButton_SetAutoLimitAntiCheat, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
    connect(m_mainWindow->switchButton_SetGameOptimizePowerPlan, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
    connect(m_mainWindow->switchButton_SetPowerPlanLock, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
    connect(m_mainWindow->switchButton_SetLimitBackgroundActivity, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
    connect(m_mainWindow->switchButton_SetNetworkDelayOptimization, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
    connect(m_mainWindow->switchButton_SetSystemSchedulerOptimization, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
    connect(m_mainWindow->switchButton_SetSystemServiceOptimization, &SwitchButton::clicked, this, &MainWnd::on_switchButton_clicked);
}

void MainWnd::on_switchButton_clicked()
{
    // 获取当前点击的按钮
    SwitchButton *currentButton = qobject_cast<SwitchButton *>(sender());
    if (!currentButton)
    {
        return;
    }

    // 调用对应的设置方法, 如果设置失败, 则恢复按钮状态, 并弹出警告框
    if (!m_switchButtonMap[currentButton](currentButton->isChecked()))
    {
        currentButton->setChecked(!currentButton->isChecked());
        QMessageBox::warning(this, "警告", "设置失败! 请查看日志并反馈!");
    }
}

void MainWnd::loadGameProcessFromConfig(const std::vector<ProcessInfo> &gameProcessList)
{
    // 清空表
    m_mainWindow->tableWidget_GameProcess->clearContents();
    m_mainWindow->tableWidget_GameProcess->setRowCount(0);

    for (const auto &gameProcess : gameProcessList)
    {
        insertRowtoTableWidget(m_mainWindow->tableWidget_GameProcess, QString::fromStdString(gameProcess.name), gameProcess.status);
    }
}

void MainWnd::insertRowtoTableWidget(QTableWidget *tableWidget, const QString &gameName, const bool &status)
{
    // 获取当前行数
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);

    // 设置前两列的文本 游戏名称
    tableWidget->setItem(row, 0, new QTableWidgetItem(gameName));
    // 设置文本 水平 + 垂直 居中
    tableWidget->item(row, 0)->setTextAlignment(Qt::AlignCenter);

    // 创建一个容器 Widget 和布局，将按钮放在布局中居中
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);

    // 在第三列（优化状态）添加开关按钮
    SwitchButton *switchButton = new SwitchButton(tableWidget);
    tableWidget->setCellWidget(row, 1, widget);

    // 设置按钮状态
    switchButton->setChecked(status);

    // 向布局中添加按钮
    layout->addWidget(switchButton);
    layout->setAlignment(Qt::AlignCenter);  // 按钮居中
    layout->setContentsMargins(0, 0, 0, 0); // 去除边距

    // 连接按钮的点击信号到槽函数
    connect(switchButton, &SwitchButton::clicked, [this, tableWidget, row, switchButton]()
            {
                // 点击按钮后启用或关闭游戏进行优化
                if (!m_application->setOptimizeGameProcess(row, switchButton->isChecked()))
                {
                    QString gameName = tableWidget->item(row, 0)->text();
                    QMessageBox::warning(this, "警告", "设置游戏优化失败: " + gameName);
                    LOG_ERROR("设置游戏优化失败: " + gameName.toStdString());
                } });
}

void MainWnd::cleanUp()
{
    // 清理资源
    // 关闭反作弊进程监控
    m_application->setAutoLimitAntiCheat(false, true);
    // 设置按钮状态为假
    // m_mainWindow->switchButton_SetAutoLimitAntiCheat->setChecked(false);
}