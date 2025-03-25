#ifndef DCTREADER_H
#define DCTREADER_H

#include <QWidget>

namespace Ui {
class dctreader;
}

class dctreader : public QWidget
{
    Q_OBJECT

public:
    explicit dctreader(QWidget *parent = nullptr);
    ~dctreader();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();
    void processDCT(const QString& , const QString& );
private:
    Ui::dctreader *ui;
};

#endif // DCTREADER_H
