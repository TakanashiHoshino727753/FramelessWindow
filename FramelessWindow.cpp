#include "FramelessWindow.h"
#include <QDir>
#include <QMetaEnum>
#include <QBuffer>
#include <QMenu>
#include <QDialog>
#include <QRadioButton>
#include <windows.h>
#include <windowsx.h>

FramelessWindow::FramelessWindow(QWidget *parent)
    : QWidget(parent)
    , m_savedTitleMode(text)
    , m_savedCloseAction(MinimizeToTray)
    , m_bgMode(None)
    , m_hasSetCloseAction(false)
    , m_settings(QDir::currentPath() + "/config.ini", QSettings::IniFormat)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_StyledBackground, true);

    titlebar = new QFrame(this);

    min = new QPushButton;
    max = new QPushButton;
    close = new QPushButton;
    min->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    max->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    min->setFixedSize(36,36);
    max->setFixedSize(36,36);
    close->setFixedSize(36,36);

    min->setStyleSheet("QPushButton{border:none; background:transparent;}");
    max->setStyleSheet("QPushButton{border:none; background:transparent;}");
    close->setStyleSheet("QPushButton{border:none; background:transparent;}");

    titletxt = new QLabel;
    titletxt->setFixedHeight(36);
    titletxt->setStyleSheet("color:white;background:transparent;font-size:14px;");
    titletxt->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    picLabel = new QLabel;
    picLabel->setFixedHeight(36);
    picLabel->setStyleSheet("background:transparent;");
    picLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    titleitems = new QWidget(this);
    mainitems = new QWidget(this);

    m_backgroundView = new QGraphicsView(this);
    m_backgroundView->setStyleSheet("border:none; background:transparent;");
    m_backgroundView->setScene(new QGraphicsScene(this));
    m_backgroundView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_backgroundView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_backgroundView->setRenderHint(QPainter::SmoothPixmapTransform);

    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setStyleSheet("border:none;");
    m_videoWidget->hide();

    m_mediaPlayer = new QMediaPlayer(this);
    QAudioOutput* audioOut = new QAudioOutput(this);
    audioOut->setVolume(1.0);
    m_mediaPlayer->setAudioOutput(audioOut);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    m_mediaPlayer->setLoops(-1);

    mainitems->setStyleSheet("background:transparent;");
    mainitems->setParent(this);
    mainitems->raise();

    windowctrlbtnslayout = new QHBoxLayout;
    windowctrlbtnslayout->setContentsMargins(0,0,10,0);
    windowctrlbtnslayout->setSpacing(8);
    windowctrlbtnslayout->addWidget(min);
    windowctrlbtnslayout->addWidget(max);
    windowctrlbtnslayout->addWidget(close);

    QHBoxLayout* barLayout = new QHBoxLayout(titlebar);
    barLayout->setContentsMargins(10,0,0,0);
    barLayout->setSpacing(10);
    barLayout->addWidget(picLabel);
    barLayout->addWidget(titletxt, 1);
    barLayout->addWidget(titleitems);
    barLayout->addLayout(windowctrlbtnslayout);

    titletxt->setVisible(true);
    picLabel->setVisible(false);

    connect(min,   &QPushButton::clicked, this, &FramelessWindow::on_clicked_min_btn);
    connect(max,   &QPushButton::clicked, this, &FramelessWindow::on_clicked_max_btn);
    connect(close, &QPushButton::clicked, this, &FramelessWindow::closeWindow);
    titlebar->installEventFilter(this);

    setupTrayIcon();
    loadConfig();
    setMinimumSize(320, 180);
    saveConfig();
}

void FramelessWindow::setTitleIconPath(const QString &path)
{
    m_titleIconPath = path;
    QPixmap pix(path);
    m_savedTitleMode = singlepic;
    settilemode(m_savedTitleMode);
    setTitlePixmap(pix);
    saveConfig();
}

void FramelessWindow::setBackgroundMode(BackgroundMode mode)
{
    m_bgMode = mode;
    refreshBackgroundDisplay();
    saveConfig();
}

void FramelessWindow::setBackgroundVideo(const QString &path)
{
    m_lastVideoPath = path;
    m_mediaPlayer->setSource(QUrl::fromLocalFile(path));
    m_mediaPlayer->play();
    setBackgroundMode(Video);
}

void FramelessWindow::refreshBackgroundDisplay()
{
    m_backgroundView->hide();
    m_videoWidget->hide();

    if (m_bgMode == Image) {
        m_backgroundView->show();
        updateBackgroundImage();
    } else if (m_bgMode == Video) {
        m_videoWidget->show();
        m_videoWidget->raise();
        mainitems->raise();
    }
}

void FramelessWindow::updateBackgroundImage()
{
    if (m_lastImagePath.isEmpty()) return;
    QPixmap pix(m_lastImagePath);
    if (pix.isNull()) return;

    m_backgroundView->scene()->clear();
    QPixmap scaled = pix.scaled(m_backgroundView->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_backgroundView->scene()->addPixmap(scaled);
    m_backgroundView->scene()->setSceneRect(m_backgroundView->viewport()->rect());
}

void FramelessWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    titlebar->setGeometry(0, 0, width(), 48);

    int bgY = 48;
    int bgH = height() - bgY;

    m_backgroundView->setGeometry(0, bgY, width(), bgH);
    m_videoWidget->setGeometry(0, bgY, width(), bgH);
    mainitems->setGeometry(0, bgY, width(), bgH);

    updateBackgroundImage();
}

void FramelessWindow::settilemode(TitleMode mode)
{
    titletxt->setVisible(false);
    picLabel->setVisible(false);
    m_savedTitleMode = mode;

    switch (mode)
    {
    case text:
        titletxt->setVisible(true);
        break;
    case singlepic:
        picLabel->setVisible(true);
        break;
    default:
        titletxt->setVisible(true);
        break;
    }
}

void FramelessWindow::setTitleText(const QString &text)
{
    titletxt->setText(text);
}

void FramelessWindow::setTitlePixmap(const QPixmap &up)
{
    m_upPixmap = up;
    if(up.isNull())
    {
        picLabel->clear();
        picLabel->setFixedWidth(0);
        return;
    }
    QPixmap scaled = up.scaledToHeight(36, Qt::SmoothTransformation);
    picLabel->setPixmap(scaled);
    picLabel->setFixedWidth(scaled.width());
}

void FramelessWindow::settileitems(TitleitemsPlace place)
{
    Q_UNUSED(place);
}

QWidget *FramelessWindow::add_titleitems(QWidget *widget)
{
    if(widget) widget->setParent(titleitems);
    return widget;
}

QWidget *FramelessWindow::add_mainitems(QWidget *widget)
{
    if(widget) widget->setParent(mainitems);
    mainitems->raise();
    return widget;
}

void FramelessWindow::on_clicked_min_btn()
{
    showMinimized();
}

void FramelessWindow::on_clicked_max_btn()
{
    toggleMaximizeRestore();
}

void FramelessWindow::toggleMaximizeRestore()
{
    if(isMaximized())
    {
        showNormal();
        max->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    }
    else
    {
        showMaximized();
        max->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    }
}

bool FramelessWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == titlebar && event->type() == QEvent::MouseButtonDblClick)
    {
        toggleMaximizeRestore();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void FramelessWindow::closeWindow()
{
    if(m_hasSetCloseAction)
    {
        if(m_savedCloseAction == MinimizeToTray)
            hide();
        else
            QWidget::close();
    }
    else
    {
        showCloseDialog();
    }
}

void FramelessWindow::showCloseDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("关闭确认");
    dialog.setFixedSize(320,160);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20,20,20,20);
    layout->setSpacing(12);

    QLabel* label = new QLabel("请选择关闭时的操作：");
    QRadioButton* btn1 = new QRadioButton("最小化到托盘");
    QRadioButton* btn2 = new QRadioButton("直接退出程序");
    btn1->setChecked(m_savedCloseAction == MinimizeToTray);

    QPushButton* okBtn = new QPushButton("确定");
    okBtn->setFixedHeight(28);

    layout->addWidget(label);
    layout->addWidget(btn1);
    layout->addWidget(btn2);
    layout->addStretch();
    layout->addWidget(okBtn);

    connect(okBtn, &QPushButton::clicked, &dialog, [&](){
        dialog.close();
        m_hasSetCloseAction = true;
        if(btn1->isChecked())
        {
            m_savedCloseAction = MinimizeToTray;
            hide();
        }
        else
        {
            QWidget::close();
        }
        saveConfig();
    });

    dialog.exec();
}

static QString titleModeToString(FramelessWindow::TitleMode mode)
{
    return QMetaEnum::fromType<FramelessWindow::TitleMode>().valueToKey(mode);
}

static FramelessWindow::TitleMode titleModeFromString(const QString& str)
{
    bool ok;
    int v = QMetaEnum::fromType<FramelessWindow::TitleMode>().keyToValue(str.toUtf8(), &ok);
    return ok ? (FramelessWindow::TitleMode)v : FramelessWindow::text;
}

static QString closeActionToString(FramelessWindow::CloseAction a)
{
    return QMetaEnum::fromType<FramelessWindow::CloseAction>().valueToKey(a);
}

static FramelessWindow::CloseAction closeActionFromString(const QString& str)
{
    bool ok;
    int v = QMetaEnum::fromType<FramelessWindow::CloseAction>().keyToValue(str.toUtf8(), &ok);
    return ok ? (FramelessWindow::CloseAction)v : FramelessWindow::MinimizeToTray;
}

static QString bgModeToString(FramelessWindow::BackgroundMode m)
{
    return QMetaEnum::fromType<FramelessWindow::BackgroundMode>().valueToKey(m);
}

static FramelessWindow::BackgroundMode bgModeFromString(const QString& str)
{
    bool ok;
    int v = QMetaEnum::fromType<FramelessWindow::BackgroundMode>().keyToValue(str.toUtf8(), &ok);
    return ok ? (FramelessWindow::BackgroundMode)v : FramelessWindow::None;
}

void FramelessWindow::loadConfig()
{
    m_savedTitleMode = titleModeFromString(m_settings.value("title_mode","text").toString());
    m_hasSetCloseAction = m_settings.value("has_set_close_action",false).toBool();
    m_bgMode = bgModeFromString(m_settings.value("background/mode", "None").toString());

    if(m_hasSetCloseAction)
        m_savedCloseAction = closeActionFromString(m_settings.value("close_action").toString());

    settilemode(m_savedTitleMode);

    if(m_savedTitleMode == text)
    {
        setTitleText(m_settings.value("title_text","Test").toString());
    }
    else
    {
        m_titleIconPath = m_settings.value("title_icon_path", "").toString();
        if (!m_titleIconPath.isEmpty()) {
            QPixmap pix(m_titleIconPath);
            setTitlePixmap(pix);
        }
    }

    m_lastImagePath = m_settings.value("background/image", "").toString();
    m_lastVideoPath = m_settings.value("background/video", "").toString();

    m_titleBarColor = m_settings.value("color/titleBar", "#87CEFA").value<QColor>();
    m_windowColor   = m_settings.value("color/window", "#C0C0C0").value<QColor>();
    setTitleBarColor(m_titleBarColor);
    setWindowColor(m_windowColor);

    refreshBackgroundDisplay();

    if (m_bgMode == Video && !m_lastVideoPath.isEmpty()) {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(m_lastVideoPath));
        m_mediaPlayer->play();
    }
}

void FramelessWindow::saveConfig()
{
    m_settings.setValue("title_mode", titleModeToString(m_savedTitleMode));
    m_settings.setValue("has_set_close_action", m_hasSetCloseAction);
    m_settings.setValue("background/mode", bgModeToString(m_bgMode));

    if(m_savedTitleMode == text)
    {
        m_settings.setValue("title_text", titletxt->text());
        m_settings.remove("title_icon_path");
    }
    else
    {
        m_settings.remove("title_text");
        m_settings.setValue("title_icon_path", m_titleIconPath);
    }

    if(m_hasSetCloseAction)
        m_settings.setValue("close_action", closeActionToString(m_savedCloseAction));

    m_settings.setValue("background/image", m_lastImagePath);
    m_settings.setValue("background/video", m_lastVideoPath);

    m_settings.setValue("color/titleBar", m_titleBarColor.name());
    m_settings.setValue("color/window", m_windowColor.name());
}

void FramelessWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(style()->standardIcon(QStyle::SP_TitleBarMenuButton));
    m_trayIcon->setToolTip("应用已最小化到托盘");

    QMenu* trayMenu = new QMenu(this);
    trayMenu->addAction("显示主窗口", this, [this](){
        showNormal();raise();activateWindow();
    });
    trayMenu->addAction("退出程序", this, &QWidget::close);
    m_trayIcon->setContextMenu(trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason){
        if(reason == QSystemTrayIcon::Trigger)
        {
            showNormal();raise();activateWindow();
        }
    });

    m_trayIcon->show();
}

int FramelessWindow::getTitleBarHeight() const
{
    return titlebar->height();
}

bool FramelessWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WINDOWS
    MSG* msg = (MSG*)message;
    if(msg->message == WM_NCHITTEST)
    {
        *result = 0;
        const int w = 8;
        RECT rc;
        GetWindowRect((HWND)winId(), &rc);
        POINT pt;
        pt.x = GET_X_LPARAM(msg->lParam);
        pt.y = GET_Y_LPARAM(msg->lParam);

        bool top    = (pt.y < rc.top + w);
        bool bottom = (pt.y > rc.bottom - w);
        bool left   = (pt.x < rc.left + w);
        bool right  = (pt.x > rc.right - w);

        if(top && left) *result = HTTOPLEFT;
        else if(top && right) *result = HTTOPRIGHT;
        else if(bottom && left) *result = HTBOTTOMLEFT;
        else if(bottom && right) *result = HTBOTTOMRIGHT;
        else if(top) *result = HTTOP;
        else if(bottom) *result = HTBOTTOM;
        else if(left) *result = HTLEFT;
        else if(right) *result = HTRIGHT;

        if(*result == 0)
        {
            POINT ptt = pt;
            ScreenToClient((HWND)winId(), &ptt);
            QPoint p(ptt.x, ptt.y);
            if(titlebar->rect().contains(p)
                && !min->geometry().contains(p)
                && !max->geometry().contains(p)
                && !close->geometry().contains(p))
            {
                *result = HTCAPTION;
            }
        }
        return (*result != 0);
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}

void FramelessWindow::setVideoVolume(float volume)
{
    if (m_mediaPlayer && m_mediaPlayer->audioOutput()) {
        m_mediaPlayer->audioOutput()->setVolume(qBound(0.0f, volume, 1.0f));
    }
}

void FramelessWindow::muteVideo(bool mute)
{
    if (m_mediaPlayer && m_mediaPlayer->audioOutput()) {
        m_mediaPlayer->audioOutput()->setMuted(mute);
    }
}

void FramelessWindow::setTitleBarColor(const QColor &color)
{
    m_titleBarColor = color;
    titlebar->setStyleSheet(QString("background-color: %1;").arg(color.name()));
}

void FramelessWindow::setWindowColor(const QColor &color)
{
    m_windowColor = color;
    setStyleSheet(QString("background-color: %1;").arg(color.name()));
}