#include "bulk_decode.h"
#include "dialog.h"
#include "ui_bulk_decode.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent>

#include "DCT.cpp"
#include "Encryption.cpp"
#include "GlobalSettings.h"
#include "Linear_Image.cpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
void bulk_decode::loadSettings() {
    auto &settings = GlobalSettings::instance();
    QString info;
    if (settings.getMode()) {
        info += "Mode: Stego;  ";
    } else {
        info += "Mode: Encryptor;  ";
    }
    if (settings.getEnc()) {
        info += "Encryption: ON;  ";
    } else {
        info += "Encryption: OFF;  ";
    }
    if (settings.getKDF() == "PBKDF2") {
        if (settings.getCstHash()) {
            info += "Hash Alg: ";
            info += QString::fromStdString(settings.getHash());
            info += ";  ";
        } else {
            info += "Hash Alg: ";
            info += QString::fromStdString(settings.getDefhash());
            info += ";  ";
        }

        if (settings.getCstIter()) {
            info += "Hash Iter: ";
            info += QString::fromStdString(std::to_string(settings.getIter()));
            info += ";  ";
        } else {
            info += "Hash Iter: ";
            info += QString::fromStdString(std::to_string(settings.getDefIter()));
            info += ";  ";
        }
    } else if (settings.getKDF() == "Argon2id") {
        info += "Argon2id; ";
    }
    info += "Encryption alg: ";
    info += QString::fromStdString(settings.getEncalg());
    info += ";  Stego alg: ";
    info += QString::fromStdString(settings.getStealg());
    ui->textEdit->setText(info);
}

bulk_decode::bulk_decode(QWidget *parent)
    : QWidget(parent), ui(new Ui::bulk_decode) {
    ui->setupUi(this);
    loadSettings();
}

bulk_decode::~bulk_decode() { delete ui; }

int64_t readInt64LE(const uint8_t *data) {
    int64_t value;
    std::memcpy(&value, data, sizeof(int64_t));
    return value;
}

void bulk_decode::on_pushButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(
        nullptr, "Select Loaded Containers route", QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->lineEdit->setText(dir);
}

void bulk_decode::on_pushButton_2_clicked() {
    QString fileName2 = QFileDialog::getSaveFileName(this, tr("Save File"));
    ui->lineEdit_2->setText(fileName2);
}

void bulk_decode::on_pushButton_3_clicked() {
    ui->pushButton_3->setEnabled(false);
    ui->progressBar->setMaximum(0);
    QString Password = ui->lineEdit_3->text();
    QByteArray passwordBytes = Password.toUtf8(); // UTF8将QString转换为QByteArray
    std::string passwordStr = passwordBytes.toStdString(); // 确保数据有效避免悬挂
    std::string outputFilePath = ui->lineEdit_2->text().toStdString();
    auto &settings = GlobalSettings::instance();
    bool isench = settings.getEnc();
    QString dirPath = ui->lineEdit->text();

    QtConcurrent::run([=]() {
        processdecode(passwordStr, outputFilePath, dirPath, isench);
        // 回到主线程更新UI
        QMetaObject::invokeMethod(ui->pushButton_3, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
        QMetaObject::invokeMethod(this, [=]() {
            ui->progressBar->setMaximum(1);
            QMessageBox::information(nullptr, "Done", "Buld Decode Done!");
        }, Qt::QueuedConnection);
    });
}

void bulk_decode:: processdecode(std::string passwordStr, std::string outputFilePath, QString dirPath, bool isench)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        QMetaObject::invokeMethod(
            this,
            [this, dirPath]() {
                QMessageBox::warning(this, "Error",
                                     "Directory does not exist: " + dirPath);
            },
            Qt::QueuedConnection);
        qDebug() << "Directory does not exist:" << dirPath;
        return;
    }

    // 只获取文件，排除子目录
    QStringList fileList = dir.entryList(QDir::Files);
    for (const QString &fileName : fileList) {
        std::vector<uint8_t> extractDataraw;
        QString fullname = dirPath + "/" + fileName;
        qDebug() << fullname;
        if (DCT::isJPEG(fullname.toStdString())) {
            DCT::decode_image(fullname.toStdString().c_str(), extractDataraw);
        } else if (Linear_Image::isPNG(fullname.toStdString())) {
            QImage image(fullname);
            if (image.isNull()) {
                qDebug() << "Error: Failed to load PNG file:" << fullname;
                continue;
            }
            extractDataraw = Linear_Image::Decode(image);
        }
        if (isench) {
            const unsigned char *dataPtr =
                reinterpret_cast<const unsigned char *>(extractDataraw.data());
            if (!extractDataraw.empty()) {
                extractDataraw =
                    Encryption::dec(dataPtr, passwordStr.c_str(),
                                                 static_cast<int>(extractDataraw.size()));
            }
            if (extractDataraw.empty()) {
                QMetaObject::invokeMethod(
                    this,
                    [this]() {
                        QMessageBox::information(this, "Fail", "Fail to decrypt!");
                    },
                    Qt::QueuedConnection);
                QMetaObject::invokeMethod(
                    this, [this]() { ui->pushButton_2->setEnabled(true); });
                return;
            }
        }
        std::map<int64_t, std::vector<uint8_t>> chunkMap;
        int64_t fileSize = -1;
        bool fileSizeSet = false;
        qDebug() << "extractDataraw.size:" << extractDataraw.size();

        size_t offset = 0;
        while (offset + 16 <= extractDataraw.size()) { // 确保至少16字节
            int64_t chunkOffset = readInt64LE(&extractDataraw[offset]);
            int64_t currentFileSize = readInt64LE(&extractDataraw[offset + 8]);
            qDebug() << "Parsed chunkOffset:" << chunkOffset;
            qDebug() << "currentFileSize:" << currentFileSize;

            // 过滤无效 offset
            if (chunkOffset < 0) {
                std::cerr << "Error: Invalid chunkOffset: " << chunkOffset << std::endl;
                break;
            }

            if (!fileSizeSet) {
                fileSize = currentFileSize;
                fileSizeSet = true;
            }

            // 计算当前数据块的起始地址
            size_t dataStart = offset + 16;
            if (dataStart >= extractDataraw.size()) {
                std::cerr << "Error: Data start position out of range!" << std::endl;
                break;
            }

            // 查找下一个块的 offset
            size_t nextOffset = offset + 16;
            int64_t nextChunkOffset = fileSize; // 默认为文件大小
            while (nextOffset + 16 <= extractDataraw.size()) {
                int64_t possibleOffset = readInt64LE(&extractDataraw[nextOffset]);
                if (possibleOffset > chunkOffset) {
                    nextChunkOffset = possibleOffset;
                    break;
                }
                nextOffset += 16;
            }

            // 计算数据块范围
            size_t chunkSize = nextChunkOffset - chunkOffset;
            size_t dataEnd = dataStart + chunkSize;
            if (dataEnd > extractDataraw.size()) {
                dataEnd = extractDataraw.size();
            }

            if (dataStart >= dataEnd) {
                std::cerr << "Error: Invalid data range! dataStart: " << dataStart
                          << ", dataEnd: " << dataEnd << std::endl;
                break;
            }

            // 提取数据
            std::vector<uint8_t> data(extractDataraw.begin() + dataStart,
                                      extractDataraw.begin() + dataEnd);
            chunkMap[chunkOffset] = data;

            // 正确更新 offset
            offset += 16 + data.size();
        }

        if (fileSize == -1) {
            std::cerr << "Error: Cannot determine the size of file!" << std::endl;
            return;
        }
        // 先以 std::ios::app（追加模式）打开文件，确保文件存在
        std::ofstream tempFile(outputFilePath, std::ios::app);
        tempFile.close();

        // 逐块写入文件
        std::ofstream outFile(outputFilePath,
                              std::ios::binary | std::ios::in | std::ios::out);
        if (!outFile) {
            std::cerr << "Error: Cannot create output file: " << outputFilePath
                      << std::endl;
            return;
        }

        for (const auto &[chunkOffset, data] : chunkMap) {
            outFile.seekp(chunkOffset); // 使用 seekp 定位到正确位置
            if (outFile.fail()) {
                std::cerr << "Error: seekp() failed at offset: " << chunkOffset
                          << std::endl;
                break;
            }
            outFile.write(reinterpret_cast<const char *>(data.data()), data.size());
            if (outFile.fail()) {
                std::cerr << "Error: Write failed at offset: " << chunkOffset
                          << std::endl;
                break;
            }
        }

        outFile.close();
    }
    std::cout << "File rebuild completed! " << outputFilePath << std::endl;
}

void bulk_decode::on_pushButton_4_clicked() {
    Dialog *dialog = new Dialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void bulk_decode::on_pushButton_5_clicked() { loadSettings(); }
