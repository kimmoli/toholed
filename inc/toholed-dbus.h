#ifndef TOHOLED_DBUS_H
#define TOHOLED_DBUS_H

#include <QtCore/QObject>

#define SERVICE_NAME "com.kimmoli.toholed"

class Toholed: public QObject
{
    Q_OBJECT
public slots:
    Q_SCRIPTABLE QString ping(const QString &arg);
    Q_SCRIPTABLE QString setVddState(const QString &arg);
    Q_SCRIPTABLE QString enableOled(const QString &arg);
    Q_SCRIPTABLE QString kill(const QString &arg);

private:
    static bool oled_init_done;
};



#endif // TOHOLED-DBUS_H
