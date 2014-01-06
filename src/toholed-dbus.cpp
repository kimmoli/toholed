#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtDBus/QtDBus>

#include "toholed-dbus.h"
#include "toholed.h"
#include "toh.h"
#include "oled.h"

QString Toholed::ping(const QString &arg)
{
    QString tmp = QString("Pinged with message \"%1\" ").arg(arg);
    QByteArray ba = tmp.toLocal8Bit();

    fprintf(stdout, "%s\n", ba.data());

    writeToLog(ba.data());


    if (oled_init_done)
        oledPuts(ba.data());

    return QString("you have been served. %1").arg(arg);
}

QString Toholed::setVddState(const QString &arg)
{
    QString tmp = QString("VDD control request - turn %1 ").arg(arg);
    QString turn = QString("%1").arg(arg);
    QByteArray ba = tmp.toLocal8Bit();

    fprintf(stdout, "%s\n", ba.data());
    writeToLog(ba.data());

    if (control_vdd( ( QString::localeAwareCompare( turn, "on") ? 1 : 0) ) )
        writeToLog("VDD controlling error.");
    else
        writeToLog("VDD control ok");

    return QString("you have been served. %1").arg(arg);
}

QString Toholed::enableOled(const QString &arg)
{
    initOled();
    clearOled();

    oled_init_done = true;

    return QString("you have been served. %1").arg(arg);
}

QString Toholed::kill(const QString &arg)
{
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit");

    return QString("AAARGH. %1").arg(arg);
}
