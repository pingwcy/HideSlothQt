#include "aboutbox.h"
#include "ui_aboutbox.h"

AboutBox::AboutBox(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutBox)
{
    ui->setupUi(this);
    aboutpic.load(":/new/prefix1/about.jpg");
    ui->label_5->setPixmap(aboutpic);
    ui->label_5->setScaledContents(true);
}

AboutBox::~AboutBox()
{
    delete ui;
}
