/*
Copyright (c) 2014 kimmoli kimmo.lindholm@gmail.com @likimmo

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

TohoSettings::TohoSettings(QObject *parent) :
    QObject(parent)
{
    readSettings();

    emit versionChanged();
}

QString TohoSettings::readVersion()
{
    return APPVERSION;
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
    settings.endGroup();

    emit blinkChanged();
    emit alsChanged();
    emit proxChanged();
    emit sspChanged();
    emit displayOffWhenMainActiveChanged();
    emit analogClockFaceChanged();
}

TohoSettings::~TohoSettings()
{
}


void TohoSettings::writeSettings()
{

    /**/
    qDebug() << "write settings";

    QDBusMessage m = QDBusMessage::createMethodCall("com.kimmoli.toholed",
                                                    "/",
                                                    "com.kimmoli.toholed",
                                                    "setSettings" );

    QList<QVariant> args;
    args.append(QString(m_blink ? "on" : "off"));
    args.append(QString(m_als ? "on" : "off"));
    args.append(QString(m_prox ? "on" : "off"));
    args.append(QString(m_displayOffWhenMainActive ? "on" : "off"));
    args.append(QString(m_analogClockFace ? "on" : "off"));
    m.setArguments(args);

    if (QDBusConnection::systemBus().send(m))
    {
        qDebug() << "success" << args;
    }
    else
    {
        qDebug() << "failed" << QDBusConnection::systemBus().lastError().message();
    }

    m = QDBusMessage::createMethodCall("com.kimmoli.toholed",
                                                    "/",
                                                    "com.kimmoli.toholed",
                                                    "setScreenCaptureOnProximity" );

    args.clear();
    args.append(QString(m_ssp ? "on" : "off"));
    args.append(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    m.setArguments(args);

    if (QDBusConnection::systemBus().send(m))
    {
        qDebug() << "success" << args;
    }
    else
    {
        qDebug() << "failed" << QDBusConnection::systemBus().lastError().message();
    }


}



