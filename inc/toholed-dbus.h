#ifndef TOHOLED_DBUS_H
#define TOHOLED_DBUS_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QTime>

#define SERVICE_NAME "com.kimmoli.toholed"

class Toholed: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    Toholed();

public slots:
    QString setVddState(const QString &arg);
    QString enableOled(const QString &arg);
    QString setOledAutoUpdate(const QString &arg);
    QString frontLed(const QString &arg);
    QString kill(const QString &arg);

private slots:
    void timerTimeout();


private:
    static bool oledInitDone;
    static bool vddEnabled;
    static bool oledAutoUpdate;
    static int timerCount;
    QTime prevTime;
    QTimer *timer;
};



#endif // TOHOLED-DBUS_H
