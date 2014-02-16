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
    QString setOledContrast(const QString &arg);
    QString frontLed(const QString &arg);
    QString kill(const QString &arg);

    /* interrupts */
    QString setInterruptEnable(const QString &arg);
    void handleGpioInterrupt();
    void handleProxInterrupt();

    /* dBus Signal handlers */
    void handleSMS(const QDBusMessage& msg);
    void handleDisplayStatus(const QDBusMessage& msg);
    void handleNotificationClosed(const QDBusMessage& msg);
    void handleCall(const QDBusMessage& msg);
    void handleCommHistory(const QDBusMessage& msg);
    void handleTweetian(const QDBusMessage& msg);

private slots:
    void timerTimeout();


private:
    QThread *thread;
    Worker *worker;

    static bool oledInitDone;
    static bool vddEnabled;
    static bool oledAutoUpdate;
    static bool interruptsEnabled;

    static unsigned int prevBrightness;
    static unsigned int prevProx;

    static int timerCount;
    QTime prevTime;
    bool timeUpdateOverride;
    QTimer *timer;

    QMutex mutex;

    int gpio_fd;
    int proximity_fd;

    static bool iconSMS;
    static bool iconEMAIL;
    static bool iconCALL;
    static bool iconTWEET;
};



#endif // TOHOLED-DBUS_H
