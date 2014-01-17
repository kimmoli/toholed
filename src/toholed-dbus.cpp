/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * toholed daemon, d-bus server call method functions.
 *
 *
 *
 *
 */


#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusArgument>
#include <QtCore/QTimer>
#include <QColor>
#include <QTime>
#include <QThread>

#include <sys/time.h>
#include <time.h>

#include <poll.h>

#include "toholed-dbus.h"
#include "toholed.h"
#include "toh.h"
#include "oled.h"
#include "frontled.h"
#include "tca8424.h"
#include "charger.h"
#include "icons.h"

static char screenBuffer[SCREENBUFFERSIZE] = { 0 };

/* Main */
Toholed::Toholed()
{
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    timer->start();
    prevTime = QTime::currentTime();

    memset(screenBuffer, 0x00, SCREENBUFFERSIZE);

    thread = new QThread();
    worker = new Worker();

    worker->moveToThread(thread);
    connect(worker, SIGNAL(gpioInterruptCaptured()), this, SLOT(handleGpioInterrupt()));
    connect(worker, SIGNAL(proxInterruptCaptured()), this, SLOT(handleProxInterrupt()));
    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    timeUpdateOverride = false;
}

/* Timer routine to update OLED clock */
void Toholed::timerTimeout()
{
    QTime current = QTime::currentTime();

    /* Update only if minute has changed and oled is powered and initialized */

    if (((current.minute() != prevTime.minute()) || timeUpdateOverride) && oledAutoUpdate && vddEnabled && oledInitDone)
    {
        timeUpdateOverride = false;
        prevTime = current;

        QString tNow = QString("%1:%2")
                        .arg((int) current.hour(), 2, 10, QLatin1Char(' '))
                        .arg((int) current.minute(), 2, 10, QLatin1Char('0'));
        QByteArray baNow = tNow.toLocal8Bit();

        QString batNow = QString("%1%")
                          .arg((int) chargerGetCapacity(), 3, 10, QLatin1Char(' '));
        QByteArray babatNow = batNow.toLocal8Bit();


        clearOled(screenBuffer);
        drawTime(baNow.data(), screenBuffer);
        drawBatteryLevel(babatNow.data(), screenBuffer);
        updateOled(screenBuffer);

        writeToLog(baNow.data());
    }

    timerCount++;
}

/* Function to set VDD (3.3V for OH) */
QString Toholed::setVddState(const QString &arg)
{
    QString tmp = QString("VDD control request - turn %1 ").arg(arg);
    QString turn = QString("%1").arg(arg);
    QByteArray ba = tmp.toLocal8Bit();

    writeToLog(ba.data());

    if (controlVdd( ( QString::localeAwareCompare( turn, "on") ? 0 : 1) ) < 0)
    {
        vddEnabled = false;
        writeToLog("VDD control FAILED");
    }
    else
    {
        vddEnabled = QString::localeAwareCompare( turn, "on") ? false : true;
        writeToLog("VDD control OK");
    }

    return QString("you have been served. %1").arg(arg);
}

/* Initialze and clear oled */
QString Toholed::enableOled(const QString &arg)
{
    if (vddEnabled)
    {
        initOled();
        clearOled(screenBuffer);
        updateOled(screenBuffer);

        oledInitDone = true;

        writeToLog("OLED Display initialized and cleared");
    }
    else
        oledInitDone = false;

    return QString("you have been served. %1").arg(arg);
}

QString Toholed::disableOled(const QString &arg)
{
    if (vddEnabled && oledInitDone)
    {
        deinitOled();

        oledInitDone = false;

        writeToLog("OLED Display cleared and shut down");
    }

    return QString("you have been served. %1").arg(arg);
}

/* user wants to show clock on screen */
QString Toholed::setOledAutoUpdate(const QString &arg)
{
    QString turn = QString("%1").arg(arg);

    oledAutoUpdate = QString::localeAwareCompare( turn, "on") ? false : true;

    timeUpdateOverride = true;
    timerTimeout(); /* draw clock immediately */

    writeToLog("OLED autoupdate set");

    return QString("you have been served. %1").arg(arg);
}

/* Kills toholed daemon */
QString Toholed::kill(const QString &arg)
{
    writeToLog("Someone wants to kill me");
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit");

    return QString("AAARGH. %1").arg(arg);
}

/* Controls RGB led on front of phone, html color format #RRGGBB */
QString Toholed::frontLed(const QString &arg)
{
    QString tmp = QString("%1").arg(arg);
    char buf[30];

    sprintf(buf, "Front led color %s", qPrintable(tmp));
    writeToLog(buf);

    if (QColor::isValidColor(tmp))
    {
        QColor col = QColor(tmp);
        if (controlFrontLed(col.red(), col.green(), col.blue()) < 0)
            writeToLog("front led control FAILED");
        else
            writeToLog("front led control OK");
    }
    else
        writeToLog("Invalid color provided");

    return QString("You have been served. %1").arg(arg);
}

/*
 *    Interrupt stuff
 */


QString Toholed::setInterruptEnable(const QString &arg)
{
    QString turn = QString("%1").arg(arg);
    int fd;

    if(QString::localeAwareCompare( turn, "on") == 0)
    {

        //setVddState("on");

        writeToLog("enabling interrupt");

        gpio_fd = getTohInterrupt();
        proximity_fd = getProximityInterrupt();

        if ((gpio_fd > -1) && (proximity_fd > -1))
        {
            worker->abort();
            thread->wait(); // If the thread is not running, this will immediately return.

            worker->requestWork(gpio_fd, proximity_fd);

            writeToLog("worker started");

            fd = tca8424_initComms(TCA_ADDR);
            if (fd<0)
            {
                writeToLog("failed to start communication with TCA8424");
                return QString("failed");
            }
            tca8424_reset(fd);
            tca8424_leds(fd, 5);
            tca8424_closeComms(fd);

            return QString("success");
        }
        else
        {
            writeToLog("FAILURE");

            return QString("failed");
        }
    }
    else
    {

        writeToLog("disabling interrupt");

        worker->abort();
        thread->wait();
        delete thread;
        delete worker;

        releaseTohInterrupt(gpio_fd);
        releaseProximityInterrupt(proximity_fd);

        //setVddState("off");

        return QString("disabled");
    }
}

void Toholed:: handleSMS(const QDBusMessage& msg)
{
    char buf[100];

    QList<QVariant> args = msg.arguments();

    sprintf(buf, "message ""%s""", qPrintable(args.at(0).toString()));
    writeToLog(buf);

    drawIcon(64, MESSAGE, screenBuffer);
    updateOled(screenBuffer);
}


/* interrupt handler */
void Toholed::handleGpioInterrupt()
{
    int fd;
    char inRep[50] = { 0 };
    char buf[100] = { 0 };

    writeToLog("TOH Interrupt reached interrupt handler routine.");

    fd = tca8424_initComms(TCA_ADDR);
    if (fd<0)
    {
        writeToLog("failed to start communication with TCA8424");
        return;
    }
    tca8424_readInputReport(fd, inRep);
    tca8424_closeComms(fd);

    sprintf(buf, "Input report: ");
    for (int i=0 ; i< 11 ; i++)
        sprintf(buf, "%s %02x", buf, inRep[i]);

    writeToLog(buf);

}

void Toholed::handleProxInterrupt()
{
    bool prox = getProximityStatus();

    if (prox)
    {
        writeToLog("Proximity interrupt - proximity");
        /* Front covered, lets show clock on TOHOLED */
        setVddState("on");
        enableOled("");
        setOledAutoUpdate("on");
    }
    else
    {
        setOledAutoUpdate("off");
        disableOled("");
        setVddState("off");

        writeToLog("Proximity interrupt - away");
    }
}
