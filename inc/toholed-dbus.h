#ifndef TOHOLED_DBUS_H
#define TOHOLED_DBUS_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>

#include <QTime>
#include <QThread>
#include "oled.h"
#include "worker.h"

#define SERVICE_NAME "com.kimmoli.toholed"




/* main class */

class Toholed: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    Toholed();

    ~Toholed()
    {
        worker->abort();
        thread->wait();
        delete thread;
        delete worker;
    }

public slots:
    QString setVddState(const QString &arg);
    QString enableOled(const QString &arg);
    QString disableOled(const QString &arg);
    QString setOledAutoUpdate(const QString &arg);
    QString frontLed(const QString &arg);
    QString kill(const QString &arg);

    /* interrupts */
    QString setInterruptEnable(const QString &arg);
    void handleGpioInterrupt();
    void handleProxInterrupt();

    /* Notification stuff */
    void handleSMS(const QDBusMessage& msg);
    void handlehandleCoverStatus(const QDBusMessage& msg);
    void handleEventsAdded(const QDBusMessage& msg);

private slots:
    void timerTimeout();


private:
    QThread *thread;
    Worker *worker;

    static bool oledInitDone;
    static bool vddEnabled;
    static bool oledAutoUpdate;

    static int timerCount;
    QTime prevTime;
    bool timeUpdateOverride;
    QTimer *timer;

    int gpio_fd;
    int proximity_fd;
};



#endif // TOHOLED-DBUS_H
