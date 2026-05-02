#include "FramelessWindow.h"
#include <QPainter>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>

/**
 * @brief 构造函数：初始化窗口、UI、配置、信号槽
 */
FramelessWindow::FramelessWindow(const QString &configPath, QWidget *parent)
    : QWidget(parent)
    , m_configPath(configPath)
{
    // ====================== 窗口基础属性 ======================
    this->setAttribute(Qt::WA_StyledBackground);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    this->setAttribute(Qt::WA_Hover, true);
    this->resize(900, 600);

    // ====================== 标题栏 ======================
    titlebar = new QWidget(this);
    titlebar->setFixedHeight(35);

    title = new QWidget(titlebar);
    titleText = new QLabel(title);
    titlePic = new QLabel(title);
    titlePic->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleWidget = new QWidget(titlebar);
    windowCtrlBtns = new QWidget(titlebar);

    // 标题内部布局
    titleLayout = new QHBoxLayout(title);
    titleLayout->setContentsMargins(0,0,0,0);
    titleLayout->setSpacing(6);
    titleLayout->addWidget(titleText);
    titleLayout->addWidget(titlePic);
    titleLayout->addStretch();

    // 窗口控制按钮
    min = new QPushButton(windowCtrlBtns);
    max = new QPushButton(windowCtrlBtns);
    close = new QPushButton(windowCtrlBtns);
    QSize btnSize(32,32);
    min->setFixedSize(btnSize);
    max->setFixedSize(btnSize);
    close->setFixedSize(btnSize);
    min->setStyleSheet("border:none;");
    max->setStyleSheet("border:none;");
    close->setStyleSheet("border:none;");

    QStyle *style = this->style();
    min->setIcon(style->standardIcon(QStyle::SP_TitleBarMinButton));
    max->setIcon(style->standardIcon(QStyle::SP_TitleBarMaxButton));
    close->setIcon(style->standardIcon(QStyle::SP_TitleBarCloseButton));

    // 标题栏整体布局
    QHBoxLayout *titlebarLayout = new QHBoxLayout(titlebar);
    titlebarLayout->setContentsMargins(10,0,10,0);
    titlebarLayout->setSpacing(8);
    titlebarLayout->addWidget(title, 1);
    titlebarLayout->addWidget(titleWidget, 1);
    titlebarLayout->addWidget(windowCtrlBtns, 1);

    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout(windowCtrlBtns);
    btnLayout->setContentsMargins(0,0,4,0);
    btnLayout->setSpacing(4);
    btnLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    btnLayout->addWidget(min);
    btnLayout->addWidget(max);
    btnLayout->addWidget(close);

    // ====================== 背景 ======================
    bgPicture = new QLabel(this);
    bgPicture->setAlignment(Qt::AlignCenter);
    bgPicture->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // ====================== 主内容区域（透明） ======================
    mainWidget = new QWidget(this);
    mainWidget->setAttribute(Qt::WA_TranslucentBackground);
    mainWidget->setAttribute(Qt::WA_StyledBackground);
    mainWidget->setStyleSheet("background:rgba(0,0,0,0);");

    m_mainOpacity = new QGraphicsOpacityEffect(this);
    m_mainOpacity->setOpacity(1.0);
    mainWidget->setGraphicsEffect(m_mainOpacity);

    // ====================== 系统托盘 ======================
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon("D:/test.png"));
    m_trayIcon->show();

    // ====================== 加载配置 ======================
    loadConfig();

    // ====================== 信号槽绑定 ======================
    connect(min, &QPushButton::clicked, this, &FramelessWindow::on_clicked_min_btn);
    connect(max, &QPushButton::clicked, this, &FramelessWindow::on_clicked_max_btn);
    connect(close, &QPushButton::clicked, this, &FramelessWindow::on_clicked_close_btn);

    // 延迟刷新布局
    QTimer::singleShot(0, this, [this](){ resizeEvent(nullptr); });
}

/**
 * @brief 更新标题对齐方式（左/中）
 */
void FramelessWindow::updateTitleAlign()
{
    if (m_titlePlace == Left) {
        titleLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    } else {
        titleLayout->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    }
}

/**
 * @brief 从配置文件加载所有设置
 */
void FramelessWindow::loadConfig()
{
    QSettings cfg(m_configPath, QSettings::IniFormat);

    // 背景模式
    QString bgMode = cfg.value("bg/mode", "PictureBG").toString();
    if (bgMode == "NoneBG") m_bgMode = NoneBG;
    else if (bgMode == "ColorBG") m_bgMode = ColorBG;
    else m_bgMode = PictureBG;

    // 背景缩放策略
    QString strategy = cfg.value("bg/strategy", "Fill").toString();
    if (strategy == "Stretch") m_bgStrategy = Stretch;
    else if (strategy == "Adapt") m_bgStrategy = Adapt;
    else m_bgStrategy = Fill;

    m_bgPicPath = cfg.value("bg/path", "").toString();
    m_bgColorValue = cfg.value("bg/color", "#111111").toString();

    // 标题模式
    QString titleMode = cfg.value("title/mode", "TextTitle").toString();
    if (titleMode == "NoneTitle") m_titleMode = NoneTitle;
    else if (titleMode == "PictureTitle") m_titleMode = PictureTitle;
    else m_titleMode = TextTitle;

    // 标题位置
    QString place = cfg.value("title/place", "Left").toString();
    m_titlePlace = (place == "Middle") ? Middle : Left;

    m_titleTextValue = cfg.value("title/text", "无边框窗口").toString();
    m_titleColorValue = cfg.value("title/color", "#ffffff").toString();
    m_titlePicPath = cfg.value("title/icon", "").toString();
    m_titleBarColor = cfg.value("titlebar/color", "#333333").toString();

    // 关闭行为
    QString closeAct = cfg.value("window/closeAction", "ExitProgram").toString();
    m_closeAction = (closeAct == "MinimizeToTray") ? MinimizeToTray : ExitProgram;

    // 应用配置
    setTitleBarColor(m_titleBarColor);
    setTitlePlace(m_titlePlace);
    setTitleMode(m_titleMode);
    setTitleText(m_titleTextValue);
    setTitleColor(m_titleColorValue);
    setTitlePic(m_titlePicPath);

    setBgMode(m_bgMode);
    setBgColor(m_bgColorValue);
    setBgPic(m_bgPicPath);
    setBgPicStrategy(m_bgStrategy);
}

/**
 * @brief 保存所有设置到配置文件
 */
void FramelessWindow::saveConfig()
{
    QSettings cfg(m_configPath, QSettings::IniFormat);

    // 背景
    QString bgModeStr;
    if (m_bgMode == NoneBG) bgModeStr = "NoneBG";
    else if (m_bgMode == ColorBG) bgModeStr = "ColorBG";
    else bgModeStr = "PictureBG";
    cfg.setValue("bg/mode", bgModeStr);

    QString strategyStr;
    if (m_bgStrategy == Stretch) strategyStr = "Stretch";
    else if (m_bgStrategy == Adapt) strategyStr = "Adapt";
    else strategyStr = "Fill";
    cfg.setValue("bg/strategy", strategyStr);

    cfg.setValue("bg/path", m_bgPicPath);
    cfg.setValue("bg/color", m_bgColorValue);

    // 标题
    QString titleModeStr;
    if (m_titleMode == NoneTitle) titleModeStr = "NoneTitle";
    else if (m_titleMode == PictureTitle) titleModeStr = "PictureTitle";
    else titleModeStr = "TextTitle";
    cfg.setValue("title/mode", titleModeStr);

    cfg.setValue("title/place", m_titlePlace == Middle ? "Middle" : "Left");
    cfg.setValue("title/text", m_titleTextValue);
    cfg.setValue("title/color", m_titleColorValue);
    cfg.setValue("title/icon", m_titlePicPath);
    cfg.setValue("titlebar/color", m_titleBarColor);

    // 关闭操作
    cfg.setValue("window/closeAction", m_closeAction == MinimizeToTray ? "MinimizeToTray" : "ExitProgram");
}

// ====================== 标题栏 ======================
void FramelessWindow::setTitleBarColor(const QString &color)
{
    m_titleBarColor = color;
    titlebar->setStyleSheet("background-color:" + color + ";");
}

void FramelessWindow::setTitlePlace(TitlePlace place)
{
    m_titlePlace = place;
    title->setVisible(true);
    updateTitleAlign();
}

void FramelessWindow::setTitleMode(TitleMode mode)
{
    m_titleMode = mode;
    titleText->setVisible(mode == TextTitle);
    titlePic->setVisible(mode == PictureTitle);
}

void FramelessWindow::setTitleText(const QString &text)
{
    m_titleTextValue = text;
    titleText->setText(text);
}

void FramelessWindow::setTitleColor(const QString &css)
{
    m_titleColorValue = css;
    titleText->setStyleSheet("color:" + css + ";");
}

void FramelessWindow::setTitlePic(const QString &filePath)
{
    m_titlePicPath = filePath;
    QPixmap pic(filePath);
    if (!pic.isNull())
        titlePic->setPixmap(pic.scaled(24,24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// ====================== 背景 ======================
void FramelessWindow::setBgMode(BGMode mode)
{
    m_bgMode = mode;
    bgPicture->setVisible(mode == PictureBG);
}

void FramelessWindow::setBgColor(const QString &css)
{
    m_bgColorValue = css;
    this->setStyleSheet("#FramelessWindow{background-color:" + css + ";}");
}

void FramelessWindow::setBgPic(const QString &filePath)
{
    m_bgPicPath = filePath;
    QPixmap pix(filePath);
    if (!pix.isNull()) {
        bgPicture->setPixmap(pix);
        bgPicture->setScaledContents(false);
    }
}

void FramelessWindow::setBgPicStrategy(BGGraphicStrategy strategy)
{
    m_bgStrategy = strategy;
}

/**
 * @brief 窗口大小变化时重新布局组件
 */
void FramelessWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    int w = width(), h = height(), th = titlebar->height();
    titlebar->setGeometry(0,0,w,th);
    bgPicture->setGeometry(0,th,w,h-th);
    mainWidget->setGeometry(0,th,w,h-th);
    mainWidget->raise();
}

/**
 * @brief 关闭按钮（记忆化执行，仅首次弹窗）
 */
void FramelessWindow::on_clicked_close_btn()
{
    QSettings cfg(m_configPath, QSettings::IniFormat);
    if (cfg.contains("window/closeAction"))
    {
        if (m_closeAction == MinimizeToTray)
            this->hide();
        else
            qApp->quit();
        return;
    }

    // 首次选择关闭行为
    QDialog d(this);
    d.setFixedSize(320,180);
    QLabel *text = new QLabel("请选择关闭操作（仅首次）");
    QRadioButton *b1 = new QRadioButton("退出", &d);
    QRadioButton *b2 = new QRadioButton("最小化托盘", &d);
    b1->setChecked(true);

    QVBoxLayout l(&d);
    l.addWidget(text);
    l.addWidget(b1);
    l.addWidget(b2);
    QPushButton btn("确定", &d);
    l.addWidget(&btn);
    connect(&btn, &QPushButton::clicked, &d, &QDialog::accept);

    if (d.exec() == QDialog::Accepted)
    {
        if (b1->isChecked())
            m_closeAction = ExitProgram;
        else
            m_closeAction = MinimizeToTray;

        saveConfig();

        if (m_closeAction == MinimizeToTray)
            this->hide();
        else
            qApp->quit();
    }
}

// ====================== 透明度 ======================
void FramelessWindow::setMainAlpha(int alpha)
{
    qreal o = qBound(0, alpha, 255) / 255.0;
    m_mainOpacity->setOpacity(o);
}

// ====================== 快捷工具 ======================
void FramelessWindow::addTitleWidget(QWidget *w) {
    if (!titleWidget->layout()) new QHBoxLayout(titleWidget);
    titleWidget->layout()->addWidget(w);
}

void FramelessWindow::setTitleFont(const QString &s) {
    titleText->setStyleSheet(s);
}

void FramelessWindow::addMainWidget(QWidget *w) {
    delete mainWidget->layout();
    QVBoxLayout *lay = new QVBoxLayout(mainWidget);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(w);
}

// ====================== 窗口按钮 ======================
void FramelessWindow::on_clicked_min_btn() {
    showMinimized();
}

void FramelessWindow::on_clicked_max_btn() {
    if (this->isMaximized()) {
        this->showNormal();
        max->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    } else {
        this->showMaximized();
        max->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    }
}

// ====================== 原生窗口缩放 ======================
bool FramelessWindow::nativeEvent(const QByteArray &t, void *m, qintptr *r)
{
#ifdef Q_OS_WIN
    MSG *msg = (MSG*)m;
    if (msg->message == WM_NCHITTEST) {
        POINT p{LOWORD(msg->lParam), HIWORD(msg->lParam)};
        RECT rc;
        GetWindowRect(msg->hwnd, &rc);
        int b = 8;

        bool leftEdge   = (p.x <= rc.left + b);
        bool rightEdge  = (p.x >= rc.right - b);
        bool topEdge    = (p.y <= rc.top + b);
        bool bottomEdge = (p.y >= rc.bottom - b);

        if (topEdge && leftEdge)       { *r = HTTOPLEFT;    return true; }
        if (topEdge && rightEdge)      { *r = HTTOPRIGHT;   return true; }
        if (bottomEdge && leftEdge)    { *r = HTBOTTOMLEFT; return true; }
        if (bottomEdge && rightEdge)   { *r = HTBOTTOMRIGHT;return true; }
        if (topEdge)                   { *r = HTTOP;         return true; }
        if (bottomEdge)                { *r = HTBOTTOM;      return true; }
        if (leftEdge)                  { *r = HTLEFT;        return true; }
        if (rightEdge)                 { *r = HTRIGHT;       return true; }
    }
#endif
    return QWidget::nativeEvent(t, m, r);
}

// ====================== 鼠标拖动 ======================
void FramelessWindow::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && titlebar->rect().contains(titlebar->mapFromGlobal(e->globalPosition().toPoint()))) {
        m_isDrag = true;
        m_dragPos = e->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(e);
}

void FramelessWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(m_isDrag) move(e->globalPosition().toPoint() - m_dragPos);
    QWidget::mouseMoveEvent(e);
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent *e)
{
    m_isDrag = false;
    QWidget::mouseReleaseEvent(e);
}

// ====================== 析构 ======================
FramelessWindow::~FramelessWindow() = default;