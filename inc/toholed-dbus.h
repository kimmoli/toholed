#ifndef TOHOLED_DBUS_H
#define TOHOLED_DBUS_H

#include <QtCore/QObject>
#include <QtCore/QTimer>

#define SERVICE_NAME "com.kimmoli.toholed"

class Toholed: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    Toholed();

public slots:
    QString ping(const QString &arg);
    QString setVddState(const QString &arg);
    QString enableOled(const QString &arg);
    QString frontLed(const QString &arg);
    QString kill(const QString &arg);

private slots:
    void timerTimeout();

    //Q_SCRIPTABLE

private:
    static bool oled_init_done;
    static int timerCount;
    QTimer *timer;
};



#endif // TOHOLED-DBUS_H
