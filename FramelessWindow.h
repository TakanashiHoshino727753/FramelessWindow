#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QDialog>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QSettings>
#include <QTimer>
#include <QStyle>
#include <QMenu>
#include <QAction>

// Windows 平台原生支持
#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

// Linux 平台原生支持
#ifdef Q_OS_LINUX
#include <xcb/xcb.h>
#endif

/**
 * @class FramelessWindow
 * @brief 无边框窗口基类（支持拖拽、缩放、背景、透明度、标题栏自定义、配置记忆化）
 */
class FramelessWindow : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param configPath 配置文件路径
     * @param parent 父窗口
     */
    explicit FramelessWindow(const QString &configPath, QWidget *parent = nullptr);
    ~FramelessWindow() override;

    // ====================== 枚举类型 ======================
    enum TitleMode { NoneTitle, TextTitle, PictureTitle };
    enum TitlePlace { Left, Middle };
    enum BGMode { NoneBG, PictureBG, ColorBG };
    enum BGGraphicStrategy { Stretch, Fill, Adapt };
    enum CloseAction { ExitProgram, MinimizeToTray };

    // ====================== 标题栏操作 ======================
    void setTitleMode(TitleMode mode);
    void setTitleColor(const QString &css);
    void setTitleText(const QString &text);
    void setTitleFont(const QString &style);
    void setTitlePic(const QString &filePath);
    void setTitlePlace(TitlePlace place);
    void addTitleWidget(QWidget *widget);
    void setTitleBarColor(const QString &color);

    // ====================== 背景操作 ======================
    void setBgMode(BGMode mode);
    void setBgColor(const QString &css);
    void setBgPic(const QString &filePath);
    void setBgPicStrategy(BGGraphicStrategy strategy);

    // ====================== 主内容区域 ======================
    void addMainWidget(QWidget *widget);
    void setMainAlpha(int alpha);

    // ======================= 托盘图标 =======================
    void setTrayIcon(const QString &filePath);

    // ====================== 配置记忆化 ======================
    void loadConfig();
    void saveConfig();

protected:
    // 原生事件（窗口缩放）
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

    // 窗口大小变化
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_clicked_min_btn();  // 最小化
    void on_clicked_max_btn();  // 最大化/还原
    void on_clicked_close_btn();// 关闭（支持记忆）
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void slotTrayRestore();
    void slotTrayQuit();

private:
    // ====================== 内部工具函数 ======================
    void updateTitleAlign(); // 更新标题对齐方式

    // ====================== 标题栏组件 ======================
    QWidget *titlebar;
    QWidget *title;
    QLabel *titleText;
    QLabel *titlePic;
    QWidget *titleWidget;
    QHBoxLayout *titleLayout;

    // ====================== 窗口按钮 ======================
    QWidget *windowCtrlBtns;
    QPushButton *min, *max, *close;

    // ====================== 背景组件 ======================
    QLabel *bgPicture;
    QString m_bgColor;

    // ====================== 主内容区域 ======================
    QWidget *mainWidget;
    QGraphicsOpacityEffect *m_mainOpacity;

    // ====================== 系统托盘 ======================
    QSystemTrayIcon *m_trayIcon;
    QMenu           *m_trayMenu;
    QAction         *m_actRestore;
    QAction         *m_actQuit;
    QString         m_trayIconPath;


    // ====================== 配置记忆变量 ======================
    BGMode m_bgMode;
    BGGraphicStrategy m_bgStrategy;
    QString m_bgPicPath;
    QString m_bgColorValue;

    TitleMode m_titleMode;
    TitlePlace m_titlePlace;
    QString m_titleTextValue;
    QString m_titleColorValue;
    QString m_titlePicPath;
    QString m_titleBarColor;

    CloseAction m_closeAction;

    // ====================== 配置文件路径 ======================
    QString m_configPath;

    // ====================== 鼠标拖动 ======================
    bool m_isDrag;
    QPoint m_dragPos;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};