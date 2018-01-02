#include "mainwindow.h"
#include "stdlib.h"
#include <stdio.h>
#include "ui_mainwindow.h"
#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QFileDialog>
#include <QDebug>
#include <QString>
#include <QPushButton>
#include <QFile>   // copy file
#include <QDir>
#include <string>
#include <iostream>
#include <stdio.h>
#include "include/IPP_World.h"
#include "select_pic_first.h"

//#define QT_NO_DEBUG_OUTPUT 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug()<<"current applicationDirPath: "<<QCoreApplication::applicationDirPath();
    version = 0;
    mix_pic_loaded = 0;
	if_selected = 0;
    already_have_mixed = 0;
    if_real_mix_change = 0;
    //ui->MainWindow->version = 0;
    ui->setupUi(this);
    setWindowTitle("Picture Editor");
    if_brightness_changed = 0;
    if_contrast_changed = 0;

    openAction = new QAction(QIcon("img/file.png"), tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing picture"));
    connect(openAction, &QAction::triggered, this, &MainWindow::open_picture);
    QMenu *open = menuBar()->addMenu(tr("&File"));
    open->addAction(openAction);
    QToolBar *toolBar_open = addToolBar(tr("&File"));
    toolBar_open->addAction(openAction);

    saveAction = new QAction(QIcon("img/save.png"), tr("&Save..."), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the current picture"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save_picture);
    QMenu *save = menuBar()->addMenu((tr("&Save")));
    open->addAction(saveAction);
    QToolBar *toolBar_save = addToolBar(tr("&Save"));
    toolBar_save->addAction(saveAction);

    undoAction = new QAction(QIcon("img/undo.png"), tr("&Undo..."), this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setStatusTip(tr("Undo the last operation"));
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    QMenu *undo = menuBar()->addMenu((tr("&Command")));
    undo->addAction(undoAction);
    QToolBar *toolBar_undo = addToolBar(tr("&Undo"));
    toolBar_undo->addAction(undoAction);

	QDir().mkdir(DEFAULTPATH);


}

int MainWindow::version = 0;

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::undo()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    if (version == 0)
    {
        QMessageBox message(QMessageBox::NoIcon, "error", "It is the origin picture!", QMessageBox::Ok, NULL);
        message.exec();
        //QMessageBox message(QMessageBox::NoIcon, "error", "This is the original picture!", QMessageBox::Yes, NULL);
        return;
    }
    version--;
    qDebug() << version;
    load_picture(version);
    history += QString::number(operate_count, 10) + ". 撤销上次操作\n";
    operate_count++;
    ui->history->setText(history);
    ui->horizontalSlider->setValue(0);
    return;

}

void MainWindow::save_picture()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    QString img = GetName();
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save Image"),
        "C:/",
        tr("*.jpg;; *.png;; *.bmp")); //选择路径
    if(filename.isEmpty())
    {
        return;
    }

    else
    {
        //QString command = "cp " + img + " " + filename;
        //char *ch;
        //QByteArray ba = command.toLatin1();
        //ch=ba.data();
        //qDebug() << ch;
        //system(ch);
		QFile::copy(img, filename);

        QMessageBox::about(this,
                    tr("Saved"),
                    tr("Already saved!"));
        history += QString::number(operate_count, 10) + "保存图片到 " + filename + "\n";
        ui->history->setText(history);
    }
}

void MainWindow::open_picture()
{
    if (if_selected)
    {
        QMessageBox message(QMessageBox::NoIcon, "warning", "After opening a new picture you will lose all previous records, continue?", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::No)
            return;
    }
    history.clear();
    operate_count = 1;
    /*QString rm_command = "rm temp/*.jpg";
    char *command;
    QByteArray command2 = rm_command.toLatin1();
    command = command2.data();
    qDebug() << command;
    system(command);*/

	QString path = DEFAULTPATH;
	QDir dir(path);
	dir.setNameFilters(QStringList() << "*.*");
	dir.setFilter(QDir::Files);
	foreach(QString dirFile, dir.entryList())
	{
		dir.remove(dirFile);
	}


    QString name = QFileDialog::getOpenFileName(NULL, "Open a file", "C:/", tr("Images (*.png *.jpg)"));
    if (name == NULL)
        return;
    if_selected = 1;
    this->pic_path = name;
    history += QString::number(operate_count, 10) + ". 打开 " + name + "\n";
    operate_count++;
    ui->history->setText(history);

    //QString cp_command = "cp " + pic_path + " " + GetName();
    //char*  ch;
    //QByteArray ba = cp_command.toLatin1();
    //ch=ba.data();
    //qDebug() << ch;
    //system(ch);

	QFile::copy(pic_path, GetName());

    load_picture(version);
}

void MainWindow::load_picture(int version)
{
    QString pic_path = DEFAULTPATH + QString::number(version, 10) + ".jpg";
    QImage *pixmap = new QImage(pic_path);
    *pixmap = pixmap->scaled(781, 541, Qt::KeepAspectRatio);
    ui->PictureArea->setPixmap(QPixmap::fromImage(*pixmap));
    ui->PictureArea->setAlignment(Qt::AlignCenter);
    IPP_hist(version);
    QString hist_path = DEFAULTPATH"hist.jpg";
    MainWindow::load_histogram(hist_path);
    ui->pic_size->setText(GetSize());
}

void MainWindow::load_histogram(QString path)
{
    QImage *pixmap = new QImage(path);
    *pixmap = pixmap->scaled(261, 171, Qt::KeepAspectRatio);
    ui->PictureHistogram->setPixmap(QPixmap::fromImage(*pixmap));
    ui->PictureHistogram->setScaledContents(1);
    ui->PictureHistogram->setAlignment(Qt::AlignCenter);
}

void MainWindow::on_resize_button_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_resize(version, COMPRESS , ui->resize_coefficient->value());
    history += QString::number(operate_count, 10) + ". 以 " + QString::number(ui->resize_coefficient->value(), 10) + "%的倍率压缩" + "\n";
    qDebug() << ui->resize_coefficient->value();
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}

void MainWindow::on_RGB_R_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_split(version, RED);
    history += QString::number(operate_count, 10) + (QString)". 获取图片的红色RGB通道\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}

void MainWindow::on_rotate_picture_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_rotate(version, 90);
    version += 1;
    MainWindow::load_picture(version);
}

void MainWindow::on_sobel_picture_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_sobel(version);
    history += QString::number(operate_count, 10) + ". 图像锐化\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}

void MainWindow::on_blur_picture_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_blur(version, 10);
    history += QString::number(operate_count, 10) + ". 图像模糊\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}

void MainWindow::on_gray_picture_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_gray(version);
    history += QString::number(operate_count, 10) + ". 转为灰度图\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}

void MainWindow::on_RGB_G_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_split(version, GREEN);
    history += QString::number(operate_count, 10) + ". 获取图片的绿色RGB通道\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}


void MainWindow::on_RGB_B_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_split(version, BLUE);
    history += QString::number(operate_count, 10) + ". 获取图片的绿色RGB通道\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}

int MainWindow::getversion()
{
    return version;
}

QString MainWindow::GetName()
{
    QString pic_path = DEFAULTPATH + QString::number(version, 10) + ".jpg";
    return pic_path;
}

QString MainWindow::GetSize()
{
    QString filepath = GetName();
    QByteArray temp = filepath.toLatin1();
    const char *temp_path = temp.data();
    ifstream in(temp_path);
    in.seekg(0, ios::end); //设置文件指针到文件流的尾部
    streampos ps = in.tellg(); //读取文件指针的位置
    double size = (double)ps / 1024;
    QString result = QString::number(size, 10, 3) + "kb";
    return result;
}

QString MainWindow::GetSize(QString mix_path)
{
    QByteArray temp = mix_path.toLatin1();
    const char *temp_path = temp.data();
    ifstream in(temp_path);
    in.seekg(0, ios::end); //设置文件指针到文件流的尾部
    streampos ps = in.tellg(); //读取文件指针的位置
    double size = (double)ps / 1024;
    QString result = QString::number(size, 10, 3) + "kb";
    return result;
}

void delay(int seconds)
{
   clock_t start = clock();
   clock_t lay = (clock_t)seconds * CLOCKS_PER_SEC / 1000;
   while ((clock()-start) < lay)
     ;
}
void MainWindow::on_horizontalSlider_valueChanged(int value)    // 改变亮度
{
    if (!if_selected){
        ui->horizontalSlider->setValue(0);
        return;
    }
    if_brightness_changed = 1;
    delay(100);
    IPP_brightness(version, value);
    QString pic_path = DEFAULTPATH + QString("lightchange.jpg");
    QImage *pixmap = new QImage(pic_path);
    *pixmap = pixmap->scaled(781, 541, Qt::KeepAspectRatio);
    ui->PictureArea->setPixmap(QPixmap::fromImage(*pixmap));
    ui->PictureArea->setAlignment(Qt::AlignCenter);
}

void MainWindow::Mouse_click(bool is_part)
{
    captureHelper = new CaptureScreen(this, is_part);
    connect(captureHelper, SIGNAL(signalCompleteCature(QPoint, QPoint)), this, SLOT(onCompleteCature(QPoint, QPoint)));
    connect(captureHelper, SIGNAL(signalCompleteCature2(QPoint)), this, SLOT(onCompleteCature2(QPoint)));
    captureHelper->show();
}


void MainWindow::onCompleteCature2(QPoint start_point)
{
    int row, col;
    row = IPP_rows(version);  // 宽度
    col = IPP_cols(version);  // 长度
    int pic_show_cols, pic_show_rows;  // 当前界面展示的图片长度与图片宽度
    double x = col / row;
    double y = 781.0 / 541.0;
    if (x <= y)
    {
        pic_show_rows = 541;
        pic_show_cols = (double)col * 541.0 / (double)row;
    }
    else{
        pic_show_cols = 781;
        pic_show_rows = row * 781.0 / (double)col;
    }
    int real_x1, real_y1;
    qDebug() << col << " " << row;
    qDebug() << "the actual size: " << pic_show_cols << " " << pic_show_rows;

    double temp = (double)start_point.x() / (double)pic_show_cols;
    real_x1 = temp * col;
    temp = (double)start_point.y() / (double)pic_show_rows;
    real_y1 = temp * row;
    qDebug() << real_x1 << real_y1 << endl;
    IPP_floodfill(version, real_x1, real_y1, 3);
    version += 1;
    load_picture(version);
}

void MainWindow::onCompleteCature(QPoint start_point, QPoint end_point)
{
    qDebug() << start_point << " " << end_point;
    int row, col;
    row = IPP_rows(version);  // 宽度
    col = IPP_cols(version);  // 长度
    int pic_show_cols, pic_show_rows;  // 当前界面展示的图片长度与图片宽度
    double x = col / row;
    double y = 781.0 / 541.0;
    if (x <= y)
    {
        pic_show_rows = 541;
        pic_show_cols = (double)col * 541.0 / (double)row;
    }
    else{
        pic_show_cols = 781;
        pic_show_rows = row * 781.0 / (double)col;
    }
    int real_x1, real_x2, real_y1, real_y2;
    qDebug() << col << " " << row;
    qDebug() << "the actual size: " << pic_show_cols << " " << pic_show_rows;
    if (start_point.x() == end_point.x() || start_point.y() == end_point.y())
    {
        QMessageBox message(QMessageBox::NoIcon, "error", "Error on selecting parts!", QMessageBox::Yes, NULL);
        message.exec();
        return;
    }
    int sim_x1, sim_x2, sim_y1, sim_y2;
    if (start_point.x() < end_point.x())
    {
        if (start_point.y() < end_point.y())
        {
            sim_x1 = start_point.x();
            sim_y1 = start_point.y();
            sim_x2 = end_point.x();
            sim_y2 = end_point.y();
        }
        else
        {
            sim_x1 = start_point.x();
            sim_y1 = end_point.y();
            sim_x2 = end_point.x();
            sim_y2 = start_point.y();
        }
    }
    else
    {
        if (start_point.y() < end_point.y())
        {
            sim_x1 = end_point.x();
            sim_x2 = start_point.x();
            sim_y1 = start_point.y();
            sim_y2 = end_point.y();
        }
        else
        {
            sim_x1 = end_point.x();
            sim_y1 = end_point.y();
            sim_x2 = start_point.x();
            sim_y2 = start_point.y();
        }
    }

    double temp = (double)sim_x1 / (double)pic_show_cols;
    real_x1 = temp * col;
    temp = (double)sim_x2 / (double)pic_show_cols;
    real_x2 = temp * col;
    temp = (double)sim_y1 / (double)pic_show_rows;
    real_y1 = temp * row;
    temp = (double)sim_y2 / (double)pic_show_rows;
    real_y2 = temp * row;
    qDebug() << "(" << real_x1 << ", " << real_y1 << "), (" << real_x2 << ", " << real_y2 << ")";

    IPP_cut(version, real_x1, real_y1, real_x2 - real_x1, real_y2 - real_y1);

    version += 1;
    load_picture(version);
    //QString path = GetName();
    //captureImage.save(path);
    //load_picture(version);
}

void MainWindow::on_cut_pic_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    Mouse_click(1);
}

void MainWindow::on_pic_mix_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    QString name = QFileDialog::getOpenFileName(NULL, "Open a file", "/Users/zuhxs/Desktop/pic", tr("Images (*.png *.jpg)"));
    if (name == NULL)
        return;
    history += QString::number(operate_count, 10) + ". 图片混合，图片2为: " + name + "\n";
    ui->history->setText(history);
    //QString cp_command = "cp " + name + " " + DEFAULTPATH + (QString)"mix.jpg";
    //char*  ch;
    //QByteArray ba = cp_command.toLatin1();
    //ch=ba.data();
    //qDebug() << ch;
    //system(ch);
	QString new_name = DEFAULTPATH"mix.jpg";
	QFile::copy(name, new_name);

    mix_pic_loaded = 1;
    if_real_mix_change = 1;
    ui->MixSlider->setValue(0);
}

void MainWindow::on_MixSlider_valueChanged(int value)
{
    if (if_real_mix_change)
    {
        if_real_mix_change = 0;
        return;
    }
    if (!mix_pic_loaded){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please first select another picture to mix!", QMessageBox::Yes | QMessageBox::No, NULL);

        if (message.exec() == QMessageBox::No){
            if_real_mix_change = 1;
            ui->MixSlider->setValue(0);
            return;
        }
        MainWindow::on_pic_mix_clicked();
        if_real_mix_change = 1;
        ui->MixSlider->setValue(0);
    }

    delay(100);
    IPP_addweight(version, (100.0-value) / 100.0, value / 100.0);
    already_have_mixed = 1;


    QString pic_path = DEFAULTPATH + QString("mixed.jpg");
    QImage *pixmap = new QImage(pic_path);
    *pixmap = pixmap->scaled(781, 541, Qt::KeepAspectRatio);
    ui->PictureArea->setPixmap(QPixmap::fromImage(*pixmap));
    ui->PictureArea->setAlignment(Qt::AlignCenter);

    std::string path_mix = DEFAULT_PATH + (string)"mix.jpg";

    IPP_hist(path_mix);
    QString hist_path = DEFAULTPATH"hist_mix.jpg";
    MainWindow::load_histogram(hist_path);
    ui->pic_size->setText(GetSize());
}

void MainWindow::on_buttonBox_accepted()
{
    if (!already_have_mixed)
    {
        QMessageBox message(QMessageBox::NoIcon, "error", "Please first mix the picture!", QMessageBox::Yes | QMessageBox::No, NULL);
        message.exec();
        if_real_mix_change = 1;
        ui->MixSlider->setValue(0);
        return;
    }
    version += 1;
    //QString cp_command = QString("cp ") + DEFAULTPATH + QString("mixed.jpg ") + GetName();
    //char*  ch;
    //QByteArray ba = cp_command.toLatin1();
    //ch=ba.data();
    //qDebug() << ch;
    //system(ch);
    //system("rm temp/mix.jpg temp/mixed.jpg");

	QFile::copy(DEFAULTPATH"mixed.jpg", GetName());
	QDir dir(DEFAULTPATH);
	dir.remove("mix.jpg");
	dir.remove("mixed.jpg");

    if_real_mix_change = 1;
    ui->MixSlider->setValue(0);
    already_have_mixed = 0;
    mix_pic_loaded = 0;

    load_picture(version);

}

void MainWindow::on_buttonBox_rejected()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    //system("rm temp/mix.jpg temp/mixed.jpg");
	QDir dir(DEFAULTPATH);
	dir.remove("mix.jpg");
	dir.remove("mixed.jpg");

    if_real_mix_change = 1;
    ui->MixSlider->setValue(0);
    already_have_mixed = 0;
    mix_pic_loaded = 0;
    load_picture(version);
}

void MainWindow::on_pushButton_3_clicked()    // generate fisheye picture
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_fisheye(version);
    history += QString::number(operate_count, 10) + ". 生成鱼眼图\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);

}

void MainWindow::on_pushButton_clicked()    // 人脸检测
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    IPP_img_face_detection(version);
    history += QString::number(operate_count, 10) + ". 人脸检测\n";
    operate_count++;
    ui->history->setText(history);
    version += 1;
    MainWindow::load_picture(version);
}

void MainWindow::on_ContrastSlider_valueChanged(int value)
{
    if (!if_selected){
        ui->ContrastSlider->setValue(0);
        return;
    }
    if_contrast_changed = 1;
    delay(100);
    IPP_contrast(version, value + 50);
    QString pic_path = DEFAULTPATH + QString("contrastchange.jpg");
    QImage *pixmap = new QImage(pic_path);
    *pixmap = pixmap->scaled(781, 541, Qt::KeepAspectRatio);
    ui->PictureArea->setPixmap(QPixmap::fromImage(*pixmap));
    ui->PictureArea->setAlignment(Qt::AlignCenter);
}

void MainWindow::on_buttonBox_2_accepted()    // 改变亮度的OK选项
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    if (!if_brightness_changed)
    {
        return;
    }
    version += 1;
    //QString cp_command = QString("cp ") + DEFAULTPATH + QString("lightchange.jpg ") + GetName();
    //char*  ch;
    //QByteArray ba = cp_command.toLatin1();
    //ch=ba.data();
    //qDebug() << ch;
    //system(ch);
    //system("rm lightchange.jpg");

	QFile::copy(DEFAULTPATH"lightchange.jpg", GetName());
	QFile::remove(DEFAULTPATH"lightchange.jpg");

    if_brightness_changed = 0;
    ui->horizontalSlider->setValue(0);

    load_picture(version);

}

void MainWindow::on_buttonBox_2_rejected()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    if_brightness_changed = 0;
    ui->horizontalSlider->setValue(0);
    load_picture(version);
    return;
}

void MainWindow::on_contrast_button_accepted()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    if (!if_contrast_changed)
    {
        return;
    }
    version += 1;
    //QString cp_command = QString("cp ") + DEFAULTPATH + QString("contrastchange.jpg ") + GetName();
    //char*  ch;
    //QByteArray ba = cp_command.toLatin1();
    //ch=ba.data();
    //qDebug() << ch;
    //system(ch);
    //system("rm contrastchange.jpg");

	QFile::copy(DEFAULTPATH"contrastchange.jpg", GetName());
	QFile::remove(DEFAULTPATH"contrastchange.jpg");

    if_contrast_changed = 0;
    ui->ContrastSlider->setValue(0);

    load_picture(version);
}

void MainWindow::on_contrast_button_rejected()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    if_contrast_changed = 0;
    ui->ContrastSlider->setValue(0);
    load_picture(version);
    return;
}

void MainWindow::on_floodfill_clicked()
{
    if (!if_selected){
        QMessageBox message(QMessageBox::NoIcon, "error", "Please select a picture first!", QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes){
            MainWindow::open_picture();
        }
        return;
    }
    Mouse_click(0);
}
