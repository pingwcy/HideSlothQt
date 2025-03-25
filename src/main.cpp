#include "mainwindow.h"
#include <QApplication>
#include <openssl/evp.h>
#include <QDebug>

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>


class MyApplication : public QApplication {
public:
    MyApplication(int &argc, char **argv) : QApplication(argc, argv) {}
    bool notify(QObject *object, QEvent *event) override {
        try {
            return QApplication::notify(object, event);
        } catch (const std::exception &e) {
            qCritical() << "Caught std::exception:" << e.what();
        } catch (...) {
            qCritical() << "Caught unknown exception!";
        }
        return false;
    }
};

int main(int argc, char *argv[])
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // 启用内存泄漏检测
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    OpenSSL_add_all_algorithms();  // OpenSSL 1.1.0 以上版本不需要
#endif
    MyApplication a(argc, argv);
    MainWindow w;
    w.show();
    int result = a.exec();
    // 清理 OpenSSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_cleanup();  // OpenSSL 1.1.0 以上版本已废弃
#endif
    return result;
}
