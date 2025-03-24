#include "dctreader.h"
#include "ui_dctreader.h"

#include <QTextStream>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QStringBuilder>
#include <QtCore/qglobal.h>
#include <QDebug>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#if defined(QT_VERSION) && (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
// Qt 6
#if defined(QT_DEBUG)
#include <qtjpegd/jpeglib.h>
#else
#include <QtJpeg/jpeglib.h>
#endif
#else
// Qt 5
#if defined(QT_DEBUG)
#include <qtlibjpegd/jpeglib.h>
#else
#include <qtlibjpeg/jpeglib.h>
#endif
#endif

#include "DCT.cpp"
#include "utils_a.h"

dctreader::dctreader(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::dctreader)
{
    ui->setupUi(this);
}

dctreader::~dctreader()
{
    delete ui;
}

void dctreader::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"));
    ui->lineEdit->setText(fileName);
}

void dctreader::on_pushButton_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select a empty txt to write"));
    ui->lineEdit_2->setText(fileName);
}

void dctreader::processDCT(const QString& jpegPath, const QString& txtPath) {
    QFile jpegFile(jpegPath);
    if (!jpegFile.open(QIODevice::ReadOnly)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::critical(this, "Error", "Failed to open JPEG file!");
        }, Qt::QueuedConnection);
        return;
    }
#ifdef _WIN32
    const std::string path = Utils::wstringToUtf8(jpegPath.toStdWString());
#else
    const std::string path = jpegPath.toUtf8().toStdString();
#endif

    if (!DCT::isJPEG(path)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::critical(this, "Error", "Not a valid JPEG file!");
            ui->pushButton_3->setEnabled(true);
            ui->progressBar->setMaximum(1);
        }, Qt::QueuedConnection);
        return;
    }

    QFile outFile(txtPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::critical(this, "Error", "Failed to open the output file!");
        }, Qt::QueuedConnection);
        return;
    }
    QTextStream out(&outFile);

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    QByteArray jpegData = jpegFile.readAll();
    jpeg_mem_src(&cinfo, (const unsigned char*)jpegData.constData(), jpegData.size());

    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::critical(this, "Error", "Invalid JPEG header!");
        }, Qt::QueuedConnection);
        jpeg_destroy_decompress(&cinfo);
        return;
    }

    jvirt_barray_ptr* coef_arrays = jpeg_read_coefficients(&cinfo);
    if (!coef_arrays) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::critical(this, "Error", "Failed to read DCT coefficients!");
        }, Qt::QueuedConnection);
        jpeg_destroy_decompress(&cinfo);
        return;
    }

    if (!cinfo.comp_info) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::critical(this, "Error", "JPEG Component Info is null!");
        }, Qt::QueuedConnection);
        jpeg_destroy_decompress(&cinfo);
        return;
    }

    for (int comp = 0; comp < cinfo.num_components; comp++) {
        jpeg_component_info* comp_info = &cinfo.comp_info[comp];
        if (!comp_info) continue;

        for (unsigned int row = 0; row < comp_info->height_in_blocks; row++) {
            JBLOCKARRAY coef_blocks = (cinfo.mem->access_virt_barray)
            ((j_common_ptr)&cinfo, coef_arrays[comp], row, 1, FALSE);

            if (!coef_blocks || !coef_blocks[0]) continue;  // 确保指针有效

            for (unsigned int col = 0; col < comp_info->width_in_blocks; col++) {
                JBLOCK* block = coef_blocks[0] + col;
                QString outputStr = QString("Component %1 Block(%2,%3): ").arg(comp).arg(row).arg(col);

                for (int k = 0; k < DCTSIZE2; k++) {
                    outputStr.append(QString::number(block[0][k]) + " ");
                }
                out << outputStr << '\n';
            }
        }
    }

    out.flush();
    outFile.close();
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // 任务成功完成后通知主线程
    QMetaObject::invokeMethod(this, [this]() {
        QMessageBox::information(this, "Success", "DCT coefficients successfully saved!");
    }, Qt::QueuedConnection);
}

void dctreader::on_pushButton_3_clicked() {
    const QString jpegPath = ui->lineEdit->text();
    const QString txtPath = ui->lineEdit_2->text();

    if (jpegPath.isEmpty() || txtPath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please provide valid file paths.");
        return;
    }

    // 禁用按钮，防止重复点击
    ui->pushButton_3->setEnabled(false);
    ui->progressBar->setMaximum(0);
    // 使用 QtConcurrent 启动后台任务
    QFuture<void> future = QtConcurrent::run([this, jpegPath, txtPath]() {
        processDCT(jpegPath, txtPath);
    });

    // 使用 QFutureWatcher 监听任务完成
    auto* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
        watcher->deleteLater();
        ui->pushButton_3->setEnabled(true);
        ui->progressBar->setMaximum(1);
        //QMessageBox::information(this, "Success", "DCT coefficients successfully saved!");
    });
    watcher->setFuture(future);
}


