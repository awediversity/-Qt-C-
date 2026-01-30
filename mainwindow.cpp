#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(nullptr)
    , m_playlist(nullptr)
    , m_currentIndex(-1)
    , m_currentMode(OrderMode)
    , m_isListVisible(true)
    , m_musicDir("")
    , m_isSliderPressed(false)
    , m_positionTimer(nullptr)
    , m_diskTimer(nullptr)
    , m_diskRotationAngle(0)
    , m_diskLabel(nullptr)
{
    ui->setupUi(this);
    //设置标题
    setWindowTitle("XBC的音乐播放器");
    //音乐播放器
    m_player = new QMediaPlayer(this);
    m_playlist = new QMediaPlaylist(this);
    m_player->setPlaylist(m_playlist);
    //固定窗口大小
    setFixedSize(1300,850);
    setBackGround(":/beijing/background2.png");
    //初始化控制音量的滑动条
    initVolume();
    //初始化按钮
    initButtons();
    //初始化进度条
    initProgressSlider();
    // 美化音乐列表
    beautifyMusicList();
    // 初始化磁盘动画
    initDiskAnimation();
    //加载音乐文件夹
    QString musicDir = "E:\\Qt5\\Projects\\MusicPlayer\\myMusic\\";
    m_musicDir = musicDir;
    loadAppointMusicDir(musicDir);
    //连接播放列表的信号
    connect(m_playlist, &QMediaPlaylist::currentIndexChanged, this, &MainWindow::updateCurrentMusic);
    //连接列表点击信号
    connect(ui->MusicList, &QListWidget::itemClicked,this, &MainWindow::onMusicListItemClicked);
    //连接播放器信号到进度条
    connect(m_player, &QMediaPlayer::positionChanged,this, &MainWindow::updateProgress);
    connect(m_player, &QMediaPlayer::durationChanged,this, &MainWindow::updateDuration);
}

// 绘制初始状态下的描边（不播放时显示）
void MainWindow::drawInitialBorder()
{
    if (!m_diskLabel || m_diskPixmap.isNull())
        return;

    // 创建与原始圆形图片相同大小的目标Pixmap
    int size = m_diskPixmap.width();
    QPixmap borderedPixmap(size, size);
    borderedPixmap.fill(Qt::transparent);

    // 绘制原始圆形图片
    QPainter painter(&borderedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制原始图片
    painter.drawPixmap(0, 0, m_diskPixmap);

    // 添加静态橙色描边（与updateDiskRotation中的描边保持一致）
    // 外层橙色描边
    QPen outerPen(QColor(255, 165, 0, 220), 3);
    outerPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(outerPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(1, 1, size - 2, size - 2);

    // 内层金色描边
    QPen innerPen(QColor(255, 215, 0, 180), 1);
    painter.setPen(innerPen);
    painter.drawEllipse(3, 3, size - 6, size - 6);

    // 设置到标签
    m_diskLabel->setPixmap(borderedPixmap);
}
// 将矩形图片裁剪为圆形
QPixmap MainWindow::cropImageToCircle(const QPixmap& originalPixmap)
{
    // 确定圆形的大小（取宽度和高度的最小值）
    int size = qMin(originalPixmap.width(), originalPixmap.height());

    // 创建正方形的Pixmap作为目标
    QPixmap circularPixmap(size, size);
    circularPixmap.fill(Qt::transparent);  // 透明背景

    // 创建圆形剪裁区域
    QPainterPath path;
    path.addEllipse(0, 0, size, size);

    // 绘制圆形图片
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 设置圆形剪裁区域
    painter.setClipPath(path);

    // 计算源图片的裁剪区域（居中裁剪）
    int x = (originalPixmap.width() - size) / 2;
    int y = (originalPixmap.height() - size) / 2;

    // 绘制源图片的相应部分
    painter.drawPixmap(0, 0, size, size,
                      originalPixmap, x, y, size, size);

    // 可选：添加圆形边框
    painter.setPen(QPen(QColor(255, 165, 0, 150), 3));  // 橙色边框
    painter.setBrush(Qt::NoBrush);  // 无填充
    painter.drawEllipse(0, 0, size, size);

    return circularPixmap;
}
// 初始化磁盘动画
void MainWindow::initDiskAnimation()
{
    // 加载磁盘图片
    m_diskPixmap.load(":/icon/disk.png");
    // 将矩形图片裁剪为圆形
    m_diskPixmap = cropImageToCircle(m_diskPixmap);
    // 创建或获取磁盘标签
    m_diskLabel = ui->DiskLabel;
    // 设置初始图片
    m_diskLabel->setPixmap(m_diskPixmap);
    // 创建定时器
    m_diskTimer = new QTimer(this);
    connect(m_diskTimer, &QTimer::timeout, this, &MainWindow::updateDiskRotation);
    // 初始角度
    m_diskRotationAngle = 0;
    //初始描边
    drawInitialBorder();
}

// 更新磁盘旋转
void MainWindow::updateDiskRotation()
{
// 每次旋转1度，可以调整这个值来改变旋转速度
   m_diskRotationAngle = (m_diskRotationAngle + 1) % 360;
// 创建与原始圆形图片相同大小的目标Pixmap
   int size = m_diskPixmap.width();  // 已经是圆形，宽度和高度相同
   QPixmap rotatedPixmap(size, size);
   rotatedPixmap.fill(Qt::transparent);

   // 使用QPainter进行旋转
   QPainter painter(&rotatedPixmap);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.setRenderHint(QPainter::SmoothPixmapTransform);

   // 设置旋转中心为圆心
   painter.translate(size / 2, size / 2);
   painter.rotate(m_diskRotationAngle);
   painter.translate(-size / 2, -size / 2);

   // 绘制旋转后的图片
   painter.drawPixmap(0, 0, m_diskPixmap);
   painter.save();  // 保存旋转状态
   painter.resetTransform();  // 重置变换，绘制不旋转的描边

   // 外层橙色描边
   QPen outerPen(QColor(255, 165, 0, 220), 3);
   outerPen.setJoinStyle(Qt::RoundJoin);
   painter.setPen(outerPen);
   painter.setBrush(Qt::NoBrush);
   painter.drawEllipse(1, 1, size - 2, size - 2);

   // 内层金色描边
   QPen innerPen(QColor(255, 215, 0, 180), 1);
   painter.setPen(innerPen);
   painter.drawEllipse(3, 3, size - 6, size - 6);

    // 设置到标签
    m_diskLabel->setPixmap(rotatedPixmap);
}
// 开始磁盘动画
void MainWindow::startDiskAnimation()
{
    if (m_diskTimer && !m_diskTimer->isActive()) {
        m_diskTimer->start(30);  // 每30ms更新一次
    }
}
// 停止磁盘动画
void MainWindow::stopDiskAnimation()
{
    if (m_diskTimer && m_diskTimer->isActive()) {
        m_diskTimer->stop();
    }
}
// 美化音乐列表
void MainWindow::beautifyMusicList()
{
    // 设置列表的整体样式
    ui->MusicList->setStyleSheet(
        // 列表背景和边框
        "QListWidget {"
        "    background-color: rgba(0, 0, 0, 150);"  // 半透明黑色背景
        "    border: 2px solid #FFA500;"  // 橙色边框
        "    border-radius: 10px;"  // 圆角
        "    padding: 5px;"  // 内边距
        "    outline: none;"  // 去掉焦点虚线框
        "    font-family: 'Microsoft YaHei', 'Segoe UI';"  // 字体
        "}"

        // 列表项正常状态
        "QListWidget::item {"
        "    color: white;"  // 文字颜色
        "    background-color: rgba(255, 255, 255, 30);"  // 半透明白色背景
        "    border-radius: 5px;"  // 圆角
        "    margin: 2px;"  // 项之间的间距
        "    padding: 8px 10px;"  // 内边距
        "    min-height: 40px;"  // 最小高度
        "    font-size: 14px;"  // 字体大小
        "    font-weight: normal;"  // 字体粗细
        "}"

        // 列表项悬停状态
        "QListWidget::item:hover {"
        "    background-color: rgba(255, 165, 0, 150);"  // 半透明橙色背景
        "    color: white;"  // 文字颜色
        "    border: 1px solid #32CD32;"  // 橙色边框
        "}"

        // 列表项选中状态（当前播放的歌曲）
        "QListWidget::item:selected {"
        "    background-color: rgba(60, 30, 0, 150);"  // 半透明橙色背景
        "    color: white;"  // 文字颜色
        "    border: 2px solid #FFD700;"  // 橙色边框
        "    font-weight: bold;"  // 粗体
        "}"

        // 列表项选中状态（但不是当前播放的歌曲）
        "QListWidget::item:selected:!active {"
        "    background-color: rgba(255, 165, 0, 150);"  // 半透明橙色背景
        "    color: white;"
        "    border: 1px solid #32CD32;"
        "}"

        // 交替行颜色（可选）
        "QListWidget::item:alternate {"
        "    background-color: rgba(255, 255, 255, 20);"
        "}"

        // 去掉默认的选中虚线框
        "QListWidget::item:focus {"
        "    outline: none;"
        "}"

        // 自定义滚动条样式
        "QScrollBar:vertical {"
        "    border: none;"
        "    background-color: rgba(0, 0, 0, 100);"  // 滚动条背景
        "    width: 12px;"  // 滚动条宽度
        "    border-radius: 6px;"  // 圆角
        "    margin: 0px;"  // 外边距
        "}"

        "QScrollBar::handle:vertical {"
        "    background-color: #FFFF00;"  // 滑块颜色
        "    border-radius: 6px;"  // 滑块圆角
        "    min-height: 30px;"  // 滑块最小高度
        "}"

        "QScrollBar::handle:vertical:hover {"
        "    background-color: #FFD700;"  // 滑块悬停颜色
        "}"

        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    background: none;"
        "    height: 0px;"  // 隐藏上下箭头
        "}"

        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );
    // 设置字体
    QFont listFont("Microsoft YaHei", 12, QFont::Normal);
    ui->MusicList->setFont(listFont);
    // 设置选中模式为单选
    ui->MusicList->setSelectionMode(QAbstractItemView::SingleSelection);
    // 设置选中行为为整行选中
    ui->MusicList->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 设置鼠标悬停时选中
    ui->MusicList->setMouseTracking(true);
    // 设置平滑滚动
    ui->MusicList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // 设置网格线（可选，通常不显示）
    ui->MusicList->setGridSize(QSize(-1, 45));  // 设置行高
    // ui->MusicList->setShowGrid(false);  // 隐藏网格线
}
//控制音量的滑动条初始化
void MainWindow::initVolume()
{
    ui->VolumeSlider->setRange(0, 100);
    ui->VolumeSlider->setValue(50);  // 默认音量50%
    ui->VolumeSlider->setStyleSheet(
        "QSlider::groove:vertical {"
        "    border: 1px solid #999999;"
        "    width: 6px;"  // 垂直滑块改为width
        "    background: #505050;"
        "    margin: 0 2px;"  // 垂直滑块边距调整
        "    border-radius: 3px;"
        "}"
        "QSlider::handle:vertical {"
        "    background: #C0C0C0;"  // 改为灰色（银灰色）
        "    border: 2px solid #666666;"  // 灰色边框
        "    width: 20px;"  // 滑块宽度
        "    height: 20px;"  // 滑块高度
        "    margin: 0 -7px;"  // 垂直滑块边距调整，负边距让滑块突出
        "    border-radius: 10px;"  // 圆形滑块
        "}"
        // 添加滑块悬停效果
        "QSlider::handle:vertical:hover {"
        "    background: #D3D3D3;"  // 浅灰色
        "    border: 2px solid #555555;"  // 深灰色边框
        "}"
        // 添加滑块按下效果
        "QSlider::handle:vertical:pressed {"
        "    background: #A9A9A9;"  // 暗灰色
        "    border: 2px solid #444444;"  // 更深的灰色边框
        "}"
        );
    ui->VolumeLabel->setText("50%");
    ui->VolumeLabel->setAlignment(Qt::AlignCenter);
    ui->VolumeLabel->setFixedSize(50, 50);
    // 设置标签样式，与你的界面风格一致
    ui->VolumeLabel->setStyleSheet("color: yellow; font-weight: bold; font-size: 18px;");
    // 连接音量控制
    connect(ui->VolumeSlider, &QSlider::valueChanged,this, &MainWindow::updateVolume);
    // 设置播放器初始音量
    m_player->setVolume(50);
}
// 更新音量
void MainWindow::updateVolume(int value)
{
    m_player->setVolume(value);
    ui->VolumeLabel->setText(QString("%1%").arg(value));
}
// 初始化进度条
void MainWindow::initProgressSlider()
{
    // 设置进度条的样式和范围
    ui->ProgressSlider->setRange(0, 1000);  // 使用0-1000的范围，便于计算
    ui->ProgressSlider->setValue(0);
    ui->ProgressSlider->setOrientation(Qt::Horizontal);
    ui->ProgressSlider->setStyleSheet(
    "QSlider::groove:horizontal {"
            "    border: 1px solid #999999;"
            "    height: 8px;"
            "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
            "    margin: 2px 0;"
            "}"
            "QSlider::handle:horizontal {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
            "    border: 1px solid #5c5c5c;"
            "    width: 18px;"
            "    margin: -2px 0;"
            "    border-radius: 3px;"
            "}"
            "QSlider::sub-page:horizontal {"
            "    background: #FFA500;"
            "    border: 1px solid #777;"
            "    height: 10px;"
            "    border-radius: 4px;"
            "}");
    // 初始化时间标签
    ui->NowLabel->setText("00:00");
    ui->TotalLabel->setText("00:00");
    ui->NowLabel->setAlignment(Qt::AlignCenter);
    ui->TotalLabel->setAlignment(Qt::AlignCenter);
    ui->NowLabel->setFixedSize(60,60);
    ui->TotalLabel->setFixedSize(60,60);
    ui->ProgressSlider->setFixedSize(600,70);
    ui->NowLabel->setStyleSheet("color: yellow;");
    ui->TotalLabel->setStyleSheet("color: yellow;");
    // 连接进度条信号
    connect(ui->ProgressSlider, &QSlider::sliderPressed,
            this, &MainWindow::onProgressSliderPressed);
    connect(ui->ProgressSlider, &QSlider::sliderReleased,
            this, &MainWindow::onProgressSliderReleased);
    connect(ui->ProgressSlider, &QSlider::sliderMoved,
            this, &MainWindow::onProgressSliderMoved);
}

//格式化时间：将毫秒转换为 MM:SS 格式
QString MainWindow::formatTime(qint64 milliseconds)
{
    if (milliseconds < 0) return "00:00";

    qint64 totalSeconds = milliseconds / 1000;
    qint64 minutes = totalSeconds / 60;
    qint64 seconds = totalSeconds % 60;

    return QString("%1:%2")
           .arg(minutes, 2, 10, QLatin1Char('0'))
           .arg(seconds, 2, 10, QLatin1Char('0'));
}

//加载指定的文件夹
void MainWindow::loadAppointMusicDir(const QString &filepath)
{
    QDir dir(filepath);
    if (!dir.exists())
    {
        QMessageBox::warning(this, "文件夹", "文件夹打开失败");
        return;
    }

    // 清空列表
    m_musicList.clear();
    ui->MusicList->clear();
    if (m_playlist)
        m_playlist->clear();

    // 获取所有文件
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);

    for (const auto &fileInfo : fileList)
    {
        // 判断是否是mp3文件
        if (fileInfo.suffix().toLower() == "mp3")
        {
            QString filePath = fileInfo.absoluteFilePath();
            m_musicList.append(filePath);

            // 添加到播放列表
            if (m_playlist)
                m_playlist->addMedia(QUrl::fromLocalFile(filePath));

            // 添加到界面列表显示
            ui->MusicList->addItem(fileInfo.baseName());
        }
    }

    // 设置初始播放模式
    updateModeButtonIcon();

    if (m_playlist)
        m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
}

//添加单个音乐文件到列表
void MainWindow::addMusicFile(const QString &filePath, bool skipUIUpdate)
{
    QFileInfo fileInfo(filePath);
    // 检查是否已经是MP3文件
    if (fileInfo.suffix().toLower() != "mp3")
        return;
    // 生成目标路径（复制到音乐文件夹）
    QString targetFileName = fileInfo.fileName();  // 获取文件名（包括扩展名）
    QString targetFilePath = m_musicDir + targetFileName;
    // 检查目标文件夹中是否已存在同名文件
    // 如果存在，生成一个唯一的文件名
    int copyNumber = 1;
    QFileInfo targetFileInfo(targetFilePath);
    while (targetFileInfo.exists())
    {
        // 分离文件名和扩展名
        QString baseName = fileInfo.baseName();
        QString suffix = fileInfo.suffix();
        // 生成新文件名：原名_副本1.mp3
        targetFileName = QString("%1_副本%2.%3").arg(baseName).arg(copyNumber).arg(suffix);
        targetFilePath = m_musicDir + targetFileName;
        targetFileInfo.setFile(targetFilePath);
        copyNumber++;
    }

    // 检查是否已经存在于当前播放列表中（避免重复添加）
    // 这里检查的是原始文件路径，不是复制后的路径
    if (m_musicList.contains(filePath))
    {
        if (!skipUIUpdate)
            QMessageBox::information(this, "提示", "歌曲已存在于列表中：" + fileInfo.baseName());
        return;
    }
    // 复制文件到目标文件夹
    if (QFile::copy(filePath, targetFilePath))
    {
        // 复制成功后，添加到音乐列表（末尾）
        m_musicList.append(targetFilePath);

        // 添加到播放列表（末尾）
        if (m_playlist)
            m_playlist->addMedia(QUrl::fromLocalFile(targetFilePath));

        // 如果不是批量添加，立即更新界面列表显示
        if (!skipUIUpdate)
        {
            QString displayName = QFileInfo(targetFilePath).baseName();
            ui->MusicList->addItem(displayName);
        }
    }
    else
    {
        if (!skipUIUpdate)
            QMessageBox::warning(this, "错误", "复制文件失败：" + targetFileName);
        return;
    }
}

//更新模式按钮图标
void MainWindow::updateModeButtonIcon()
{
    switch (m_currentMode)
    {
    case OrderMode:  // 顺序循环
        ui->ModeButton->setIcon(QIcon(":/icon/order.png"));
        if (m_playlist)
            m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
        break;
    case RandomMode:  // 随机播放
        ui->ModeButton->setIcon(QIcon(":/icon/random.png"));
        if (m_playlist)
            m_playlist->setPlaybackMode(QMediaPlaylist::Random);
        break;
    case AroundMode:  // 单曲循环
        ui->ModeButton->setIcon(QIcon(":/icon/around.png"));
        if (m_playlist)
            m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        break;
    }
}

//设置播放器背景
void  MainWindow::setBackGround(const QString & filename)
{
    //创建照片
    QPixmap pixmap(filename);
    //获取当前窗口大小
    QSize windowSize = this->size();
    //将图片缩放到当前窗口的大小
    QPixmap scalePixmap = pixmap.scaled(windowSize,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    //创建QPalette对象并设置背景图片-调色板
    QPalette pa = this->palette();
    pa.setBrush(QPalette::Background,QBrush(scalePixmap));
    //将调色板应用到窗口上
    this->setPalette(pa);
}

//设置按钮的样式
void MainWindow::setButtonStyle(QPushButton * button,const QString & filename,int bthwidth,int bthheight)
{
    //设置按钮大小
    button->setFixedSize(bthwidth,bthheight);
    //设置按钮图标
    button->setIcon(QIcon(filename));
    //设置图标大小
    button->setIconSize(QSize(button->width(),button->height()));
    //设置图标背景颜色
    button->setStyleSheet("background-color:transparent");
}

//初始化按钮
void MainWindow::initButtons()
{
    setButtonStyle(ui->PreButton,":/icon/pre.png",60,60);
    setButtonStyle(ui->PlayButton,":/icon/play.png",75,75);
    setButtonStyle(ui->NextButton,":/icon/next.png",60,60);
    setButtonStyle(ui->ModeButton,":/icon/around.png",60,60);
    setButtonStyle(ui->ListButton,":/icon/List.png",85,85);
    setButtonStyle(ui->AddButton,":/icon/add.png",75,75);
    setButtonStyle(ui->DeleteButton,":/icon/delete.png",89,89);

    //connect(ui->PlayButton,&QPushButton::clicked,this,&MainWindow::handlePlaySlot);
}
// 播放指定索引的音乐
void MainWindow::playMusicByIndex(int index)
{
    if (!m_playlist)
        return;

    if (index >= 0 && index < m_musicList.size())
    {
        m_playlist->setCurrentIndex(index);
        m_player->play();
        ui->PlayButton->setIcon(QIcon(":/icon/stop.png"));
        m_currentIndex = index;
    }
}

// 更新当前播放的音乐
void MainWindow::updateCurrentMusic(int position)
{
    if (position >= 0 && position < m_musicList.size())
    {
        m_currentIndex = position;

        // 高亮显示当前播放的歌曲
        ui->MusicList->setCurrentRow(position);

        // 更新窗口标题
        QFileInfo fileInfo(m_musicList[position]);
        setWindowTitle("XBC的音乐播放器 - " + fileInfo.baseName());
    }
}
// ==================== 槽函数实现 =================
//上一首按钮
void MainWindow::on_PreButton_clicked()
{
    if (!m_playlist || m_playlist->mediaCount() == 0)
        return;

    // 播放上一首
    m_playlist->previous();

    // 如果播放器没有在播放，开始播放
    if (m_player->state() != QMediaPlayer::PlayingState)
    {
        m_player->play();
        ui->PlayButton->setIcon(QIcon(":/icon/stop.png"));        
        // 开始磁盘动画
        startDiskAnimation();
    }
    //如果已经在播放，磁盘动画会自动继续
}
//播放暂停按钮
void MainWindow::on_PlayButton_clicked()
{
    if (m_player->state() == QMediaPlayer::PlayingState)
    {
        m_player->pause();
        ui->PlayButton->setIcon(QIcon(":/icon/play.png"));
        // 停止磁盘动画
        stopDiskAnimation();
    }
    else
    {
        // 如果没有在播放且有音乐，开始播放
        if (m_playlist && m_playlist->mediaCount() > 0)
        {
            // 如果没有当前索引，设置到第一首
            if (m_playlist->currentIndex() < 0)
                m_playlist->setCurrentIndex(0);

            m_player->play();
            ui->PlayButton->setIcon(QIcon(":/icon/stop.png"));
            // 开始磁盘动画
            startDiskAnimation();
            // 确保进度条可用
            ui->ProgressSlider->setEnabled(true);
        }
        else
        {
            // 没有音乐时，进度条不可用
            ui->ProgressSlider->setEnabled(false);
            // 停止磁盘动画
            stopDiskAnimation();
        }
    }
}
//下一首按钮
void MainWindow::on_NextButton_clicked()
{
    if (!m_playlist || m_playlist->mediaCount() == 0)
        return;

    // 播放下一首
    m_playlist->next();

    // 如果播放器没有在播放，开始播放
    if (m_player->state() != QMediaPlayer::PlayingState)
    {
        m_player->play();
        ui->PlayButton->setIcon(QIcon(":/icon/stop.png"));
        // 开始磁盘动画
        startDiskAnimation();
    }
}
//播放模式切换按钮
void MainWindow::on_ModeButton_clicked()
{
    // 切换到下一个播放模式
    m_currentMode = static_cast<PlayMode>((m_currentMode + 1) % 3);

    // 更新按钮图标和播放列表模式
    updateModeButtonIcon();

    // 显示当前模式提示
    QString modeName;
    switch (m_currentMode)
    {
    case OrderMode: modeName = "顺序循环"; break;
    case RandomMode: modeName = "随机播放"; break;
    case AroundMode: modeName = "单曲循环"; break;
    }

    // 可以添加状态栏提示或工具提示
    ui->ModeButton->setToolTip("当前模式: " + modeName);
}
//播放列表按钮
void MainWindow::on_ListButton_clicked()
{
    // 切换音乐列表的显示/隐藏
     m_isListVisible = !m_isListVisible;

     if (m_isListVisible)
     {
         // 显示列表
         ui->MusicList->show();
         ui->ListButton->setIcon(QIcon(":/icon/List.png"));
         ui->ListButton->setToolTip("隐藏列表");
     }
     else
     {
         // 隐藏列表
         ui->MusicList->hide();
         ui->ListButton->setToolTip("显示列表");
     }
}
//点击列表功能
void MainWindow::onMusicListItemClicked(QListWidgetItem *item)
{
    if (!m_playlist)
        return;

    int row = ui->MusicList->row(item);
    if (row >= 0 && row < m_musicList.size())
    {
        m_playlist->setCurrentIndex(row);
        m_player->play();
        ui->PlayButton->setIcon(QIcon(":/icon/stop.png"));
        // 开始磁盘动画
        startDiskAnimation();
    }
}

void MainWindow::on_AddButton_clicked()
{
    // 确保音乐文件夹存在
    QDir dir(m_musicDir);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            QMessageBox::critical(this, "错误", "无法创建音乐文件夹：" + m_musicDir);
            return;
        }
    }
    // 打开文件对话框，可以选择多个音乐文件
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "选择音乐文件",
        QDir::homePath(),  // 默认从用户主目录开始
        "MP3音乐文件 (*.mp3);;所有文件 (*.*)"
    );

    // 如果没有选择文件，直接返回
    if (filePaths.isEmpty())
        return;
    int addedCount = 0;
    int duplicateCount = 0;
    int copyFailedCount = 0;

    // 记录添加前的列表大小
    int oldListSize = m_musicList.size();

    // 用于存储成功添加的文件显示名称
    QStringList addedDisplayNames;

    // 遍历所有选择的文件
    for (const QString &filePath : filePaths)
    {
        QFileInfo fileInfo(filePath);

        // 检查文件是否存在
        if (!fileInfo.exists())
        {
            QMessageBox::warning(this, "文件不存在", "文件不存在：" + filePath);
            continue;
        }

        // 检查是否已经是MP3文件
        if (fileInfo.suffix().toLower() != "mp3")
        {
            QMessageBox::warning(this, "文件格式错误", "只能添加MP3文件：" + filePath);
            continue;
        }

        // 检查是否已经存在于当前播放列表中（避免重复添加）
        bool alreadyExists = false;
        for (const QString &musicPath : m_musicList)
        {
            QFileInfo existingFileInfo(musicPath);
            // 只检查文件名是否相同（不区分路径）
            if (existingFileInfo.fileName() == fileInfo.fileName())
            {
                duplicateCount++;
                alreadyExists = true;
                break;
            }
        }
        if (alreadyExists)
            continue;

        // 保存当前列表大小
        int currentListSize = m_musicList.size();

        // 调用addMusicFile函数，传入true表示跳过UI更新
        addMusicFile(filePath, true);

        // 检查是否成功添加（列表大小是否增加）
        if (m_musicList.size() > currentListSize)
        {
            // 获取刚刚添加的文件显示名称
            QString displayName = QFileInfo(m_musicList.last()).baseName();
            addedDisplayNames.append(displayName);
            addedCount++;
        }
        else
        {
            copyFailedCount++;
        }
    }

    // 所有文件处理完成后，将新添加的歌曲显示在界面列表末尾
    for (const QString &displayName : addedDisplayNames)
    {
        ui->MusicList->addItem(displayName);
    }

    // 显示添加结果
    QString message;
    if (addedCount > 0)
    {
        message = QString("成功添加 %1 首歌曲到本地资源").arg(addedCount);
        if (duplicateCount > 0)
        {
            message += QString(", %1 首歌曲已存在，未重复添加").arg(duplicateCount);
        }
        if (copyFailedCount > 0)
        {
            message += QString(", %1 首歌曲添加失败").arg(copyFailedCount);
        }
    }
    else
    {
        message = "没有添加新的歌曲";
        if (duplicateCount > 0)
        {
            message = QString("所有选中的 %1 首歌曲都已经在播放列表中").arg(duplicateCount);
        }
        else if (copyFailedCount > 0)
        {
            message = QString("%1 首歌曲添加失败").arg(copyFailedCount);
        }
    }

    // 显示消息框
    QMessageBox::information(this, "添加结果", message);
}

void MainWindow::on_DeleteButton_clicked()
{
    // 获取选中的列表项
    QList<QListWidgetItem*> selectedItems = ui->MusicList->selectedItems();

    // 如果没有选中任何项，提示用户
    if (selectedItems.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先选择要删除的歌曲");
        return;
    }
    // 确认对话框
    int songCount = selectedItems.size();
    QString confirmMessage;
    if (songCount == 1)
    {
        confirmMessage = QString("确定要删除歌曲 '%1' 吗？\n该操作会同时删除本地文件。")
                            .arg(selectedItems.first()->text());
    }
    else
    {
        confirmMessage = QString("确定要删除选中的 %1 首歌曲吗？\n该操作会同时删除本地文件。")
                            .arg(songCount);
    }

    confirmMessage += "\n\n警告：此操作不可撤销！";
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除", confirmMessage,
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;
    int deletedCount = 0;
    int fileDeleteFailedCount = 0;

    // 记录当前播放的歌曲索引
    int currentPlayingIndex = m_playlist->currentIndex();
    bool needResetCurrentIndex = false;
    // 从后往前删除，避免索引变化导致问题
    QList<int> rowsToDelete;
    for (QListWidgetItem* item : selectedItems)
    {
        rowsToDelete.append(ui->MusicList->row(item));
    }
    // 按从大到小的顺序排序，确保从后往前删除
    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());
    // 遍历要删除的行
    for (int row : rowsToDelete)
    {
        // 确保行号有效
        if (row < 0 || row >= m_musicList.size())
            continue;
        // 获取要删除的文件路径
        QString filePath = m_musicList[row];
        // 检查文件是否存在
        QFile file(filePath);
        if (file.exists())
        {
            // 尝试删除本地文件
            if (file.remove())
            {
                // 文件删除成功
                deletedCount++;
                // 如果删除的是当前正在播放的歌曲
                if (row == currentPlayingIndex)
                {
                    needResetCurrentIndex = true;
                }
                // 如果删除的歌曲在当前播放歌曲之前，需要调整当前播放索引
                else if (row < currentPlayingIndex)
                {
                    currentPlayingIndex--;
                }
            }
            else
            {
                // 文件删除失败
                fileDeleteFailedCount++;
                QMessageBox::warning(this, "删除失败",
                                     QString("无法删除文件：%1\n错误：%2")
                                     .arg(QFileInfo(filePath).fileName())
                                     .arg(file.errorString()));
                continue;  // 跳过后续操作
            }
        }
        else
        {
            // 文件不存在，但仍从列表中移除
            deletedCount++;
            if (row == currentPlayingIndex)
            {
                needResetCurrentIndex = true;
            }
            else if (row < currentPlayingIndex)
            {
                currentPlayingIndex--;
            }
        }
        // 从音乐列表中移除
        m_musicList.removeAt(row);
        // 从播放列表中移除
        if (m_playlist)
        {
            m_playlist->removeMedia(row);
        }

        // 从界面列表中移除
        QListWidgetItem* item = ui->MusicList->takeItem(row);
        delete item;
    }

    // 如果需要重置当前播放索引
    if (needResetCurrentIndex)
    {
        // 如果还有歌曲，设置当前播放索引为0（或合适的其他位置）
        if (m_playlist && m_playlist->mediaCount() > 0)
        {
            // 如果之前删除的是当前播放的歌曲，播放下一首（如果存在）
            if (currentPlayingIndex >= m_playlist->mediaCount())
            {
                currentPlayingIndex = m_playlist->mediaCount() - 1;
            }

            if (currentPlayingIndex >= 0)
            {
                m_playlist->setCurrentIndex(currentPlayingIndex);
                // 如果播放器正在播放，继续播放；否则暂停
                if (m_player->state() == QMediaPlayer::PlayingState)
                {
                    m_player->play();
                }
            }
        }
        else
        {
            // 没有歌曲了，停止播放
            m_player->stop();
            ui->PlayButton->setIcon(QIcon(":/icon/play.png"));
            // 停止磁盘动画
            stopDiskAnimation();
        }
    }

    // 显示删除结果
    QString message;
    if (deletedCount > 0)
    {
        message = QString("成功删除 %1 首歌曲").arg(deletedCount);
        if (fileDeleteFailedCount > 0)
        {
            message += QString(", %1 首歌曲文件删除失败（已从列表中移除）").arg(fileDeleteFailedCount);
        }
    }
    else
    {
        message = "没有删除任何歌曲";
    }
    ui->statusbar->showMessage(message, 3000);
}

// 更新播放进度
void MainWindow::updateProgress(qint64 position)
{
    // 如果用户正在拖动进度条，不自动更新
    if (m_isSliderPressed)
        return;

    qint64 duration = m_player->duration();
    if (duration > 0)
    {
        // 计算进度条位置（0-1000）
        int sliderPosition = static_cast<int>((position * 1000) / duration);
        ui->ProgressSlider->setValue(sliderPosition);
    }

    // 更新当前时间显示
    ui->NowLabel->setText(formatTime(position));
}

// 更新总时长
void MainWindow::updateDuration(qint64 duration)
{
    // 更新总时间显示
    ui->TotalLabel->setText(formatTime(duration));

    // 如果歌曲时长发生变化，重置进度条范围
    if (duration > 0)
    {
        ui->ProgressSlider->setRange(0, 1000);
        ui->ProgressSlider->setEnabled(true);
    }
    else
    {
        ui->ProgressSlider->setValue(0);
        ui->ProgressSlider->setEnabled(false);
    }
}

// 进度条被按下
void MainWindow::onProgressSliderPressed()
{
    m_isSliderPressed = true;
}

// 进度条被释放
void MainWindow::onProgressSliderReleased()
{
    m_isSliderPressed = false;

    // 获取进度条的值（0-1000）
    int sliderValue = ui->ProgressSlider->value();

    // 计算对应的播放位置（毫秒）
    qint64 duration = m_player->duration();
    qint64 position = (sliderValue * duration) / 1000;

    // 设置播放位置
    m_player->setPosition(position);
}

// 进度条被拖动（实时更新当前时间显示）
void MainWindow::onProgressSliderMoved(int value)
{
    // 计算对应的播放位置（毫秒）
    qint64 duration = m_player->duration();
    qint64 position = (value * duration) / 1000;

    // 更新当前时间显示（但不改变实际播放位置）
    ui->NowLabel->setText(formatTime(position));
}

MainWindow::~MainWindow()
{
    // 停止并删除磁盘定时器
    if (m_diskTimer) {
        m_diskTimer->stop();
        delete m_diskTimer;
    }
    delete ui;
}

