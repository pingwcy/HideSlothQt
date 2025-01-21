#ifndef ABOUTBOX_H
#define ABOUTBOX_H
#include <QPixmap>
#include <QDialog>

namespace Ui {
class AboutBox;
}

class AboutBox : public QDialog
{
    Q_OBJECT

public:
    explicit AboutBox(QWidget *parent = nullptr);
    ~AboutBox();
    QPixmap aboutpic;
private:
    Ui::AboutBox *ui;
};

#endif // ABOUTBOX_H
