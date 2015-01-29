/*
Copyright (c) 2014-2015 kimmoli kimmo.lindholm@gmail.com @likimmo

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "tohosettings.h"
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QtDBus>
#include <QDBusArgument>
#include <QImage>

TohoSettings::TohoSettings(QObject *parent) :
    QObject(parent)
{
    QDBusConnection::systemBus().connect("com.kimmoli.toholed", "/", "com.kimmoli.toholed", "displayUpdated",
                              this, SLOT(handleDisplayUpdated()));

    readSettings();

    emit versionChanged();
}

void TohoSettings::handleDisplayUpdated()
{
    emit screenShotChanged();
}

QString TohoSettings::readVersion()
{
    return APPVERSION;
}

QString TohoSettings::readDaemonVersion()
{
    qDebug() << "reading daemon version";

    QDBusInterface getDaemonVersionCall("com.kimmoli.toholed", "/", "com.kimmoli.toholed", QDBusConnection::systemBus());
    getDaemonVersionCall.setTimeout(2000);

    QDBusMessage getDaemonVersionReply = getDaemonVersionCall.call(QDBus::AutoDetect, "getVersion");

    if (getDaemonVersionReply.type() == QDBusMessage::ErrorMessage)
    {
        qDebug() << "Error reading daemon version:" << getDaemonVersionReply.errorMessage();
        return QString("N/A");
    }

    QString daemonVersion = getDaemonVersionReply.arguments().at(0).toString();

    qDebug() << "Daemon version is" << daemonVersion;

    return daemonVersion;
}

void TohoSettings::readSettings()
{
    qDebug() << "reading settings";

    QSettings settings("harbour-toholed", "toholed");

    settings.beginGroup("basic");
    m_blink = settings.value("blink", true).toBool();
    m_als = settings.value("als", true).toBool();
    m_prox = settings.value("proximity", true).toBool();
    m_ssp = settings.value("ssp", false).toBool();
    m_displayOffWhenMainActive = settings.value("displayOffWhenMainActive", false).toBool();
    m_analogClockFace = settings.value("analogClockFace", false).toBool();
    m_showAlarmsPresent = settings.value("showAlarmsPresent", true).toBool();
    settings.endGroup();

    emit blinkChanged();
    emit alsChanged();
    emit proxChanged();
    emit sspChanged();
    emit displayOffWhenMainActiveChanged();
    emit analogClockFaceChanged();
    emit showAlarmsPresentChanged();
}

TohoSettings::~TohoSettings()
{
}


void TohoSettings::writeSettings(QString key, bool value)
{

    /**/
    qDebug() << "write settings" << key << value;

    QDBusMessage m = QDBusMessage::createMethodCall("com.kimmoli.toholed",
                                                    "/",
                                                    "com.kimmoli.toholed",
                                                    "setSettings" );

    QVariantMap map;
    map.insert(key, value);

    QList<QVariant> args;
    args.append(map);

    m.setArguments(args);

    if (QDBusConnection::systemBus().send(m))
        qDebug() << "success" << map;
    else
        qDebug() << "failed" << QDBusConnection::systemBus().lastError().message();
}

void TohoSettings::writeScreenCapture()
{

    QDBusMessage m = QDBusMessage::createMethodCall("com.kimmoli.toholed",
                                                    "/",
                                                    "com.kimmoli.toholed",
                                                    "setSettings" );

    QVariantMap map;
    map.insert("ssp", m_ssp);
    map.insert("sspPath", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

    QList<QVariant> args;
    args.append(map);

    m.setArguments(args);

    if (QDBusConnection::systemBus().send(m))
        qDebug() << "success" << args;
    else
        qDebug() << "failed" << QDBusConnection::systemBus().lastError().message();

}
