#ifndef LOGICMAIN_H
#define LOGICMAIN_H
#include <string>
#include <vector>
#include <QString>
#include <QObject>
#include <QFutureWatcher>
#include <QtConcurrent>

class LogicMain
{
public:
    LogicMain();
};
namespace logic{
QString encryptString(QString PlainText, std::string pwd);
QString decryptString(QString PlainText, std::string pwd);
void encryptFile(const QString &inputPath, const QString &outputPath, std::string pwd);
void decryptFile(const QString &SecretRoute, const QString &fileName, std::string pwd);

void encode_logic(const QString& PlainText, const QString& SecretRoute, bool Isstring, bool File, const QString& ContainerRoute, const std::string& passwordStr, QString fileName2, QWidget* parent);
void decode_logic(const QString& ContainerRoute, const std::string& passwordStr, bool File, bool Isstring, QWidget* parent);

}
#endif // LOGICMAIN_H
