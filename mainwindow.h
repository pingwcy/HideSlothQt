#ifndef MAINWINDOW_H


#define MAINWINDOW_H


#include <QMainWindow>


QT_BEGIN_NAMESPACE


namespace Ui { class MainWindow; }


QT_END_NAMESPACE


class MainWindow : public QMainWindow


{


    Q_OBJECT


public:


    MainWindow(QWidget *parent = nullptr);


    ~MainWindow();


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


private:


    Ui::MainWindow *ui;


};


#endif // MAINWINDOW_H


