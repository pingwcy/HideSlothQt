#include "bulk_encode.h"
#include "ui_bulk_encode.h"
#include "dialog.h"

#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDataStream>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

#include "DCT.cpp"
#include "Linear_Image.cpp"
#include "databaseviewer.h"
#include "GlobalSettings.h"
#include "Encryption.cpp"
#include "utils_a.h"

void bulk_encode::loadSettings() {
    auto &settings = GlobalSettings::instance();
    QString info;
    if (settings.getMode()){
        info += "Mode: Stego;  ";
    }
    else{
        info += "Mode: Encryptor;  ";
    }
    if (settings.getEnc()){
        info += "Encryption: ON;  ";
    }
    else{
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
bulk_encode::bulk_encode(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::bulk_encode)
{
    ui->setupUi(this);
    ui->pushButton_5->setEnabled(false);
    loadSettings();
}

bulk_encode::~bulk_encode()
{
    delete ui;
}

void bulk_encode::on_pushButton_8_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, "Select Containers route",  QDir::homePath(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    ui->lineEdit_2->setText(dir);
}

// 读取一个 int64（转换为小端序）
QByteArray int64ToLittleEndian(qint64 value) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << value;
    return data;
}

void bulk_encode:: processFileChunks(const QString &filePath, QString dbpath, std::string passwordStr, QString outroute) {
    QSqlDatabase db;
    QString connectionName = "LocalDBConnection";
    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }

    db.setDatabaseName(dbpath);
    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开文件：" << file.errorString();
        return;
    }

    qint64 fileSize = file.size(); // 获取文件大小
    qint64 offset = 0; // 当前位置偏移量

    QSqlQuery query(db);
    query.prepare("SELECT * FROM Images ORDER BY id ASC");  // 查询所有块大小

    if (!query.exec()) {
        qDebug() << "数据库查询失败：" << query.lastError().text();
        return;
    }

    while (query.isActive() && query.next()) {
        //qDebug() << "Query active: " << query.isActive();
        //qDebug() << "Query valid: " << query.isValid();
        //qDebug() << "Database open: " << db.isOpen();

        qint64 chunkSize = query.value(4).toLongLong()*1024;  // 获取数据库中的块大小
        //qDebug() << "处理数据块，偏移量:";

        if (chunkSize <= 0) break;  // 无效块大小，终止

        file.seek(offset); // 移动到当前偏移位置
        QByteArray fileChunk = file.read(chunkSize); // 读取数据

        if (fileChunk.isEmpty()) break; // 读取结束

        // 生成小端序的 offset 和 fileSize
        QByteArray offsetBytes = int64ToLittleEndian(offset);
        QByteArray fileSizeBytes = int64ToLittleEndian(fileSize);

        // 合并数据 (offset + fileSize + 读取的内容)
        QByteArray finalData = offsetBytes + fileSizeBytes + fileChunk;

        // 对 finalData 进行处理（这里可以换成你的逻辑）
        qDebug() << "处理数据块，偏移量:" << offset << " 块大小:" << chunkSize;


        //将读取内容转换为向量
        const std::vector<uint8_t> data(finalData.begin(), finalData.end()); // 使用QByteArray初始化std::vector
        //获取数据指针，为了加密需要
        const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(data.data());
        //获取长度，为了加密需要
        int dataLength = static_cast<int>(data.size());
        //初始化加密后的结构体
        Utils::EncryptedData encryptedData;
        //如果需要加密则执行
        if (GlobalSettings::instance().getEnc()){
            encryptedData = Encryption::enc(dataPtr,passwordStr.c_str(),dataLength);
        }
        //路径处理，避免Windows下toutf8非ASC字符乱码，tolocal8bits不可靠，先处理为wstring，再用Windows提供的API转换
        QString ContainerRoute = query.value(1).toString()+"/"+query.value(2).toString();
#ifdef _WIN32
        const std::string path = Utils::wstringToUtf8(ContainerRoute.toStdWString());
#else
        const std::string path = ContainerRoute.toUtf8().toStdString();
#endif
        //获取指定输出文件名
        QString fileName2 = outroute+"/"+query.value(2).toString();
        //路径处理
#ifdef _WIN32
        std::string containerPath = Utils::wstringToUtf8(ContainerRoute.toStdWString());
        std::string fileNamePath = Utils::wstringToUtf8(fileName2.toStdWString());
#else
        std::string containerPath = ContainerRoute.toUtf8().toStdString();
        std::string fileNamePath = fileName2.toUtf8().toStdString();
#endif

        //不同算法 是否加密
        if (query.value(3).toString()=="JPG"){
            DCT::encode_image(containerPath.c_str(), fileNamePath.c_str(), GlobalSettings::instance().getEnc()? Utils::encryptedDataToVector(encryptedData): data);}
        else if (query.value(3).toString()=="PNG"){
            QImage image(ContainerRoute);
            if (image.isNull()) {
                qDebug() << "Fail to load PNG: " << ContainerRoute;
                continue;
            }
            Linear_Image::Encode(&image, GlobalSettings::instance().getEnc()? Utils::encryptedDataToVector(encryptedData): data);
            image.save(fileName2);
        }
        else {
        qDebug() << "Non-Image File";
        }
        // 更新偏移量
        offset += fileChunk.size();
        //qDebug() <<offset<<"  "<<fileSize;
        if (offset >= fileSize) break; // 读取到文件末尾
    }
    db.close();
    QSqlDatabase::removeDatabase(connectionName);
    file.close();
}

void bulk_encode::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false); // 禁用按钮，防止重复点击
    ui->progressBar->setMaximum(0);
    QString secFilePath = ui->lineEdit->text();
    QString dirPath = ui->lineEdit_2->text();
    auto &settings2 = GlobalSettings::instance();
    int setcap = settings2.getBulkmin();

    QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);
    connect(watcher, &QFutureWatcher<int>::finished, this, [this, watcher]() {
        ui->progressBar->setMaximum(1);
        int sum2 = watcher->result();
        watcher->deleteLater();

        ui->pushButton->setEnabled(true);

        if (sum2 > 0) {
            ui->pushButton_5->setEnabled(true);
            QMessageBox::information(this, "Success", "The container capacity is enough!");
        } else {
            ui->pushButton_5->setEnabled(false);
            QMessageBox::critical(this, "No enough capacity", "The container capacity is NOT enough!");
        }
    });

    QFuture<int> future = QtConcurrent::run([secFilePath, dirPath, setcap]() -> int {
        QFile secfile(secFilePath);
        int fileSize = 0;
        if (secfile.exists()) {
            fileSize = secfile.size() / 1024;
            secfile.close();
        }

        QDir dir(dirPath);
        if (!dir.exists()) {
            qDebug() << "Directory does not exist:" << dirPath;
            return 0;
        }

        QStringList fileList = dir.entryList(QDir::Files);

        QSqlDatabase db(QSqlDatabase::addDatabase("QSQLITE"));
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/images.db");
        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
            return 0;
        }

        QSqlQuery query(db);
        query.exec("DROP TABLE IF EXISTS Images");
        query.exec("CREATE TABLE Images (ID INTEGER PRIMARY KEY AUTOINCREMENT, Path TEXT, Filename TEXT, FType TEXT, Capacity TEXT)");
        db.transaction();

        int count = 0;
        for (const QString &fileName : fileList) {
            QString filePath = dir.absoluteFilePath(fileName);
            bool isPNG = Linear_Image::isPNG(filePath.toStdString());
            bool isJPEG = DCT::isJPEG(filePath.toStdString());

            if (isPNG || isJPEG) {
                query.prepare("INSERT INTO Images (Path, Filename, FType, Capacity) VALUES (?, ?, ?, ?)");
                query.addBindValue(dirPath);
                query.addBindValue(fileName);
                QImage items(filePath);
                int itemcap = 0;

                if (isPNG) {
                    query.addBindValue("PNG");
                    itemcap = Linear_Image::CheckSize(items);
                } else if (isJPEG) {
                    query.addBindValue("JPG");
                    itemcap = (items.height() * items.width() / 64) * 1.5 / 8 / 1024 * 0.95;
                }

                if (itemcap < setcap) continue;
                query.addBindValue(itemcap);

                if (query.exec()) {
                    count++;
                } else {
                    qDebug() << "Failed to insert data:" << query.lastError().text();
                }
            }
        }
        db.commit();

        int sum2 = 0;
        if (query.exec("SELECT SUM(Capacity) FROM Images") && query.next()) {
            sum2 = query.value(0).toInt();
        }
        db.close();
        return (sum2 > fileSize) ? sum2 : 0;
    });

    watcher->setFuture(future);
}



void bulk_encode::on_pushButton_7_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"));
    ui->lineEdit->setText(fileName);
}


void bulk_encode::on_pushButton_2_clicked()
{
    DatabaseViewer *bken3 = new DatabaseViewer();
    bken3->setAttribute(Qt::WA_DeleteOnClose);
    bken3->show();
}


void bulk_encode::on_pushButton_3_clicked()
{
    Dialog *dialog = new Dialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();

}



void bulk_encode::on_pushButton_4_clicked()
{
    loadSettings();
}


void bulk_encode::on_pushButton_5_clicked()
{
    // 禁用按钮防止重复点击
    ui->pushButton_5->setEnabled(false);
    ui->progressBar->setMaximum(0);
    // 获取 密码框 的文本
    QString Password = ui->lineEdit_3->text();
    QByteArray passwordBytes = Password.toUtf8(); // UTF8将QString转换为QByteArray
    std::string passwordStr = passwordBytes.toStdString(); // 确保数据有效避免悬挂

    QString outr = ui->lineEdit_4->text();
    QString dbPath = QCoreApplication::applicationDirPath() + "/images.db";
    QString inputPath = ui->lineEdit->text();

    // 运行在后台线程，避免主线程卡死
    QtConcurrent::run([=]() {
        processFileChunks(inputPath, dbPath, passwordStr, outr);

        // 回到主线程更新UI
        QMetaObject::invokeMethod(ui->pushButton_5, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
        QMetaObject::invokeMethod(this, [=]() {
            ui->progressBar->setMaximum(1);
            QMessageBox::information(nullptr, "Done", "Buld Encode Done!");
        }, Qt::QueuedConnection);
    });
}


void bulk_encode::on_pushButton_9_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, "Select Containers route",  QDir::homePath(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    ui->lineEdit_4->setText(dir);

}

