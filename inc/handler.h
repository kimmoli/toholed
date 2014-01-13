#ifndef HANDLER_H
#define HANDLER_H

#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <QDBusMessage>


class Handler : public QObject
{
    Q_OBJECT
//private:

public:
    Handler() : QObject() {}

public slots:
    void handleNotification(const QDBusMessage& msg)
    {
        qDebug() << msg;
    }
};


#endif // HANDLER_H
