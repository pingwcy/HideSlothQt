#include "dctreader.h"
#include "ui_dctreader.h"
#include <QTextStream>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QStringBuilder>
#include <vector>
#include <cstdint>
#include <qtlibjpeg/jpeglib.h>

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


void dctreader::on_pushButton_3_clicked()
{
    QString jpegrr = ui->lineEdit->text();
    QString txtname = ui->lineEdit_2->text();
    QFile file(txtname);

    // 以追加模式打开文件
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        //QMessageBox::critical(this, "Error", "Failed to open the output file!");
        return;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // 打开 JPEG 文件
    FILE* infile = fopen(jpegrr.toLocal8Bit().constData(), "rb");
    if (!infile) {
        //QMessageBox::critical(this, "Error", QString("Error opening file: %1").arg(strerror(errno)));
        file.close();
        return;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);

    // 读取 JPEG header
    jpeg_read_header(&cinfo, TRUE);

    // 获取压缩系数
    jvirt_barray_ptr* coef_arrays = jpeg_read_coefficients(&cinfo);
    JBLOCKARRAY coef_blocks;
    JBLOCK* block;

    // 用于存储要写入文件的内容
    QString outputText;

    for (int comp = 0; comp < cinfo.num_components; comp++) {
        jpeg_component_info* comp_info = &cinfo.comp_info[comp];
        for (int row = 0; row < comp_info->height_in_blocks; row++) {
            coef_blocks = (cinfo.mem->access_virt_barray)(
                (j_common_ptr)&cinfo, coef_arrays[comp], row, 1, FALSE);
            for (int col = 0; col < comp_info->width_in_blocks; col++) {
                block = coef_blocks[0] + col;
                // 使用 QStringBuilder 进行字符串拼接
                outputText += QStringLiteral("Component %1 Block(%2,%3): ").arg(comp).arg(row).arg(col);
                for (int k = 0; k < DCTSIZE2; k++) {
                    outputText += QString::number(block[0][k]) % ' ';
                }
                outputText += '\n';
            }
        }
    }

    // 一次性将内容写入文件
    QTextStream out(&file);
    out << outputText;

    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    file.close();
    QMessageBox::information(this, "Success", "Success saved!");

}

