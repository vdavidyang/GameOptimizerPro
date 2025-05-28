
#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QtUiPlugin/qdesignerexportwidget.h>
#include <QAbstractButton>
#include <QPropertyAnimation>
#include <QColor>

#ifndef SWITCHBUTTON_EXPORT
#   if defined(SWITCHBUTTON_STATIC)
#       define SWITCHBUTTON_EXPORT
#   else
#       define SWITCHBUTTON_EXPORT QDESIGNER_WIDGET_EXPORT
#   endif
#endif

class SWITCHBUTTON_EXPORT SwitchButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(QColor onColor READ onColor WRITE setOnColor DESIGNABLE true)
    Q_PROPERTY(QColor offColor READ offColor WRITE setOffColor DESIGNABLE true)
    Q_PROPERTY(QColor thumbColor READ thumbColor WRITE setThumbColor DESIGNABLE true)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration DESIGNABLE true)
    Q_PROPERTY(qreal currentThumbPos READ currentThumbPos WRITE setCurrentThumbPos)
    Q_PROPERTY(qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity)

public:
    explicit SwitchButton(QWidget *parent = nullptr);
    virtual ~SwitchButton() override = default;

    // 尺寸建议
    QSize sizeHint() const override;

    // 颜色属性
    QColor onColor() const;
    void setOnColor(const QColor &color);

    QColor offColor() const;
    void setOffColor(const QColor &color);

    QColor thumbColor() const;
    void setThumbColor(const QColor &color);

    int animationDuration() const;
    void setAnimationDuration(int duration);

    // 动画属性访问器
    qreal currentThumbPos() const;
    void setCurrentThumbPos(qreal pos);

    qreal currentOpacity() const;
    void setCurrentOpacity(qreal opacity);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY(SwitchButton)

    void setupAnimation();

    QColor m_onColor = QColor(46, 204, 113);        // 默认开启颜色
    QColor m_offColor = QColor(188, 188, 188);      // 默认关闭颜色
    QColor m_thumbColor = QColor(255, 255, 255);    // 默认滑块颜色
    QColor m_disabledColor = QColor(179, 182, 183); // 禁用状态颜色

    int m_animationDuration = 300; // 动画持续时间(ms) - 调整为更流畅的值

    // 用于动画的状态变量
    qreal m_currentThumbPos = 0.0; // 当前滑块位置 (0.0 到 1.0)
    qreal m_currentOpacity = 1.0;  // 当前不透明度 (用于悬停)

    QPropertyAnimation *m_thumbPositionAnimation = nullptr; // 初始化为 nullptr
    QPropertyAnimation *m_opacityAnimation = nullptr;       // 初始化为 nullptr

    bool m_isAnimatingFromClick = false; // 添加标志位，阻止 toggled 信号处理程序在点击时干扰动画
};

#endif // SWITCHBUTTON_H
