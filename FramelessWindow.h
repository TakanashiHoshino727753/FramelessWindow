#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#include <QAudioOutput>
#include <QEvent>
#include <QFrame>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>

class FramelessWindow : public QWidget
{
    Q_OBJECT
public:
    // 标题控件位置
    enum TitleitemsPlace { left, middle };
    // 标题模式：文本/图标
    enum TitleMode { text, singlepic };
    // 关闭窗口行为
    enum CloseAction { MinimizeToTray, CloseProgram };
    // 背景模式：无/图片/视频
    enum BackgroundMode { None, Image, Video };

    Q_ENUM(TitleitemsPlace)
    Q_ENUM(TitleMode)
    Q_ENUM(CloseAction)
    Q_ENUM(BackgroundMode)

    explicit FramelessWindow(QWidget *parent = nullptr);

    // 标题栏扩展控件
    void settileitems(TitleitemsPlace place);
    QWidget *add_titleitems(QWidget *widget);
    QWidget *add_mainitems(QWidget *widget);

    // 标题设置
    void settilemode(TitleMode mode);
    void setTitleText(const QString &text);
    void setTitlePixmap(const QPixmap &up);
    QGraphicsView *getBackgroundView() { return m_backgroundView; }

    // 标题&背景配置
    void setTitleIconPath(const QString &path);
    void setBackgroundMode(BackgroundMode mode);
    void setBackgroundVideo(const QString &path);

    // 视频音量控制
    void setVideoVolume(float volume);
    void muteVideo(bool mute);

protected:
    QWidget *titleitems;
    QWidget *mainitems;
    QFrame *titlebar;
    QPushButton *max, *min, *close;
    QLabel *titletxt;
    QLabel *picLabel;
    QGraphicsView *m_backgroundView;
    QMediaPlayer *m_mediaPlayer;
    QVideoWidget *m_videoWidget;

    QVBoxLayout *mainlayout;
    QHBoxLayout *windowctrlbtnslayout;

    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *titleLayout;
    QSettings m_settings;
    QSystemTrayIcon *m_trayIcon;

    TitleMode m_savedTitleMode;
    CloseAction m_savedCloseAction;
    BackgroundMode m_bgMode;
    bool m_hasSetCloseAction;
    QPixmap m_upPixmap;
    QString m_titleIconPath;
    QString m_lastImagePath;
    QString m_lastVideoPath;

    void setupTrayIcon();
    void showCloseDialog();
    int getTitleBarHeight() const;
    void loadConfig();
    void saveConfig();
    void updateBackgroundImage();
    void refreshBackgroundDisplay();

private slots:
    void on_clicked_min_btn();
    void on_clicked_max_btn();
    void toggleMaximizeRestore();
    void closeWindow();
};

#endif