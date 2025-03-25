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
    if (settings.getCstHash()){
        info += "Hash Alg: ";
        info += QString::fromStdString(settings.getHash());
        info += ";  ";
    }
    else{
        info += "Hash Alg: ";
        info += QString::fromStdString(settings.getDefhash());
        info += ";  ";
    }

    if (settings.getCstIter()){
        info += "Hash Iter: ";
        info += QString::fromStdString(std::to_string(settings.getIter()));
        info += ";  ";
    }
    else{
        info += "Hash Iter: ";
        info += QString::fromStdString(std::to_string(settings.getDefIter()));
        info += ";  ";
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
    QFile secfile(ui->lineEdit->text());
    int fileSize = 0;
    if (secfile.exists()) {
        qint64 size = secfile.size(); // 获取文件大小
        fileSize = size/1024;
        secfile.close();
    }

    auto &settings2 = GlobalSettings::instance();
    QString dirPath = ui->lineEdit_2->text();
    QDir dir(dirPath);
    if (!dir.exists()) {
        qDebug() << "Directory does not exist:" << dirPath;
        return;
    }
    // 只获取文件，排除子目录
    QStringList fileList = dir.entryList(QDir::Files);

    // 连接 SQLite 数据库
    QSqlDatabase db;
    QString connectionName = "LocalDBConnection";
    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }
    QString dbPath = QCoreApplication::applicationDirPath() + "/images.db";
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    // 创建表（如存在则删除）
    query.exec("DROP TABLE IF EXISTS Images");
    if (!query.exec("CREATE TABLE Images (ID INTEGER PRIMARY KEY AUTOINCREMENT, Path TEXT, Filename TEXT, FType TEXT, Capacity TEXT)")) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return;
    }

    int count = 0;
    db.transaction();

    for (const QString& fileName : fileList) {
        QString filePath = dir.absoluteFilePath(fileName);
        std::string stdFilePath = filePath.toStdString();
        bool isPNG = Linear_Image::isPNG(stdFilePath);
        bool isJPEG = DCT::isJPEG(stdFilePath);

        if (isPNG || isJPEG) {
            query.prepare("INSERT INTO Images (Path, Filename, FType, Capacity) VALUES (?, ?, ?, ?)");
            query.addBindValue(dirPath);
            query.addBindValue(fileName);
            if (isPNG) {
                query.addBindValue("PNG");
                QImage items(dirPath+"/"+fileName);
                int itemcap = Linear_Image::CheckSize(items);
                if (itemcap<settings2.getBulkmin()){
                    continue;
                }
                query.addBindValue(itemcap);
            }
            else if (isJPEG){
                query.addBindValue("JPG");
                QImage items(dirPath+"/"+fileName);
                int itemcap = (items.height()*items.width()/64)*1.5/8/1024*0.95;
                if (itemcap<settings2.getBulkmin()){
                    continue;
                }
                query.addBindValue(itemcap);
            }
            if (!query.exec()) {
                qDebug() << "Failed to insert data:" << query.lastError().text();
            } else {
                count++;
            }
        }
    }
    db.commit();

    qDebug() << "Inserted" << count << "files into database.";
    int sum2 = 0;
    if (query.exec("SELECT SUM(Capacity) FROM Images")) {
        if (query.next() && !query.value(0).isNull()) { // 检查是否有值
            sum2 = query.value(0).toInt();
            qDebug() << "Total sum of Capacity:" << sum2;
        } else {
            qDebug() << "No data found in Images table.";
        }
    } else {
        qDebug() << "Query execution error:" << query.lastError().text();
    }
    if (sum2>fileSize){
        ui->pushButton_5->setEnabled(true);
        QMessageBox::information(this,QString::fromStdString("Success"),QString::fromStdString("The container capacity is enough!"));
    }
    else{
        ui->pushButton_5->setEnabled(false);
        QMessageBox::critical(this,QString::fromStdString("No enough capacity"),QString::fromStdString("The container capacity is NOT enough!"));
    }
    db.close();
    QSqlDatabase::removeDatabase(connectionName);

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
    // 获取 密码框 的文本
    QString Password = ui->lineEdit_3->text();
    QByteArray passwordBytes = Password.toUtf8(); // UTF8将QString转换为QByteArray
    std::string passwordStr = passwordBytes.toStdString(); // 确保数据有效避免悬挂
    //const char* passwordPtr = passwordStr.c_str(); // 使用 std::string 管理生命周期
    QString outr = ui->lineEdit_4->text();
    QString dbPath = QCoreApplication::applicationDirPath() + "/images.db";
    processFileChunks(ui->lineEdit->text(), dbPath, passwordStr, outr);
}


void bulk_encode::on_pushButton_9_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, "Select Containers route",  QDir::homePath(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    ui->lineEdit_4->setText(dir);

}

