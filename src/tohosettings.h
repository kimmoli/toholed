/*
Copyright (c) 2014 kimmoli kimmo.lindholm@gmail.com @likimmo

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TOHOSETTINGS_H
#define TOHOSETTINGS_H
#include <QObject>

class TohoSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString version READ readVersion NOTIFY versionChanged())

    Q_PROPERTY(bool blink READ readBlink WRITE writeBlink NOTIFY blinkChanged())
    Q_PROPERTY(bool als READ readAls WRITE writeAls NOTIFY alsChanged())
    Q_PROPERTY(bool prox READ readProx WRITE writeProx NOTIFY proxChanged())
    Q_PROPERTY(bool ssp READ readSsp WRITE writeSsp NOTIFY sspChanged())
    Q_PROPERTY(bool displayOffWhenMainActive READ readDisplayOffWhenMainActive WRITE writeDisplayOffWhenMainActive NOTIFY displayOffWhenMainActiveChanged())
    Q_PROPERTY(bool analogClockFace READ readAnalogClockFace WRITE writeAnalogClockFace NOTIFY analogClockFaceChanged())


public:
    explicit TohoSettings(QObject *parent = 0);
    ~TohoSettings();

    QString readVersion();

    void writeBlink(bool x) { m_blink = x; emit blinkChanged(); }
    void writeAls(bool x) { m_als = x; emit alsChanged(); }
    void writeProx(bool x) { m_prox = x; emit proxChanged(); }
    void writeSsp(bool x) { m_ssp = x; emit sspChanged(); }
    void writeDisplayOffWhenMainActive(bool x) { m_displayOffWhenMainActive = x; emit displayOffWhenMainActiveChanged(); }
    void writeAnalogClockFace(bool x) { m_analogClockFace = x; emit analogClockFaceChanged(); }

    bool readBlink() { return m_blink; }
    bool readAls() { return m_als; }
    bool readProx() { return m_prox; }
    bool readSsp() { return m_ssp; }
    bool readDisplayOffWhenMainActive() { return m_displayOffWhenMainActive; }
    bool readAnalogClockFace() { return m_analogClockFace; }

    Q_INVOKABLE void readSettings();
    Q_INVOKABLE void writeSettings();
    Q_INVOKABLE QString readDaemonVersion();

signals:
    void versionChanged();

    void blinkChanged();
    void alsChanged();
    void proxChanged();
    void sspChanged();
    void displayOffWhenMainActiveChanged();
    void analogClockFaceChanged();

private:
    bool m_blink;
    bool m_als;
    bool m_prox;
    bool m_ssp;
    bool m_displayOffWhenMainActive;
    bool m_analogClockFace;
};


#endif // TOHOSETTINGS_H

