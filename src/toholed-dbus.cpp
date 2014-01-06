#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtDBus/QtDBus>

#include "toholed-dbus.h"
#include "toholed.h"
#include "toh.h"
#include "oled.h"
#include "frontled.h"

QString Toholed::ping(const QString &arg)
{
    QString tmp;
    tmp = QString("Pinged with message \"%1\" ").arg(arg);
    QByteArray ba = tmp.toLocal8Bit();
    tmp = QString("%1").arg(arg);
    QByteArray showOnDisplay = tmp.toLocal8Bit();


    fprintf(stdout, "%s\n", ba.data());

    writeToLog(ba.data());


    if (oled_init_done)
        oledPuts(showOnDisplay.data());

    return QString("you have been served. %1").arg(arg);
}

QString Toholed::setVddState(const QString &arg)
{
    QString tmp = QString("VDD control request - turn %1 ").arg(arg);
    QString turn = QString("%1").arg(arg);
    QByteArray ba = tmp.toLocal8Bit();

    fprintf(stdout, "%s\n", ba.data());
    writeToLog(ba.data());

    if (control_vdd( ( QString::localeAwareCompare( turn, "on") ? 0 : 1) ) < 0)
        writeToLog("VDD control FAILED");
    else
        writeToLog("VDD control OK");

    return QString("you have been served. %1").arg(arg);
}

QString Toholed::enableOled(const QString &arg)
{
    initOled();
    clearOled();

    writeToLog("OLED Display initialized and cleared");

    oled_init_done = true;

    return QString("you have been served. %1").arg(arg);
}

QString Toholed::kill(const QString &arg)
{
    writeToLog("Someone wants to kill me");
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit");

    return QString("AAARGH. %1").arg(arg);
}

QString Toholed::frontLed(const QString &arg)
{
    QString turn = QString("%1").arg(arg);

    if (control_frontLed((QString::localeAwareCompare( turn, "on") ? 0 : 255), 0, 0) < 0)
        writeToLog("front led control FAILED");
    else
        writeToLog("front led control OK");

    return QString("You have been served. %1").arg(arg);
}
