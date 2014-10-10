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

extern "C"
{
    #include "iphbd/libiphb.h"
}


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

        if(iphbdHandler)
            (void)iphb_close(iphbdHandler);

        if(iphbNotifier)
            delete iphbNotifier;

        setInterruptEnable(false);
        deinitOled();
        setVddState(false);
        printf("Toholed terminated!\n");
    }

public slots:
    QString getVersion();
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
    void handleDisplayStatus(const QDBusMessage& msg);
    void handleNotificationClosed(const QDBusMessage& msg);
    void handleCall(const QDBusMessage& msg);
    void handleTweetian(const QDBusMessage& msg);
    void handleCommuni(const QDBusMessage& msg);
    void handleNotificationActionInvoked(const QDBusMessage& msg);
    void handleChargerStatus(const QDBusMessage& msg);
    void handleMitakuuluu(const QDBusMessage& msg);
    void handleProfileChanged(const QDBusMessage& msg);
    void handleAlarm(const QDBusMessage& msg);
    void handleNetworkRegistration(const QDBusMessage& msg);
    void handleBluetooth(const QDBusMessage& msg);
    void handleWifi(const QDBusMessage& msg);
    void handleCellular(const QDBusMessage& msg);

    void handleEmailNotify();
    void handleTwitterNotify();
    void handleFacebookNotify();
    void handleIrssiNotify();
    void handleImNotify();
    void handleSmsNotify();
    void handleSystemUpdateNotify();
    void handleOtherNotify();

    void updateDisplay(bool timeUpdateOverride = false, int blinks = 0);
    void alarmTimerTimeout( );
    void notificationSend(QString summary, QString body);

    void heartbeatReceived(int sock);
    void iphbStop();
    void iphbStart();

private:
    QThread *thread;
    Worker *worker;

    bool ScreenCaptureOnProximity;

    bool oledInitDone;
    bool vddEnabled;
    bool interruptsEnabled;

    unsigned int prevBrightness;
    unsigned int prevProx;

    unsigned int ssNotifyReplacesId;
    QString ssFilename;
    QString ScreenCaptureOnProximityStorePath;

    int activeHighlights;
    int mitakuuluuUnread;

    int timerCount;
    QTime prevTime;
    QTimer *timer;
    QTimer *alarmTimer;

    QMutex mutex;

    iphb_t iphbdHandler;
    int iphb_fd;
    QSocketNotifier *iphbNotifier;
    bool iphbRunning;

    int gpio_fd;
    int proximity_fd;

    bool iconSMS;
    bool iconEMAIL;
    bool iconCALL;
    bool iconTWEET;
    bool iconIRC;
    bool iconMITAKUULUU;
    bool systemUpdate;

    bool blinkOnNotification;

    bool chargerConnected;
    bool silentProfile;

    bool displayOffWhenMainActive;
    bool analogClockFace;

    QString getCurrentProfile();
    void getCurrentNetworkConnectionStates();

    QVariantMap getDbusProperties(const QString & service,
                                  const QString & path,
                                  const QString & interface,
                                  const QDBusConnection & connection = QDBusConnection::sessionBus());

    QString networkType;
    bool wifiPowered;
    bool bluetoothPowered;
    bool wifiConnected;
    bool bluetoothConnected;
    bool cellularConnected;
    bool cellularPowered;

    bool lockDrawingMode;

};



#endif // TOHOLED-DBUS_H
