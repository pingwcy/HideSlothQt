#include "dctreader.h"
#include "ui_dctreader.h"
#include <QTextStream>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QStringBuilder>
//#include <vector>
//#include <cstdint>
#include <qtlibjpeg/jpeglib.h>
#include <QDebug>
#include "DCT.cpp"
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

void dctreader::on_pushButton_3_clicked() {
    const QString jpegPath = ui->lineEdit->text();
    const QString txtPath = ui->lineEdit_2->text();

    QFile jpegFile(jpegPath);
    if (!jpegFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open JPEG file!");
        return;
    }

    if (!DCT::isJPEG(jpegPath.toUtf8().constData())) {
        QMessageBox::critical(this, "Error", "Not a valid JPEG file!");
        return;
    }

    QFile outFile(txtPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open the output file!");
        return;
    }
    QTextStream out(&outFile);

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // 直接使用 QFile 读取数据
    QByteArray jpegData = jpegFile.readAll();
    jpeg_mem_src(&cinfo, (const unsigned char*)jpegData.constData(), jpegData.size());

    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        QMessageBox::critical(this, "Error", "Invalid JPEG header!");
        jpeg_destroy_decompress(&cinfo);
        return;
    }

    jvirt_barray_ptr* coef_arrays = jpeg_read_coefficients(&cinfo);
    if (!coef_arrays) {
        QMessageBox::critical(this, "Error", "Failed to read DCT coefficients!");
        jpeg_destroy_decompress(&cinfo);
        return;
    }

    // 遍历 DCT 系数
    if (!cinfo.comp_info) {
        QMessageBox::critical(this, "Error", "JPEG Component Info is null!");
        jpeg_destroy_decompress(&cinfo);
        return;
    }

    for (int comp = 0; comp < cinfo.num_components; comp++) {
        jpeg_component_info* comp_info = &cinfo.comp_info[comp];
        if (!comp_info) continue; // 额外的安全检查

        for (unsigned int row = 0; row < comp_info->height_in_blocks; row++) {
            JBLOCKARRAY coef_blocks = (cinfo.mem->access_virt_barray)
            ((j_common_ptr)&cinfo, coef_arrays[comp], row, 1, FALSE);

            if (!coef_blocks) continue;

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

    out.flush();  // 确保数据写入
    outFile.close();
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    QMessageBox::information(this, "Success", "DCT coefficients successfully saved!");
}




