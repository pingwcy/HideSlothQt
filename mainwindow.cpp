//引用UI
#include "mainwindow.h"
#include "dialog.h"
#include "aboutbox.h"
#include "ui_mainwindow.h"

//引用Qt库
#include <QMessageBox>
#include <QFuture>
#include <QFutureWatcher>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QByteArray>
#include <QtConcurrent>
#include <QDir>
#include <QDialog>

//引用C++标准库
#include <string>

//引用项目中的自写文件
#include "Linear_Image.cpp"
#include "Encryption.cpp"
#include "GlobalSettings.h"
#include <DCT.cpp>
#include <dctreader.h>
#include "utils_a.h"
#include "bulk_decode.h"
#include "bulk_encode.h"
#include "logicmain.h"

//Qt主窗口初始化
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    statusBar()->setStyleSheet("QStatusBar{background-color: #e8ebe9; border: 1px  #e3e6e4;}");
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::runAsyncTask(std::function<void()> task, std::function<void()> onFinished) {
    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
    QObject::connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
    QObject::connect(watcher, &QFutureWatcher<void>::finished, this, onFinished);

    QFuture<void> future = QtConcurrent::run(task);
    watcher->setFuture(future);
}

void MainWindow::handleEncodeMode(bool File, bool Isstring, const QString SecretRoute, QString PlainText,std::string passwordStr) {
    if (File && !Isstring) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
        runAsyncTask(
            [=]() { logic::encryptFile(SecretRoute, fileName, passwordStr); },
            [this]() { QMessageBox::information(this, tr("Done"), tr("Encryption Done!")); }
            );
    } else if (Isstring && !File) {
        ui->TextIO->setText(logic::encryptString(PlainText, passwordStr));
    }
}

void MainWindow::handleDecodeMode(bool File, bool Isstring, const QString SecretRoute, QString PlainText,std::string passwordStr) {
    if (File && !Isstring) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
        runAsyncTask(
            [=]() { logic::decryptFile(SecretRoute, fileName, passwordStr); },
            [this]() { QMessageBox::information(this, tr("Done"), tr("Decryption Done!")); }
            );
    } else {
        ui->TextIO->setText(logic::decryptString(PlainText, passwordStr));
    }
}

//主按钮事件
void MainWindow::on_pushButton_2_clicked()
{
    // 获取 密码框 的文本
    QString Password = ui->Password_Text->text();
    QByteArray passwordBytes = Password.toUtf8(); // UTF8将QString转换为QByteArray
    std::string passwordStr = passwordBytes.toStdString(); // 确保数据有效避免悬挂
    //获取路径，后续需要处理路径字符串问题
    QString SecretRoute = ui-> Secret_Text->text();
    QString ContainerRoute = ui->Container_Text->text();
    //获取明文，直接UTF8就可
    QString PlainText = ui->TextIO->toPlainText();
    // 检查 radio buttons 的状态
    bool Encode = ui->radioButton->isChecked();
    bool Decode = ui->radioButton_2->isChecked();
    bool File = ui->radioButton_3->isChecked();
    bool Isstring = ui->radioButton_4->isChecked();
    bool isMode = GlobalSettings::instance().getMode();

    if (Encode && isMode) {
        QString FileName2 = QFileDialog::getSaveFileName(nullptr, "Save File");
        runAsyncTask(
            [=]() {
                logic::encode_logic(PlainText, SecretRoute, Isstring, File, ContainerRoute, passwordStr, FileName2, this);
            },
            [this]() { showSuccessMessage(); }
            );
    } else if (Decode && isMode) {
        ui->pushButton_2->setEnabled(false);
        runAsyncTask(
            [=]() {
                logic::decode_logic(ContainerRoute, passwordStr, File, Isstring, this);
                ui->pushButton_2->setEnabled(true);
            },
            [this]() { showSuccessMessage2(); }
            );
    } else if (Encode && !isMode) {
        handleEncodeMode(File, Isstring, SecretRoute, PlainText, passwordStr);
    } else if (Decode && !isMode) {
        handleDecodeMode(File, Isstring, SecretRoute, PlainText, passwordStr);
    }
}

void MainWindow::on_Check_Button_clicked()

{
    QString Capainfo = "";
    QString SecretRoute = ui-> Secret_Text->text();
    QString ContainerRoute = ui->Container_Text->text();
    QImage image(ContainerRoute);
    QFile file(SecretRoute);
    double ImageCapa;
#ifdef _WIN32
    const std::string path = Utils::wstringToUtf8(ContainerRoute.toStdWString());
#else
    const std::string path = ContainerRoute.toUtf8().toStdString();
#endif

    if (Linear_Image::isPNG(path)){
        ImageCapa = Linear_Image::CheckSize(image);
        Capainfo += "This container's capacity is "+QString::number(ImageCapa)+" KB. ";
    }
    else if (DCT::isJPEG(path)){
        ImageCapa = (image.height()*image.width()/64)*1.5/8/1024;
        Capainfo += "This container's capacity is "+QString::number(ImageCapa)+" KB. ";
    }
    if (file.exists()) {
        qint64 size = file.size(); // 获取文件大小
        QString fileSize = QString::number(size/1024);
        Capainfo+="The secret file is ";
        Capainfo+=fileSize;
        Capainfo+=" KB.";
    }
    ui->CapaInfo->setText(Capainfo);
}

void MainWindow::on_SelectContainer_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"));
    ui->Container_Text->setText(fileName);
}

void MainWindow::on_SelectSecret_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"));
    ui->Secret_Text->setText(fileName);
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->TextIO->setText("");
    ui->CapaInfo->setText("Capacity Info");
    ui->Container_Text->setText("");
    ui->Secret_Text->setText("");
    ui->Password_Text->setText("");
    ui->radioButton->setChecked(false);
    ui->radioButton_2->setChecked(false);
    ui->radioButton_3->setChecked(false);
    ui->radioButton_4->setChecked(false);

    /*
    if(!setting){
        QDialog dialog(this);
        dialog.exec();
    }*/
}

void MainWindow::on_adv_btn_clicked()
{
    Dialog *dialog = new Dialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::on_actionAdvanced_Settings_triggered()

{
    Dialog *dialog = new Dialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::on_actionExit_triggered()
{
    //qApp->dumpObjectTree();
    this->close();
}

void MainWindow::on_actionSelect_Container_triggered()


{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"));
    ui->Container_Text->setText(fileName);
}

void MainWindow::on_actionSelect_Secret_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"));
    ui->Secret_Text->setText(fileName);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutBox *aboutbox = new AboutBox();
    aboutbox->setAttribute(Qt::WA_DeleteOnClose); // 设置自动删除属性
    aboutbox->exec();
}

void MainWindow::on_actionDCT_Cof_Reader_triggered()
{
    dctreader *dctreader1 = new dctreader();
    dctreader1->setAttribute(Qt::WA_DeleteOnClose);
    dctreader1->show();
}


void MainWindow::on_pushButton_clicked()
{

}


void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}


void MainWindow::on_actionRestart_triggered()
{
    QString program = QCoreApplication::applicationFilePath();
    QStringList arguments = QCoreApplication::arguments();

    QProcess::startDetached(program, arguments); // 启动新的进程
    QCoreApplication::quit(); // 退出当前进程

}


void MainWindow::on_actionBulk_Encode_triggered()
{
    bulk_encode *bken1 = new bulk_encode();
    bken1->setAttribute(Qt::WA_DeleteOnClose);
    bken1->show();
}


void MainWindow::on_actionBuld_Decode_triggered()
{
    bulk_decode *bken2 = new bulk_decode();
    bken2->setAttribute(Qt::WA_DeleteOnClose);
    bken2->show();

}

