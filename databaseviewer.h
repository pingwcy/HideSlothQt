#ifndef DATABASEVIEWER_H
#define DATABASEVIEWER_H

#include <QDialog>

namespace Ui {
class DatabaseViewer;
}

class DatabaseViewer : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseViewer(QWidget *parent = nullptr);
    ~DatabaseViewer();

private:
    Ui::DatabaseViewer *ui;
    void loadDatabase();  // 读取数据库

};

#endif // DATABASEVIEWER_H
