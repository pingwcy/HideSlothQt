#include "mainwindow.h"


#include <QApplication>


#include <openssl/evp.h>


int main(int argc, char *argv[])


{


    // 初始化 OpenSSL


    OpenSSL_add_all_algorithms();


    QApplication a(argc, argv);


    MainWindow w;


    w.show();


    int result = a.exec();


    // 清理 OpenSSL


    EVP_cleanup();


    return result;


}


// 假设这是按钮点击事件的槽函数


