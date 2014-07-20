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
        /* Disable everything */
        setInterruptEnable(false);
        deinitOled();
        setVddState(false);
    }

public slots:
    QString testSomething();
    QString draw(const QDBusMessage& msg);
    QString setScreenCaptureOnProximity(const QDBusMessage &msg);
    QString setSettings(const QDBusMessage& msg);

private slots:
    void setVddState(bool turn);
    void enableOled();
    void reloadSettings();

    /* interrupts */
    int setInterruptEnable(bool turn);
    void handleGpioInterrupt();
    void handleProxInterrupt();

    /* dBus Signal handlers */
    void handleSMS(const QDBusMessage& msg);
    void handleDisplayStatus(const QDBusMessage& msg);
    void handleNotificationClosed(const QDBusMessage& msg);
    void handleCall(const QDBusMessage& msg);
    void handleTweetian(const QDBusMessage& msg);
    void handleCommuni(const QDBusMessage& msg);
    void handleActiveSync(const QDBusMessage& msg);
    void handleNotificationActionInvoked(const QDBusMessage& msg);
    void handleChargerStatus(const QDBusMessage& msg);
    void handleMitakuuluu(const QDBusMessage& msg);

    void timerTimeout();
    void checkNewMailNotifications();
    void notificationSend(QString summary, QString body);


private:
    QThread *thread;
    Worker *worker;

    static bool ScreenCaptureOnProximity;

    static bool oledInitDone;
    static bool vddEnabled;
    static bool interruptsEnabled;

    static unsigned int prevBrightness;
    static unsigned int prevProx;

    static unsigned int ssNotifyReplacesId;
    QString ssFilename;
    QString ScreenCaptureOnProximityStorePath;

    static int activeHighlights;
    static int mitakuuluuUnread;

    static int timerCount;
    QTime prevTime;
    bool timeUpdateOverride;
    QTimer *timer;
    QTimer *mailCheckTimer;

    QMutex mutex;

    int gpio_fd;
    int proximity_fd;

    static bool iconSMS;
    static bool iconEMAIL;
    static bool iconCALL;
    static bool iconTWEET;
    static bool iconIRC;
    static bool iconMITAKUULUU;

    static bool chargerConnected;    
};



#endif // TOHOLED-DBUS_H
