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
//加密模块中，前端保存和传递数据的结构体
std::vector<uint8_t> encryptedDataToVector(const Encryption::EncryptedData& encryptedData) {
    std::vector<uint8_t> result;
    // 添加 salt
    result.insert(result.end(), encryptedData.salt.begin(), encryptedData.salt.end());
    // 添加 iv
    result.insert(result.end(), encryptedData.iv.begin(), encryptedData.iv.end());
    // 添加 tag
    result.insert(result.end(), encryptedData.tag.begin(), encryptedData.tag.end());
    // 添加 ciphertext
    result.insert(result.end(), encryptedData.ciphertext.begin(), encryptedData.ciphertext.end());
    return result;
}
//主按钮事件
void MainWindow::on_pushButton_2_clicked()
{
    // 获取 密码框 的文本
    QString Password = ui->Password_Text->text();
    QByteArray passwordBytes = Password.toUtf8(); // UTF8将QString转换为QByteArray
    std::string passwordStr = passwordBytes.toStdString(); // 确保数据有效避免悬挂
    const char* passwordPtr = passwordStr.c_str(); // 使用 std::string 管理生命周期
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
    // 下面开始逻辑
    if (Encode && GlobalSettings::instance().getMode()){
        //先获取文本框内容
        QByteArray utf8Text = PlainText.toUtf8(); // 将QString转换为QByteArray
        //如果选择的是文件则用下方方式覆盖utf8Text
        if (File && !Isstring){
            QFile file(SecretRoute);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Error", "Failed to open the file: " + file.errorString());
                return;
            }
            utf8Text = file.readAll();
            file.close();
            //读取明文结束，下面嵌入文件名到头部用竖线分割
            QFileInfo fileInfo(SecretRoute);
            QString fileName = fileInfo.fileName()+ "|";
            QByteArray byteArray = fileName.toUtf8();
            utf8Text = byteArray + utf8Text;
        }
        //将读取内容转换为向量
        const std::vector<uint8_t> data(utf8Text.begin(), utf8Text.end()); // 使用QByteArray初始化std::vector
        //获取数据指针，为了加密需要
        const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(data.data());
        //获取长度，为了加密需要
        int dataLength = static_cast<int>(data.size());
        //初始化加密后的结构体
        Encryption::EncryptedData encryptedData;
        //如果需要加密则执行
        if (GlobalSettings::instance().getEnc()){
            encryptedData = Encryption::enc(dataPtr,passwordStr.c_str(),dataLength);
        }
        //路径处理，避免Windows下toutf8非ASC字符乱码，tolocal8bits不可靠，先处理为wstring，再用Windows提供的API转换
#ifdef _WIN32
        const std::string path = Utils::wstringToUtf8(ContainerRoute.toStdWString());
#else
        const std::string path = ContainerRoute.toUtf8().toStdString();
#endif

        //判断使用什么算法并且核实文件格式，首先是PNG-LSB
        if (GlobalSettings::instance().getStealg()=="PNG-LSB"){
            if (!Linear_Image::isPNG(path) && !GlobalSettings::instance().getJPGLSB()){
                QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                return;
            }
        }
        //然后是JPG-DCT
        if (GlobalSettings::instance().getStealg()=="JPG-DCT"){
            if (!DCT::isJPEG(path)){
                QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                return;
            }
        }
        //获取指定输出文件名
        QString fileName2 = QFileDialog::getSaveFileName(this,tr("Save File"));
        //路径处理
#ifdef _WIN32
        std::string containerPath = Utils::wstringToUtf8(ContainerRoute.toStdWString());
        std::string fileNamePath = Utils::wstringToUtf8(fileName2.toStdWString());
#else
        std::string containerPath = ContainerRoute.toUtf8().toStdString();
        std::string fileNamePath = fileName2.toUtf8().toStdString();
#endif

        //随后开启Concurrent避免阻塞GUI
        QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
        // 连接信号和槽
        QObject::connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
        QObject::connect(watcher, &QFutureWatcher<void>::finished, this, &MainWindow::showSuccessMessage);
        //不同算法 是否加密
        QFuture<void> future = QtConcurrent::run([=]() {
            if (GlobalSettings::instance().getStealg()=="JPG-DCT"){
                DCT::encode_image(containerPath.c_str(), fileNamePath.c_str(), GlobalSettings::instance().getEnc()? encryptedDataToVector(encryptedData): data);}
            if (GlobalSettings::instance().getStealg()=="PNG-LSB"){
                QImage image(ContainerRoute);
                Linear_Image::Encode(&image, GlobalSettings::instance().getEnc()? encryptedDataToVector(encryptedData): data);
                image.save(fileName2);
            }
        });
        watcher->setFuture(future);
    }
    else if (Decode && GlobalSettings::instance().getMode()){
        std::string passwordCopy = passwordBytes.toStdString(); // 拷贝到 std::string
        //路径处理
#ifdef _WIN32
        std::string containerPath = Utils::wstringToUtf8(ContainerRoute.toStdWString());
#else
        std::string containerPath = ContainerRoute.toUtf8().toStdString();
#endif

        auto watcher = new QFutureWatcher<std::vector<uint8_t>>(this);
        auto &settings = GlobalSettings::instance();
        std::string stealg = settings.getStealg();

        if ((stealg == "PNG-LSB" && !Linear_Image::isPNG(containerPath.c_str()) && !settings.getJPGLSB()) ||
            (stealg == "JPG-DCT" && !DCT::isJPEG(containerPath.c_str()))) {
            QMessageBox::information(this, "Wrong", "Wrong image format!");
            return;
        }

        ui->pushButton_2->setEnabled(false);

        QFuture<std::vector<uint8_t>> future = QtConcurrent::run([=]() -> std::vector<uint8_t> {
            if (stealg == "PNG-LSB") {
                QImage image(ContainerRoute);
                return Linear_Image::Decode(image);
            } else if (stealg == "JPG-DCT") {
                std::vector<uint8_t> extractDataraw;
                DCT::decode_image(containerPath.c_str(), extractDataraw);
                return extractDataraw;
            }
            return {};
        });


        connect(watcher, &QFutureWatcher<std::vector<uint8_t>>::finished, this, [this, watcher, passwordCopy, File, Isstring]() {
            auto &settings = GlobalSettings::instance();
            std::vector<uint8_t> extractDataraw = watcher->result();

            if (settings.getEnc()) {
                const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(extractDataraw.data());
                if (!extractDataraw.empty()) {
                    extractDataraw = Encryption::dec(dataPtr, passwordCopy.c_str(), static_cast<int>(extractDataraw.size()));
                }
                if (extractDataraw.empty()) {
                    QMetaObject::invokeMethod(this, [this]() {
                        QMessageBox::information(this, "Fail", "Fail to decrypt!");
                    }, Qt::QueuedConnection);
                    QMetaObject::invokeMethod(this, [this]() {
                        ui->pushButton_2->setEnabled(true);
                    });
                    watcher->deleteLater();  // 释放watcher
                    return;
                }
            }

            QMetaObject::invokeMethod(this, [this, extractDataraw, File, Isstring]() {
                ui->pushButton_2->setEnabled(true);

                if (File && !Isstring) {
                    QByteArray byteData(reinterpret_cast<const char*>(extractDataraw.data()), static_cast<int>(extractDataraw.size()));
                    QString Filename;
                    int splitIndex = byteData.indexOf('|');
                    if (splitIndex != -1) {
                        Filename = QString::fromUtf8(byteData.left(splitIndex));
                        byteData = byteData.mid(splitIndex + 1);
                    }
                    else {
                        Filename = "default_filename"; // 防止 Filename 为空
                    }

                    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), Filename);
                    if (fileName.isEmpty()) {
                        return;  // 避免创建空文件
                    }

                    QFile file(fileName);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(byteData);
                    }
                    file.close();
                } else {
                    ui->TextIO->setText(QString::fromUtf8(reinterpret_cast<const char*>(extractDataraw.data()), static_cast<int>(extractDataraw.size())));
                }
            }, Qt::QueuedConnection);

            watcher->deleteLater();  // 释放watcher
        });
        watcher->setFuture(future);
    }
    else if (Encode && !GlobalSettings::instance().getMode()){
        QByteArray utf8Text = PlainText.toUtf8(); // 将QString转换为QByteArray
        if (File && !Isstring){
            QFile file(SecretRoute); // 替换为你的文件路径
            if (file.open(QIODevice::ReadOnly)) {
                utf8Text = file.readAll();
            }
            file.close();
            QFileInfo fileInfo(SecretRoute);
            QString fileName = fileInfo.fileName()+ "|";
            QByteArray byteArray = fileName.toUtf8();
            utf8Text = byteArray + utf8Text;
        }
        const std::vector<uint8_t> data(utf8Text.begin(), utf8Text.end()); // 使用QByteArray初始化std::vector
        const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(data.data());
        int dataLength = static_cast<int>(data.size());
        Encryption::EncryptedData encryptedData = Encryption::enc(dataPtr,passwordPtr,dataLength);
        QByteArray byteArray(reinterpret_cast<const char*>(encryptedDataToVector(encryptedData).data()), static_cast<int>(encryptedDataToVector(encryptedData).size()));
        if (File && !Isstring){
            QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"));
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)){
                qint64 byteswri = file.write(byteArray);
                if (byteswri == -1){
                    qDebug()<<"Error to write"<<file.errorString();
                }
                file.close();
            }
            QMessageBox::information(this,QString::fromStdString("Success"),QString::fromStdString("Success saved!"));
        }
        else{
            QByteArray base64Encoded = byteArray.toBase64();
            QString base64String = QString::fromUtf8(base64Encoded);
            ui->TextIO->setText(base64String);
        }
    }
    else if (Decode && !GlobalSettings::instance().getMode()){
        if (File && !Isstring){
            QFile file(SecretRoute); // 替换为你的文件路径
            QByteArray byteDatar;
            if (file.open(QIODevice::ReadOnly)) {
                byteDatar = file.readAll();
            }
            file.close();
            std::vector<uint8_t> extractDataraw(byteDatar.begin(),byteDatar.end());
            //const std::vector<uint8_t> *extractDataraw = reinterpret_cast<const uint8_t *>(byteDatar.constData());
            const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(extractDataraw.data());
            extractDataraw = Encryption::dec(dataPtr,passwordPtr,static_cast<int>(extractDataraw.size()));
            if (extractDataraw.size()<1){
                QMessageBox::information(this,QString::fromStdString("Fail"),QString::fromStdString("Invalid input!"));
                return;
            }
            QByteArray byteData(reinterpret_cast<const char*>(extractDataraw.data()), static_cast<int>(extractDataraw.size()));
            QString Filename = "";
            int splitIndex = byteData.indexOf('|'); // 查找 '|' 的位置
            if (splitIndex != -1) { // 如果找到 '|'
                Filename = QString::fromUtf8(byteData.left(splitIndex));
                byteData = byteData.mid(splitIndex + 1);
            }
            QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),Filename);
            QFile files(fileName);
            if (files.open(QIODevice::WriteOnly)) {
                files.write(byteData);
            }
            files.close();
            QMessageBox::information(this,QString::fromStdString("Success"),QString::fromStdString("Success saved!"));
        }
        else{
            QByteArray decodedData = QByteArray::fromBase64(PlainText.toUtf8());
            // 将QByteArray转换为std::vector<uint8_t>
            std::vector<uint8_t> dataVector(decodedData.begin(), decodedData.end());
            const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(dataVector.data());
            dataVector = Encryption::dec(dataPtr,passwordPtr,static_cast<int>(dataVector.size()));
            if (dataVector.size()<1){
                QMessageBox::information(this,QString::fromStdString("Fail"),QString::fromStdString("Invalid input!"));
                return;
            }
            ui->TextIO->setText(QString::fromUtf8(reinterpret_cast<const char*>(dataVector.data()), static_cast<int>(dataVector.size())));
        }
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

