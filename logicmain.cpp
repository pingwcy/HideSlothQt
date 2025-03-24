#include "logicmain.h"
#include <QString>
#include <utils_a.h>
#include <Encryption.cpp>
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QImage>
#include "Linear_Image.cpp"
#include "DCT.cpp"
#include <QAction>
#include <QTextEdit>

LogicMain::LogicMain() {

}
namespace logic{
int CHUNK_SIZE (GlobalSettings::instance().getChunks() * 1024 * 1024); // 10MB Default
void encode_logic(const QString& PlainText, const QString& SecretRoute, bool Isstring, bool File, const QString& ContainerRoute, const std::string& passwordStr, QString fileName2, QWidget* parent)
{
    QByteArray utf8Text = PlainText.toUtf8();

    if (File && !Isstring){
        QFile file(SecretRoute);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(parent, "Error", "Failed to open the file: " + file.errorString());
            return;
        }
        utf8Text = file.readAll();
        file.close();
        // 添加文件名到内容头部
        QFileInfo fileInfo(SecretRoute);
        QString fileName = fileInfo.fileName() + "|";
        QByteArray byteArray = fileName.toUtf8();
        utf8Text = byteArray + utf8Text;
    }

    const std::vector<uint8_t> data(utf8Text.begin(), utf8Text.end());
    const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(data.data());
    int dataLength = static_cast<int>(data.size());

    Utils::EncryptedData encryptedData;
    if (GlobalSettings::instance().getEnc()){
        encryptedData = Encryption::enc(dataPtr, passwordStr.c_str(), dataLength);
    }

    std::string path = (GlobalSettings::instance().getEnc()) ? Utils::wstringToUtf8(ContainerRoute.toStdWString()) : ContainerRoute.toUtf8().toStdString();

    if (GlobalSettings::instance().getStealg() == "PNG-LSB"){
        if (!Linear_Image::isPNG(path) && !GlobalSettings::instance().getJPGLSB()){
            QMessageBox::information(parent, "Error", "Wrong image format!");
            return;
        }
    }

    if (GlobalSettings::instance().getStealg() == "JPG-DCT"){
        if (!DCT::isJPEG(path)){
            QMessageBox::information(parent, "Error", "Wrong image format!");
            return;
        }
    }

    std::string containerPath = Utils::wstringToUtf8(ContainerRoute.toStdWString());
    std::string fileNamePath = Utils::wstringToUtf8(fileName2.toStdWString());

    if (GlobalSettings::instance().getStealg() == "JPG-DCT"){
        DCT::encode_image(containerPath.c_str(), fileNamePath.c_str(), GlobalSettings::instance().getEnc() ? Utils::encryptedDataToVector(encryptedData) : data);
    }

    if (GlobalSettings::instance().getStealg() == "PNG-LSB"){
        QImage image(ContainerRoute);
        Linear_Image::Encode(&image, GlobalSettings::instance().getEnc() ? Utils::encryptedDataToVector(encryptedData) : data);
        image.save(fileName2);
    }

    // 通知成功
    //QMessageBox::information(parent, "Success", "Processing completed successfully!");
}

void decode_logic(const QString& ContainerRoute, const std::string& passwordStr, bool File, bool Isstring, QWidget* parent)
{
// 路径处理
#ifdef _WIN32
    std::string containerPath = Utils::wstringToUtf8(ContainerRoute.toStdWString());
#else
    std::string containerPath = ContainerRoute.toUtf8().toStdString();
#endif
    auto& settings = GlobalSettings::instance();
    std::string stealg = settings.getStealg();
    std::vector<uint8_t> extractDataraw;

    if ((stealg == "PNG-LSB" && !Linear_Image::isPNG(containerPath.c_str()) && !settings.getJPGLSB()) ||
        (stealg == "JPG-DCT" && !DCT::isJPEG(containerPath.c_str()))) {
        QMetaObject::invokeMethod(parent, [=]() {
            QMessageBox::information(parent, "Wrong", "Wrong image format!");
        }, Qt::QueuedConnection);

        return;
    }

    if (stealg == "PNG-LSB") {
        QImage image(ContainerRoute);
        extractDataraw = Linear_Image::Decode(image);
    } else if (stealg == "JPG-DCT") {
        DCT::decode_image(containerPath.c_str(), extractDataraw);
    }

    if (settings.getEnc()) {
        const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(extractDataraw.data());
        if (!extractDataraw.empty()) {
            extractDataraw = Encryption::dec(dataPtr, passwordStr.c_str(), static_cast<int>(extractDataraw.size()));
        }
        if (extractDataraw.empty()) {
            QMetaObject::invokeMethod(parent, [=]() {
                QMessageBox::information(parent, "Fail", "Fail to decrypt!");
            }, Qt::QueuedConnection);
            return;
        }
    }

    if (File && !Isstring) {
        QByteArray byteData(reinterpret_cast<const char*>(extractDataraw.data()), static_cast<int>(extractDataraw.size()));
        QString Filename;
        int splitIndex = byteData.indexOf('|');
        if (splitIndex != -1) {
            Filename = QString::fromUtf8(byteData.left(splitIndex));
            byteData = byteData.mid(splitIndex + 1);
        }
        else {
            Filename = "default_filename";
        }
        // 创建一个 promise 以存储文件名
        std::promise<QString> promise;
        std::future<QString> futureFileName = promise.get_future();

        QMetaObject::invokeMethod(parent, [=, &promise]() {
            QString fileName = QFileDialog::getSaveFileName(parent, "Save File", Filename);
            promise.set_value(fileName); // 将获取的文件名存入 promise
        }, Qt::QueuedConnection);

        // 在子线程等待获取的文件名
        QString fileName = futureFileName.get();

        if (fileName.isEmpty()) {
            return;
        }

        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(byteData);
        }
        file.close();
    } else {
        QTextEdit* textEdit = parent->findChild<QTextEdit*>("TextIO");
        if (textEdit) {
            QMetaObject::invokeMethod(parent, [=]() {
                textEdit->setText(QString::fromUtf8(reinterpret_cast<const char*>(extractDataraw.data()), static_cast<int>(extractDataraw.size())));
            }, Qt::QueuedConnection);

        }
    }
}



QString encryptString(QString PlainText, std::string pwd){
    QByteArray utf8Text = PlainText.toUtf8();
    std::vector<uint8_t> data(utf8Text.begin(), utf8Text.end());
    const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(data.data());
    int dataLength = static_cast<int>(data.size());

    Utils::EncryptedData encryptedData = Encryption::enc(dataPtr, pwd.c_str(), dataLength);

    std::vector<uint8_t> encryptedVector = Utils::encryptedDataToVector(encryptedData);
    QByteArray byteArray(reinterpret_cast<const char*>(encryptedVector.data()),
                         static_cast<int>(encryptedVector.size()));

    QString base64String = QString::fromUtf8(byteArray.toBase64());
    return base64String;
}
QString decryptString(QString PlainText, std::string pwd){
    QByteArray decodedData = QByteArray::fromBase64(PlainText.toUtf8());
    // 将QByteArray转换为std::vector<uint8_t>
    std::vector<uint8_t> dataVector(decodedData.begin(), decodedData.end());
    const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(dataVector.data());
    dataVector = Encryption::dec(dataPtr,pwd.c_str(),static_cast<int>(dataVector.size()));
    if (dataVector.size()<1){
        //QMessageBox::information(this,QString::fromStdString("Fail"),QString::fromStdString("Invalid input!"));
        return "";
    }
    return QString::fromUtf8(reinterpret_cast<const char*>(dataVector.data()), static_cast<int>(dataVector.size()));

}
void encryptFile(const QString &inputPath, const QString &outputPath, std::string pwd) {
    QFile inputFile(inputPath);
    QFile outputFile(outputPath);
    int CHUNK_SIZE (GlobalSettings::instance().getChunks() * 1024 * 1024); // 10MB Default

    if (!inputFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open input file!" << inputFile.errorString();
        return;
    }

    if (!outputFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Cannot open output file!" << outputFile.errorString();
        return;
    }

    QDataStream in(&inputFile);
    QDataStream out(&outputFile);

    QByteArray buffer;
    while (!in.atEnd()) {
        buffer = inputFile.read(CHUNK_SIZE);

        std::vector<uint8_t> data(buffer.begin(), buffer.end());
        const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(data.data());
        int dataLength = static_cast<int>(data.size());

        Utils::EncryptedData encryptedData = Encryption::enc(dataPtr, pwd.c_str(), dataLength);

        std::vector<uint8_t> encryptedVector = Utils::encryptedDataToVector(encryptedData);
        QByteArray processedData(reinterpret_cast<const char*>(encryptedVector.data()),
                                 static_cast<int>(encryptedVector.size()));

        out.writeRawData(processedData.constData(), processedData.size());

        qDebug() << "Processed chunk of size:" << buffer.size();
    }

    inputFile.close();
    outputFile.close();

    qDebug() << "Encryption Done!";
}
void decryptFile(const QString &SecretRoute, const QString &fileName, std::string pwd){
    QFile inputFile(SecretRoute);
    QFile outputFile(fileName);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Can not open input file!" << inputFile.errorString();
        return;
    }

    if (!outputFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Can not open output file!" << outputFile.errorString();
        return;
    }
    QDataStream in(&inputFile);
    QDataStream out(&outputFile);

    QByteArray buffer;
    while (!in.atEnd()) {
        buffer = inputFile.read(CHUNK_SIZE+44); // 读取10MB数据

        std::vector<uint8_t> dataVector(buffer.begin(), buffer.end());
        const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(dataVector.data());
        dataVector = Encryption::dec(dataPtr, pwd.c_str(), static_cast<int>(dataVector.size()));

        // 将处理后的数据写入到新文件
        out.writeRawData(reinterpret_cast<const char*>(dataVector.data()), static_cast<int>(dataVector.size()));

        qDebug() << "Processed chunk of size:" << buffer.size();
    }

    inputFile.close();
    outputFile.close();

    qDebug() << "Decryption Down!";

}
}

