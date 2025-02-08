#include "mainwindow.h"
#include <QApplication>
#include <openssl/evp.h>
#include <QDebug>

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>


int main(int argc, char *argv[])
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // 启用内存泄漏检测
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
class MyApplication : public QApplication {
public:
    MyApplication(int &argc, char **argv) : QApplication(argc, argv) {}
    bool notify(QObject *object, QEvent *event) override {
        try {
            return QApplication::notify(object, event);
        } catch (const std::exception &e) {
            // 处理异常
            qDebug() << "Caught exception:" << e.what();
            return false;
        }
    }
};

