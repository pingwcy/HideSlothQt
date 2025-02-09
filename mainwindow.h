#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
QT_BEGIN_NAMESPACE
#include <QMessageBox>

namespace Ui { class MainWindow; }

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    std::vector<uint8_t> extractDataraw;


public slots:
    void showSuccessMessage() {
        QMessageBox::information(this, QString::fromStdString("Success"), QString::fromStdString("Succes Encode and save!"));
    }
    void showSuccessMessage2() {
        QMessageBox::information(this, QString::fromStdString("Success"), QString::fromStdString("Succes Decode!"));
    }

private slots:
    void on_pushButton_2_clicked();
    void on_Check_Button_clicked();
    void on_SelectContainer_clicked();
    void on_SelectSecret_clicked();
    void on_pushButton_3_clicked();
    void on_adv_btn_clicked();
    void on_actionAdvanced_Settings_triggered();
    void on_actionExit_triggered();
    void on_actionSelect_Container_triggered();
    void on_actionSelect_Secret_triggered();

    void on_actionAbout_triggered();

    void on_actionDCT_Cof_Reader_triggered();

    void on_pushButton_clicked();

    void on_actionAbout_Qt_triggered();

    void on_actionRestart_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
