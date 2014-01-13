#include "worker.h"
#include <QTimer>
#include <QEventLoop>
#include <poll.h>
#include "toh.h"

#include <QThread>
#include <QDebug>

Worker::Worker(QObject *parent) :
    QObject(parent)
{
    _working =false;
    _abort = false;
}

void Worker::requestWork(int gpio_fd)
{
    mutex.lock();
    _working = true;
    _gpio_fd = gpio_fd;
    _abort = false;
    qDebug()<<"Request worker start in Thread "<<thread()->currentThreadId();
    mutex.unlock();

    emit workRequested();
}

void Worker::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
    mutex.unlock();
}

void Worker::doWork()
{
    struct pollfd fdset[1];
    int nfds = 1;

    int timeout;
    int rc = -1;
    char *buf[20];

    timeout = POLL_TIMEOUT;

    qDebug()<<"Starting worker process in Thread "<<thread()->currentThreadId();

    for (;;)
    {

        // Checks if the process should be aborted
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort) {
            qDebug()<<"Aborting worker process in Thread "<<thread()->currentThreadId();
            break;
        }

        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd = _gpio_fd;
        fdset[0].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (rc < 0)
        {
            qDebug()<<"poll failure in Thread "<<thread()->currentThreadId();
        }

//        if (rc == 0)
//        {
//            qDebug()<<"poll timeout";
//        }

        if (fdset[0].revents & POLLPRI)
        {
            qDebug()<<"toh gpio interrupt in Thread "<<thread()->currentThreadId();
            read(fdset[0].fd, buf, 20);
            emit interruptCaptured();
        }

    }

    mutex.lock();
    _working = false;
    mutex.unlock();

    qDebug()<<"Worker process finished in Thread "<<thread()->currentThreadId();

    emit finished();
}
