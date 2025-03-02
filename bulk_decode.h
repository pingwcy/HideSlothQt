#ifndef BULK_DECODE_H
#define BULK_DECODE_H

#include <QWidget>

namespace Ui {
class bulk_decode;
}

class bulk_decode : public QWidget
{
    Q_OBJECT

public:
    explicit bulk_decode(QWidget *parent = nullptr);
    ~bulk_decode();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::bulk_decode *ui;
    void loadSettings();
};

#endif // BULK_DECODE_H
