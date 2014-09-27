#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QtDBus/QDBusContext>

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>

typedef QHash<QString, QString> QStringHash;

class NotificationManager : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

    Q_PROPERTY(QDBusInterface* interface READ interface)
public:
    explicit NotificationManager(QObject *parent = 0);
            ~NotificationManager();

    QDBusInterface* interface() const;

Q_SIGNALS:
    void twitterNotify();
    void facebookNotify(); /* Not yet implemented */
    void emailNotify();
    void irssiNotify();
    void imNotify();
    void otherNotify();

public Q_SLOTS:
    uint Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantHash &hints, int expire_timeout);

protected Q_SLOTS:
    void initialize();

private:
    class NotificationManagerPrivate *d_ptr;

    Q_DISABLE_COPY(NotificationManager)
    Q_DECLARE_PRIVATE(NotificationManager)
};

#endif // NOTIFICATIONMANAGER_H
