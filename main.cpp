#include "mainwindow.h"
#include <QApplication>
#include <openssl/evp.h>
#include <QDebug>
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

// 假设这是按钮点击事件的槽函数
