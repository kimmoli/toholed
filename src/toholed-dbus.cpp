#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QtCore/QTimer>
#include <QColor>
#include <QTime>

#include <sys/time.h>
#include <time.h>

#include "toholed-dbus.h"
#include "toholed.h"
#include "toh.h"
#include "oled.h"
#include "frontled.h"

/* Main */
Toholed::Toholed()
{
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    timer->start();
    prevTime = QTime::currentTime();
}

/* Timer routine to update OLED clock */
void Toholed::timerTimeout()
{
    QTime current = QTime::currentTime();

    /* Update only if minute has changed and oled is powered and initialized */

    if ((current.minute() != prevTime.minute()) && oledAutoUpdate && vddEnabled && oledInitDone)
    {
        prevTime = current;

        QString tNow = QString("%1:%2")
                        .arg((int) current.hour(), 2, 10,QLatin1Char(' '))
                        .arg((int) current.minute(), 2, 10,QLatin1Char('0'));
        QByteArray baNow = tNow.toLocal8Bit();

        clearOled();
        drawTime(baNow.data());
        updateOled();
    }

    timerCount++;
}

/* Function to set VDD (3.3V for OH) */
QString Toholed::setVddState(const QString &arg)
{
    QString tmp = QString("VDD control request - turn %1 ").arg(arg);
    QString turn = QString("%1").arg(arg);
    QByteArray ba = tmp.toLocal8Bit();

    fprintf(stdout, "%s\n", ba.data());
    writeToLog(ba.data());

    if (control_vdd( ( QString::localeAwareCompare( turn, "on") ? 0 : 1) ) < 0)
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
        clearOled();
        updateOled();

        oledInitDone = true;

        writeToLog("OLED Display initialized and cleared");
    }
    else
        oledInitDone = false;

    return QString("you have been served. %1").arg(arg);
}

/* user wants to show clock on screen */
QString Toholed::setOledAutoUpdate(const QString &arg)
{
    QString turn = QString("%1").arg(arg);

    oledAutoUpdate = QString::localeAwareCompare( turn, "on") ? false : true;

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
        if (control_frontLed(col.red(), col.green(), col.blue()) < 0)
            writeToLog("front led control FAILED");
        else
            writeToLog("front led control OK");
    }
    else
        writeToLog("Invalid color provided");

    return QString("You have been served. %1").arg(arg);
}
