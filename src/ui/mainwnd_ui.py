# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'mainwnd.ui'
##
## Created by: Qt User Interface Compiler version 6.6.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QAbstractScrollArea, QApplication, QGridLayout, QHeaderView,
    QLabel, QSizePolicy, QTableWidget, QTableWidgetItem,
    QWidget)

from ui.components.switchbutton import SwitchButton
import res_rc

class Ui_MainWnd(object):
    def setupUi(self, MainWnd):
        if not MainWnd.objectName():
            MainWnd.setObjectName(u"MainWnd")
        MainWnd.resize(720, 480)
        MainWnd.setMinimumSize(QSize(720, 480))
        MainWnd.setMaximumSize(QSize(720, 480))
        font = QFont()
        font.setPointSize(10)
        MainWnd.setFont(font)
        icon = QIcon()
        icon.addFile(u":/icons/app.ico", QSize(), QIcon.Normal, QIcon.Off)
        MainWnd.setWindowIcon(icon)
        self.widget_functions = QWidget(MainWnd)
        self.widget_functions.setObjectName(u"widget_functions")
        self.widget_functions.setGeometry(QRect(0, 10, 720, 461))
        self.gridLayoutWidget = QWidget(self.widget_functions)
        self.gridLayoutWidget.setObjectName(u"gridLayoutWidget")
        self.gridLayoutWidget.setGeometry(QRect(400, 10, 291, 341))
        self.gridLayout = QGridLayout(self.gridLayoutWidget)
        self.gridLayout.setObjectName(u"gridLayout")
        self.gridLayout.setContentsMargins(0, 0, 0, 0)
        self.label_BackgroundActivityLimit = QLabel(self.gridLayoutWidget)
        self.label_BackgroundActivityLimit.setObjectName(u"label_BackgroundActivityLimit")
        font1 = QFont()
        font1.setPointSize(10)
        font1.setBold(False)
        self.label_BackgroundActivityLimit.setFont(font1)
        self.label_BackgroundActivityLimit.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_BackgroundActivityLimit, 7, 0, 1, 1)

        self.label_SystemServiceOptimization = QLabel(self.gridLayoutWidget)
        self.label_SystemServiceOptimization.setObjectName(u"label_SystemServiceOptimization")
        self.label_SystemServiceOptimization.setFont(font1)
        self.label_SystemServiceOptimization.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_SystemServiceOptimization, 10, 0, 1, 1)

        self.label_GameOptimizePowerPlan = QLabel(self.gridLayoutWidget)
        self.label_GameOptimizePowerPlan.setObjectName(u"label_GameOptimizePowerPlan")
        self.label_GameOptimizePowerPlan.setFont(font1)
        self.label_GameOptimizePowerPlan.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_GameOptimizePowerPlan, 5, 0, 1, 1)

        self.label_PowerPlanLock = QLabel(self.gridLayoutWidget)
        self.label_PowerPlanLock.setObjectName(u"label_PowerPlanLock")
        self.label_PowerPlanLock.setFont(font1)
        self.label_PowerPlanLock.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_PowerPlanLock, 6, 0, 1, 1)

        self.label_AutoLimitAntiCheat = QLabel(self.gridLayoutWidget)
        self.label_AutoLimitAntiCheat.setObjectName(u"label_AutoLimitAntiCheat")
        self.label_AutoLimitAntiCheat.setFont(font1)
        self.label_AutoLimitAntiCheat.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_AutoLimitAntiCheat, 4, 0, 1, 1)

        self.label_AutoStartup = QLabel(self.gridLayoutWidget)
        self.label_AutoStartup.setObjectName(u"label_AutoStartup")
        self.label_AutoStartup.setFont(font1)
        self.label_AutoStartup.setTextFormat(Qt.TextFormat.AutoText)
        self.label_AutoStartup.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_AutoStartup, 0, 0, 1, 1)

        self.label_SystemSchedulerOptimization = QLabel(self.gridLayoutWidget)
        self.label_SystemSchedulerOptimization.setObjectName(u"label_SystemSchedulerOptimization")
        self.label_SystemSchedulerOptimization.setFont(font1)
        self.label_SystemSchedulerOptimization.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_SystemSchedulerOptimization, 9, 0, 1, 1)

        self.switchButton_SetSystemServiceOptimization = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetSystemServiceOptimization.setObjectName(u"switchButton_SetSystemServiceOptimization")

        self.gridLayout.addWidget(self.switchButton_SetSystemServiceOptimization, 10, 1, 1, 1)

        self.switchButton_SetPowerPlanLock = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetPowerPlanLock.setObjectName(u"switchButton_SetPowerPlanLock")
        self.switchButton_SetPowerPlanLock.setEnabled(False)

        self.gridLayout.addWidget(self.switchButton_SetPowerPlanLock, 6, 1, 1, 1)

        self.switchButton_SetAutoStartup = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetAutoStartup.setObjectName(u"switchButton_SetAutoStartup")

        self.gridLayout.addWidget(self.switchButton_SetAutoStartup, 0, 1, 1, 1)

        self.switchButton_SetLimitBackgroundActivity = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetLimitBackgroundActivity.setObjectName(u"switchButton_SetLimitBackgroundActivity")

        self.gridLayout.addWidget(self.switchButton_SetLimitBackgroundActivity, 7, 1, 1, 1)

        self.switchButton_SetAutoLimitAntiCheat = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetAutoLimitAntiCheat.setObjectName(u"switchButton_SetAutoLimitAntiCheat")

        self.gridLayout.addWidget(self.switchButton_SetAutoLimitAntiCheat, 4, 1, 1, 1)

        self.switchButton_SetSystemSchedulerOptimization = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetSystemSchedulerOptimization.setObjectName(u"switchButton_SetSystemSchedulerOptimization")

        self.gridLayout.addWidget(self.switchButton_SetSystemSchedulerOptimization, 9, 1, 1, 1)

        self.switchButton_SetGameOptimizePowerPlan = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetGameOptimizePowerPlan.setObjectName(u"switchButton_SetGameOptimizePowerPlan")

        self.gridLayout.addWidget(self.switchButton_SetGameOptimizePowerPlan, 5, 1, 1, 1)

        self.label_NetWorkDelayOptimization = QLabel(self.gridLayoutWidget)
        self.label_NetWorkDelayOptimization.setObjectName(u"label_NetWorkDelayOptimization")
        self.label_NetWorkDelayOptimization.setFont(font1)
        self.label_NetWorkDelayOptimization.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.label_NetWorkDelayOptimization, 8, 0, 1, 1)

        self.switchButton_SetNetworkDelayOptimization = SwitchButton(self.gridLayoutWidget)
        self.switchButton_SetNetworkDelayOptimization.setObjectName(u"switchButton_SetNetworkDelayOptimization")

        self.gridLayout.addWidget(self.switchButton_SetNetworkDelayOptimization, 8, 1, 1, 1)

        self.tableWidget_GameProcess = QTableWidget(self.widget_functions)
        if (self.tableWidget_GameProcess.columnCount() < 2):
            self.tableWidget_GameProcess.setColumnCount(2)
        __qtablewidgetitem = QTableWidgetItem()
        self.tableWidget_GameProcess.setHorizontalHeaderItem(0, __qtablewidgetitem)
        __qtablewidgetitem1 = QTableWidgetItem()
        self.tableWidget_GameProcess.setHorizontalHeaderItem(1, __qtablewidgetitem1)
        self.tableWidget_GameProcess.setObjectName(u"tableWidget_GameProcess")
        self.tableWidget_GameProcess.setGeometry(QRect(10, 10, 380, 451))
        self.tableWidget_GameProcess.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        self.tableWidget_GameProcess.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.tableWidget_GameProcess.setSizeAdjustPolicy(QAbstractScrollArea.SizeAdjustPolicy.AdjustIgnored)
        self.tableWidget_GameProcess.setRowCount(0)
        self.tableWidget_GameProcess.setColumnCount(2)
        self.tableWidget_GameProcess.horizontalHeader().setVisible(True)
        self.tableWidget_GameProcess.horizontalHeader().setStretchLastSection(False)
        self.tableWidget_GameProcess.verticalHeader().setVisible(False)
        self.tableWidget_GameProcess.verticalHeader().setCascadingSectionResizes(False)
        self.tableWidget_GameProcess.verticalHeader().setMinimumSectionSize(24)
        self.tableWidget_GameProcess.verticalHeader().setStretchLastSection(False)
        self.gridLayoutWidget_2 = QWidget(self.widget_functions)
        self.gridLayoutWidget_2.setObjectName(u"gridLayoutWidget_2")
        self.gridLayoutWidget_2.setGeometry(QRect(399, 359, 311, 140))
        self.gridLayout_2 = QGridLayout(self.gridLayoutWidget_2)
        self.gridLayout_2.setObjectName(u"gridLayout_2")
        self.gridLayout_2.setContentsMargins(0, 0, 0, 0)
        self.label_note = QLabel(self.widget_functions)
        self.label_note.setObjectName(u"label_note")
        self.label_note.setGeometry(QRect(400, 350, 309, 138))
        self.label_note.setFont(font)
        self.label_note.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.label_note.setOpenExternalLinks(True)

        self.retranslateUi(MainWnd)

        QMetaObject.connectSlotsByName(MainWnd)
    # setupUi

    def retranslateUi(self, MainWnd):
        MainWnd.setWindowTitle(QCoreApplication.translate("MainWnd", u"\u9c7c\u8165\u5473\u7684\u6e38\u620f\u4f18\u5316\u5de5\u5177\u7bb1 V0.0.0.0", None))
        self.label_BackgroundActivityLimit.setText(QCoreApplication.translate("MainWnd", u"\u9650\u5236\u540e\u53f0\u6d3b\u52a8", None))
        self.label_SystemServiceOptimization.setText(QCoreApplication.translate("MainWnd", u"\u7cfb\u7edf\u670d\u52a1\u4f18\u5316", None))
        self.label_GameOptimizePowerPlan.setText(QCoreApplication.translate("MainWnd", u"\u6e38\u620f\u7535\u6e90\u4f18\u5316", None))
        self.label_PowerPlanLock.setText(QCoreApplication.translate("MainWnd", u"\u9501\u5b9a\u7535\u6e90\u8ba1\u5212\uff08\u6682\u4e0d\u53ef\u7528\uff09", None))
        self.label_AutoLimitAntiCheat.setText(QCoreApplication.translate("MainWnd", u"\u81ea\u52a8\u9650\u5236\u53cd\u4f5c\u5f0a", None))
        self.label_AutoStartup.setText(QCoreApplication.translate("MainWnd", u"\u5f00\u673a\u81ea\u542f\u52a8", None))
        self.label_SystemSchedulerOptimization.setText(QCoreApplication.translate("MainWnd", u"\u7cfb\u7edf\u8c03\u5ea6\u4f18\u5316", None))
        self.label_NetWorkDelayOptimization.setText(QCoreApplication.translate("MainWnd", u"\u7f51\u7edc\u5ef6\u8fdf\u4f18\u5316", None))
        ___qtablewidgetitem = self.tableWidget_GameProcess.horizontalHeaderItem(0)
        ___qtablewidgetitem.setText(QCoreApplication.translate("MainWnd", u"\u6e38\u620f\u540d\u79f0", None));
        ___qtablewidgetitem1 = self.tableWidget_GameProcess.horizontalHeaderItem(1)
        ___qtablewidgetitem1.setText(QCoreApplication.translate("MainWnd", u"\u4f18\u5316\u72b6\u6001", None));
#if QT_CONFIG(tooltip)
        self.label_note.setToolTip(QCoreApplication.translate("MainWnd", u"<html><head/><body><p align=\"center\"><img src=\":/imgs/qr_code.png\" width=\"800\" height=\"300\"/></p></body></html>", None))
#endif // QT_CONFIG(tooltip)
        self.label_note.setText(QCoreApplication.translate("MainWnd", u"<html><head/><body><p>\u672c\u8f6f\u4ef6<a href=\"https://www.baidu.com\"><span style=\" text-decoration: underline; color:#004275;\">Github\u5f00\u6e90\u5730\u5740</span></a>\uff0c\u559c\u6b22\u7684\u7ed9\u4e2aStar\u5427\uff01\uff01\uff01</p><p>\u5927\u5b66\u751f\u4e3a\u7231\u53d1\u7535\u4e0d\u6613\uff0c<a href=\"https://www.baidu.com\"><span style=\" text-decoration: underline; color:#004275;\">\u8bf7\u6211\u559d\u74f6\u6c34\u5427~</span></a></p><p>\u6709\u9700\u8981\u978b\u5b50\u7684\u5144\u5f1f\u53ef\u4ee5\u6dfb\u52a0\u5fae\u4fe1\uff1a<a href=\"https://www.baidu.com\"><span style=\" text-decoration: underline; color:#004275;\">Mrmuscle12138</span></a><br/></p></body></html>", None))
    # retranslateUi

