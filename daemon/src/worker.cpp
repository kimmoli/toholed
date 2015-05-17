#include "worker.h"

const int Worker::timeout = 1000;

Worker::Worker(QObject *parent) :
    QObject(parent)
{
    _working =false;
    _abort = false;
}

void Worker::requestWork(int fd, short events)
{
    mutex.lock();
    _working = true;
    _fd = fd;
    _abort = false;
    _events = events;

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
    struct pollfd fdset[1];
    int nfds = 1;

    int dummy = 0;
    char *buf[20];

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

        fdset[0].fd = _fd;
        fdset[0].events = _events;

        poll(fdset, nfds, timeout);

        if (fdset[0].revents & _events)
        {
            dummy += read(fdset[0].fd, buf, 20);
            emit interruptCaptured();
        }
    }

    mutex.lock();
    _working = false;
    mutex.unlock();

    emit finished();
}
