#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include "GlobalSettings.h"
#include <string>
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    bool workingmode = GlobalSettings::instance().getMode();
    ui->radioButton_3->setChecked(workingmode);
    ui->radioButton_4->setChecked(!workingmode);
    bool enc = GlobalSettings::instance().getEnc();
    ui->radioButton->setChecked((enc));
    ui->radioButton_2->setChecked((!enc));
    int Iterations = GlobalSettings::instance().getIter();
    ui->lineEdit->setText(QString::number(Iterations));
    bool csthash = GlobalSettings::instance().getCstHash();
    ui->checkBox_2->setChecked(csthash);
    bool cstiter = GlobalSettings::instance().getCstIter();
    ui->checkBox->setChecked(cstiter);
    std::string hash = GlobalSettings::instance().getHash();
    int indexs = ui->comboBox->findText(QString::fromStdString(hash));
    ui->comboBox->setCurrentIndex(indexs);
    std::string encal = GlobalSettings::instance().getEncalg();
    int indexe = ui->comboBox_2->findText(QString::fromStdString(encal));
    ui->comboBox_2->setCurrentIndex(indexe);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_buttonBox_accepted()
{
    GlobalSettings::instance().setMode(ui->radioButton_3->isChecked());
    GlobalSettings::instance().setEnc(ui->radioButton->isChecked());
    GlobalSettings::instance().setCstHash(ui->checkBox_2->checkState());
    GlobalSettings::instance().setCstIter(ui->checkBox->checkState());
    GlobalSettings::instance().setIter(ui->lineEdit->text().toInt());
    GlobalSettings::instance().setHash(ui->comboBox->currentText().toStdString());
    GlobalSettings::instance().setEncalg(ui->comboBox_2->currentText().toStdString());
    QMessageBox::information(this,QString::fromStdString("Saved"),QString::fromStdString("Settings Saved!"));
}
