#include <QProcess>
#include <QVariant>

class Process : public QProcess {
Q_OBJECT

public:
Process(QObject *parent = 0) : QProcess(parent) { }

Q_INVOKABLE void start(const QString &program, const QVariantList &arguments)                {
    QStringList args;

    // convert QVariantList from QML to QStringList for QProcess 

    for (int i = 0; i < arguments.length(); i++)
        args << arguments[i].toString();

    QProcess::start(program, args);
}

Q_INVOKABLE QByteArray readAll() {
    return QProcess::readAll();
}
};