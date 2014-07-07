#include "worker.h"
#include <QTimer>
#include <QEventLoop>
#include <poll.h>
#include "toh.h"

#include <QThread>
#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <QDBusMessage>


Worker::Worker(QObject *parent) :
    QObject(parent)
{
    _working =false;
    _abort = false;
}

void Worker::requestWork(int gpio_fd, int proximity_fd)
{
    mutex.lock();
    _working = true;
    _gpio_fd = gpio_fd;
    _proximity_fd = proximity_fd;
    _abort = false;

    mutex.unlock();

    emit workRequested();
}

void Worker::abort()
{
    mutex.lock();
    if (_working)
    {
        _abort = true;
    }
    mutex.unlock();
}

void Worker::doWork()
{
    struct pollfd fdset[2]; // 2
    int nfds = 2; // 2

    int timeout;
    int dummy = 0;
    char *buf[20];

    timeout = POLL_TIMEOUT;

    for (;;)
    {

        // Checks if the process should be aborted
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort)
        {
            break;
        }

        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd = _gpio_fd;
        fdset[0].events = POLLPRI;
        fdset[1].fd = _proximity_fd;
        fdset[1].events = POLLIN;

        poll(fdset, nfds, timeout);

        if (fdset[0].revents & POLLPRI)
        {
            dummy += read(fdset[0].fd, buf, 20);
            emit gpioInterruptCaptured();
        }
        if (fdset[1].revents != 0)
        {
            dummy += read(fdset[1].fd, buf, 200);
            emit proxInterruptCaptured();
        }


    }

    mutex.lock();
    _working = false;
    mutex.unlock();

    emit finished();
}
