#include "mainwindow.h"


#include "dialog.h"


#include "ui_mainwindow.h"


//#include "ui_dialog.h"


#include <QMessageBox>


#include <string>


#include <iostream>


#include <fstream>


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


        QImage image(ContainerRoute);


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


            QtConcurrent::run(&Linear_Image::Encode, &image, encryptedDataToVector(encryptedData));


        }


        else {


            QtConcurrent::run(&Linear_Image::Encode, &image, data);


        }


        QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"));


        image.save(fileName,"PNG");


        QMessageBox::information(this,QString::fromStdString("Success"),QString::fromStdString("Succes Encode and save!"));


    }


    else if (Decode && GlobalSettings::instance().getMode()){


        QImage image(ContainerRoute);


        std::vector<uint8_t> extractDataraw = Linear_Image::Decode(image);


        if (GlobalSettings::instance().getEnc()){


            const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(extractDataraw.data());


            extractDataraw = Encryption::dec(dataPtr,passwordPtr,static_cast<int>(extractDataraw.size()));


        }


        if (File && !Isstring){


            QByteArray byteData(reinterpret_cast<const char*>(extractDataraw.data()), extractDataraw.size());


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


        }


        else{


            ui->TextIO->setText(QString::fromUtf8(reinterpret_cast<const char*>(extractDataraw.data()), extractDataraw.size()));


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


        QByteArray byteArray(reinterpret_cast<const char*>(encryptedDataToVector(encryptedData).data()), encryptedDataToVector(encryptedData).size());


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


            QByteArray byteData(reinterpret_cast<const char*>(extractDataraw.data()), extractDataraw.size());


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


            ui->TextIO->setText(QString::fromUtf8(reinterpret_cast<const char*>(dataVector.data()), dataVector.size()));


        }


    }


}


void MainWindow::on_Check_Button_clicked()


{


    QString Capainfo = "";


    QString SecretRoute = ui-> Secret_Text->text();


    QString ContainerRoute = ui->Container_Text->text();


    QImage image(ContainerRoute);


    double ImageCapa = Linear_Image::CheckSize(image);


    Capainfo += "This container's capacity is "+QString::number(ImageCapa)+" KB. ";


    QFile file(SecretRoute);


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


{/*


    if(!setting){


        QDialog dialog(this);


        dialog.exec();


    }*/


}


void MainWindow::on_adv_btn_clicked()


{


    Dialog *dialog = new Dialog();


    dialog->exec();


}


void MainWindow::on_actionAdvanced_Settings_triggered()


{


    Dialog *dialog = new Dialog();


    dialog->exec();


}


void MainWindow::on_actionExit_triggered()


{


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


