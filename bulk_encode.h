#ifndef BULK_ENCODE_H
#define BULK_ENCODE_H

#include <QWidget>

namespace Ui {
class bulk_encode;
}

class bulk_encode : public QWidget
{
    Q_OBJECT

public:
    explicit bulk_encode(QWidget *parent = nullptr);
    ~bulk_encode();

private slots:
    void on_pushButton_8_clicked();

    void on_pushButton_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_9_clicked();

private:
    Ui::bulk_encode *ui;
    void processFileChunks(const QString &filePath, QString dbpath, std::string passwordStr,QString);
    void loadSettings();
};

#endif // BULK_ENCODE_H
