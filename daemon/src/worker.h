#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <QDBusMessage>

#include <poll.h>

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = 0);
    void requestWork(int fd, short events);
    void abort();

    enum event
    {
        PollPri = POLLPRI,
        PollIn = POLLIN,
        PollOut = POLLOUT
    };

private:
    bool _abort;
    bool _working;
    int _fd;
    short _events;

    QMutex mutex;

    static const int timeout;

signals:
    void workRequested();
    void interruptCaptured();
    void finished();

public slots:
    void doWork();
};

#endif // WORKER_H
