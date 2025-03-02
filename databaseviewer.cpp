#include "databaseviewer.h"
#include "ui_databaseviewer.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseViewer::DatabaseViewer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseViewer)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels({"Route", "FileName","FileType","Capacity (KB)"});
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    loadDatabase();  // 读取数据库内容
}

DatabaseViewer::~DatabaseViewer()
{
    delete ui;
}
void DatabaseViewer::loadDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QCoreApplication::applicationDirPath() + "/images.db");  // 修改为实际数据库路径

    if (!db.open()) {
        qDebug() << "Unable to open database" << db.lastError().text();
        return;
    }

    QSqlQuery query("SELECT Path, Filename, FType, Capacity FROM Images");
    int row = 0;
    ui->tableWidget->setRowCount(0);

    while (query.next()) {
        QString path = query.value(0).toString();
        QString filename = query.value(1).toString();
        QString FFType = query.value(2).toString();
        QString Capac = query.value(3).toString();

        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(path));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(filename));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(FFType));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(Capac));
        row++;
    }
    db.close();
}
