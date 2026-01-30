#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QPushButton>
#include <QIcon>
#include <QPixmap>
#include <QPalette>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QMediaPlaylist>
#include <QFileDialog>
#include <QPainter>
#include <QTimer>

//文件系统
#include <QDir>
#include <QFile>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onMusicListItemClicked(QListWidgetItem * item);//列表点击
    void playMusicByIndex(int index);               //通过索引播放音乐列表
    void updateCurrentMusic(int position);          //当前音乐更新
    void addMusicFile(const QString &filePath, bool skipUIUpdate = false); //添加音乐文件
    QString formatTime(qint64 milliseconds);        // 格式化时间函数
    void initDiskAnimation();      // 初始化磁盘动画
    void updateDiskRotation();     // 更新磁盘旋转
    void startDiskAnimation();     // 开始磁盘动画
    void stopDiskAnimation();      // 停止磁盘动画
    QPixmap cropImageToCircle(const QPixmap& originalPixmap);//绘制圆形碟片
private slots:
    //各类按钮槽函数
    void on_PreButton_clicked();
    void on_PlayButton_clicked();
    void on_NextButton_clicked();
    void on_ModeButton_clicked();
    void on_ListButton_clicked();
    void on_AddButton_clicked();
    void on_DeleteButton_clicked();

    void updateProgress(qint64 position);           // 更新进度条位置
    void updateDuration(qint64 duration);           // 更新总时长
    void onProgressSliderPressed();                 // 进度条按下事件
    void onProgressSliderReleased();                // 进度条释放事件
    void onProgressSliderMoved(int value);          // 进度条拖动事件
    void updateVolume(int value);                   // 音量控制
private:
    //设置背景
    void  setBackGround(const QString & filename);
    //设置按钮的样式
    void setButtonStyle(QPushButton * button,const QString & filename,int bthwidth,int bthheight);
    //初始化按钮
    void initButtons();
    // 初始化进度条
    void initProgressSlider();
    //初始化控制音量的滑动条
    void initVolume();
    //美化音乐列表
    void beautifyMusicList();
    //加载指定的文件夹
    void loadAppointMusicDir(const QString & filepath);
    void updateModeButtonIcon();  // 更新模式按钮图标
    void drawInitialBorder();      //绘制初始描边
private:
    Ui::MainWindow *ui;
    QMediaPlayer *m_player;      //音乐播放器
    QStringList m_musicList;     //存储所有音乐文件路径
    int m_currentIndex;          //当前播放的歌曲索引
    QMediaPlaylist *m_playlist;  //播放列表
    QString m_musicDir;          //音乐文件夹路径
    //播放模式枚举
     enum PlayMode {
         OrderMode =  0,     // 顺序循环
         RandomMode = 1,    // 随机播放
         AroundMode = 2     // 单曲循环
     };
     PlayMode m_currentMode;  // 当前播放模式
     bool m_isListVisible;    // 列表是否可见
     bool m_isSliderPressed;                         // 标记进度条是否被拖动
     QTimer* m_positionTimer;                        // 定时器更新进度（备用方案）
     //磁盘动画相关
     QTimer* m_diskTimer;           // 定时器控制旋转
     QPixmap m_diskPixmap;          // 原始磁盘图片
     int m_diskRotationAngle;       // 当前旋转角度（0-359）
     QLabel* m_diskLabel;           // 显示磁盘的标签（如果在UI中添加了对象）

};
#endif // MAINWINDOW_H
