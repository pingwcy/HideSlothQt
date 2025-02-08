#include "mainwindow.h"
#include "dialog.h"
#include "aboutbox.h"
#include "ui_mainwindow.h"
//#include "ui_aboutbox.h"
//#include "ui_dialog.h"
#include <QMessageBox>
#include <string>
//#include <iostream>
//#include <fstream>
#include <QLabel>
#include "Linear_Image.cpp"
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QByteArray>
#include <QtConcurrent>
#include <QDir>
#include "Encryption.cpp"
#include <QDialog>
#include "GlobalSettings.h"
#include <DCT.cpp>
#include <dctreader.h>
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

void MainWindow::on_pushButton_2_clicked()
{
    // 获取 QLineEdit 的文本
    QString Password = ui->Password_Text->text();
    QByteArray passwordBytes = Password.toUtf8(); // 将QString转换为QByteArray
    const char* passwordPtr = passwordBytes.constData(); // 获取C风格字符串指针
    QString SecretRoute = ui-> Secret_Text->text();
    QString ContainerRoute = ui->Container_Text->text();
    QString PlainText = ui->TextIO->toPlainText();
    // 检查 radio buttons 的状态
    bool Encode = ui->radioButton->isChecked();
    bool Decode = ui->radioButton_2->isChecked();
    bool File = ui->radioButton_3->isChecked();
    bool Isstring = ui->radioButton_4->isChecked();
    // 根据需要处理这些值
    if (Encode && GlobalSettings::instance().getMode()){
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
        if (GlobalSettings::instance().getEnc()){
            Encryption::EncryptedData encryptedData = Encryption::enc(dataPtr,passwordPtr,dataLength);
            if (GlobalSettings::instance().getStealg()=="PNG-LSB"){
                if (!Linear_Image::isPNG(ContainerRoute.toLocal8Bit().toStdString()) && !GlobalSettings::instance().getJPGLSB()){
                    QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                    return;
                }
                QImage image(ContainerRoute);
                QtConcurrent::run(&Linear_Image::Encode, &image, encryptedDataToVector(encryptedData));
                QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"));
                image.save(fileName,"PNG");
            }
            if (GlobalSettings::instance().getStealg()=="JPG-DCT"){
                if (!DCT::isJPEG((ContainerRoute.toLocal8Bit().toStdString()))){
                    QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                    return;
                }
                QString fileName2 = QFileDialog::getSaveFileName(this,tr("Save File"));
                DCT::encode_image(ContainerRoute.toLocal8Bit().constData(), fileName2.toLocal8Bit().constData(),encryptedDataToVector(encryptedData));
            }
        }
        else {
            if (GlobalSettings::instance().getStealg()=="PNG-LSB" && !GlobalSettings::instance().getJPGLSB()){
                if (!Linear_Image::isPNG(ContainerRoute.toLocal8Bit().toStdString())){
                    QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                    return;
                }
                QImage image(ContainerRoute);
                QtConcurrent::run(&Linear_Image::Encode, &image, data);
                QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"));
                image.save(fileName,"PNG");
            }
            if (GlobalSettings::instance().getStealg()=="JPG-DCT"){
                if (!DCT::isJPEG((ContainerRoute.toLocal8Bit().toStdString()))){
                    QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                    return;
                }
                QString fileName2 = QFileDialog::getSaveFileName(this,tr("Save File"));
                DCT::encode_image(ContainerRoute.toLocal8Bit().constData(), fileName2.toLocal8Bit().constData(),data);
            }
        }
        QMessageBox::information(this,QString::fromStdString("Success"),QString::fromStdString("Succes Encode and save!"));
    }
    else if (Decode && GlobalSettings::instance().getMode()){
        std::vector<uint8_t> extractDataraw;
        if (GlobalSettings::instance().getStealg()=="PNG-LSB"){
            if (!Linear_Image::isPNG(ContainerRoute.toLocal8Bit().toStdString()) && !GlobalSettings::instance().getJPGLSB()){
                QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                return;
            }
            QImage image(ContainerRoute);
            extractDataraw = Linear_Image::Decode(image);
        }
        if (GlobalSettings::instance().getStealg()=="JPG-DCT"){
            if (!DCT::isJPEG((ContainerRoute.toLocal8Bit().toStdString()))){
                QMessageBox::information(this,QString::fromStdString("Wrong"),QString::fromStdString("Wrong image format!"));
                return;
            }
            DCT::decode_image(ContainerRoute.toLocal8Bit().constData(), extractDataraw);
        }
        if (GlobalSettings::instance().getEnc()){
            const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(extractDataraw.data());
            extractDataraw = Encryption::dec(dataPtr,passwordPtr,static_cast<int>(extractDataraw.size()));
            if (extractDataraw.size()<1){
                QMessageBox::information(this,QString::fromStdString("Fail"),QString::fromStdString("Fail to decrypt!"));
                //extractDataraw.clear();
                return;
            }
        }
        if (File && !Isstring){
            QByteArray byteData(reinterpret_cast<const char*>(extractDataraw.data()), static_cast<int>(extractDataraw.size()));
            QString Filename = "";
            int splitIndex = byteData.indexOf('|'); // 查找 '|' 的位置
            if (splitIndex != -1) { // 如果找到 '|'
                Filename = QString::fromUtf8(byteData.left(splitIndex));
                byteData = byteData.mid(splitIndex + 1);
            }
            QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),Filename);
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(byteData);
            }
            file.close();
            //extractDataraw.clear();
        }
        else{
            ui->TextIO->setText(QString::fromUtf8(reinterpret_cast<const char*>(extractDataraw.data()), static_cast<int>(extractDataraw.size())));
            //extractDataraw.clear();
        }
        QMessageBox::information(this,QString::fromStdString("Success"),QString::fromStdString("Success Decode!"));
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
    if (Linear_Image::isPNG(ContainerRoute.toStdString())){
    ImageCapa = Linear_Image::CheckSize(image);
    Capainfo += "This container's capacity is "+QString::number(ImageCapa)+" KB. ";
    }
    else if (DCT::isJPEG(ContainerRoute.toStdString())){
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

