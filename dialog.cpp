#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include "GlobalSettings.h"
#include <string>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog) {
    ui->setupUi(this);
    loadSettings();
}

Dialog::~Dialog() {
    delete ui;
}

void Dialog::loadSettings() {
    auto &settings = GlobalSettings::instance();

    ui->radioButton_3->setChecked(settings.getMode());
    ui->radioButton_4->setChecked(!settings.getMode());

    ui->radioButton->setChecked(settings.getEnc());
    ui->radioButton_2->setChecked(!settings.getEnc());

    ui->lineEdit->setText(QString::number(settings.getIter()));

    ui->checkBox_2->setChecked(settings.getCstHash());
    ui->checkBox->setChecked(settings.getCstIter());

    ui->checkBox_3->setChecked(settings.getJPGLSB());
    ui->lineEdit_2->setText(QString::fromStdString((std::to_string(settings.getBulkmin()))));
    setComboBoxValue(ui->comboBox, settings.getHash());
    setComboBoxValue(ui->comboBox_2, settings.getEncalg());
    setComboBoxValue(ui->comboBox_3, settings.getStealg());
}

void Dialog::saveSettings() {
    auto &settings = GlobalSettings::instance();

    settings.setMode(ui->radioButton_3->isChecked());
    settings.setEnc(ui->radioButton->isChecked());
    settings.setCstHash(ui->checkBox_2->isChecked());
    settings.setCstIter(ui->checkBox->isChecked());
    settings.setIter(ui->lineEdit->text().toInt());

    settings.setHash(ui->comboBox->currentText().toStdString());
    settings.setEncalg(ui->comboBox_2->currentText().toStdString());
    settings.setStealg(ui->comboBox_3->currentText().toStdString());

    settings.setJPGLSB(ui->checkBox_3->isChecked());
    settings.setBulkmin(ui->lineEdit_2->text().toInt());
    QMessageBox::information(this, "Saved", "Settings Saved!");
}

void Dialog::on_buttonBox_accepted() {
    saveSettings();
}

void Dialog::setComboBoxValue(QComboBox *comboBox, const std::string &value) {
    int index = comboBox->findText(QString::fromStdString(value));
    if (index >= 0) {
        comboBox->setCurrentIndex(index);
    }
}
