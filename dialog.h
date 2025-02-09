#ifndef DIALOG_H
#define DIALOG_H
#include <QDialog>
#include <QComboBox>
#include <string>     // 引入 C++ 标准库中的 std::string

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void setComboBoxValue(QComboBox *comboBox, const std::string &value);
    void loadSettings();
    void saveSettings();
    //void Dialog::setComboBoxValue();
private slots:
    void on_buttonBox_accepted();
private:
    Ui::Dialog  *ui;
};
#endif // DIALOG_H


