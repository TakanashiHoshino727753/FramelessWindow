#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QStyle>
#include <QEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QResizeEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QColor>

class FramelessWindow : public QWidget
{
    Q_OBJECT
public:
    enum TitleitemsPlace { left, middle };
    enum TitleMode { text, singlepic };
    enum CloseAction { MinimizeToTray, CloseProgram };
    enum BackgroundMode { None, Image, Video };

    Q_ENUM(TitleitemsPlace)
    Q_ENUM(TitleMode)
    Q_ENUM(CloseAction)
    Q_ENUM(BackgroundMode)

    explicit FramelessWindow(QWidget *parent = nullptr);

    void settileitems(TitleitemsPlace place);
    QWidget *add_titleitems(QWidget *widget);
    QWidget *add_mainitems(QWidget *widget);
    void settilemode(TitleMode mode);
    void setTitleText(const QString &text);
    void setTitlePixmap(const QPixmap &up);
    QGraphicsView* getBackgroundView() { return m_backgroundView; }

    void setTitleIconPath(const QString &path);
    void setBackgroundMode(BackgroundMode mode);
    void setBackgroundVideo(const QString& path);
    void setVideoVolume(float volume);
    void muteVideo(bool mute);

    void setTitleBarColor(const QColor& color);
    void setWindowColor(const QColor& color);

protected:
    QWidget *titleitems;
    QWidget *mainitems;
    QFrame *titlebar;
    QPushButton *max, *min, *close;
    QLabel *titletxt;
    QLabel *picLabel;
    QGraphicsView *m_backgroundView;
    QMediaPlayer* m_mediaPlayer;
    QVideoWidget* m_videoWidget;

    QVBoxLayout *mainlayout;
    QHBoxLayout *windowctrlbtnslayout;

    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
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

    QColor m_titleBarColor;
    QColor m_windowColor;

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