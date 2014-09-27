#include "notificationmanager.h"

#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QSettings>
#include <QDBusInterface>
#include <QDBusPendingReply>

class NotificationManagerPrivate
{
    Q_DECLARE_PUBLIC(NotificationManager)

public:
    NotificationManagerPrivate(NotificationManager *q)
        : q_ptr(q),
          interface(NULL),
          connected(false)
    { /*...*/ }

    NotificationManager *q_ptr;

    QDBusInterface *interface;

    bool connected;
};

NotificationManager::NotificationManager(QObject *parent)
    : QObject(parent), d_ptr(new NotificationManagerPrivate(this))
{
    Q_D(NotificationManager);
    QDBusConnection::sessionBus().registerObject("/org/freedesktop/Notifications", this, QDBusConnection::ExportAllSlots);

    d->interface = new QDBusInterface("org.freedesktop.DBus",
                                      "/org/freedesktop/DBus",
                                      "org.freedesktop.DBus");

    d->interface->call("AddMatch", "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'");

    this->initialize();
}

NotificationManager::~NotificationManager()
{
    Q_D(NotificationManager);
    delete d;
}


void NotificationManager::initialize()
{
    Q_D(NotificationManager);
    bool success = false;

    if(d->interface->isValid())
    {
        success = true;
        printf("Connected to Notifications D-Bus service.\n");
    }

    if(!(d->connected = success))
    {
        QTimer::singleShot(2000, this, SLOT(initialize()));
        printf("Failed to connect to Notifications D-Bus service.\n");
    }
}

QDBusInterface* NotificationManager::interface() const
{
    Q_D(const NotificationManager);
    return d->interface;
}

uint NotificationManager::Notify(const QString &app_name, uint replaces_id, const QString &app_icon,
                                 const QString &summary, const QString &body, const QStringList &actions,
                                 const QVariantHash &hints, int expire_timeout)
{
    Q_UNUSED(replaces_id);
    Q_UNUSED(app_icon);
    Q_UNUSED(actions);
    Q_UNUSED(expire_timeout);

    // Ignore notifcations from myself
    if (app_name == "toholed")
    {
        return 0;
    }

    printf("Got notification via dbus from %s\n", qPrintable(app_name));

    // Avoid sending a reply for this method call, since we've received it because we're eavesdropping.
    // The actual target of the method call will send the proper reply.

    Q_ASSERT(calledFromDBus());
    setDelayedReply(true);

    if (app_name == "messageserver5") /* emails */
    {
        emit this->emailNotify();
    }
    else if (app_name == "twitter-notifications-client")  /* Build-in twitter */
    {
        printf("twitter-notifications-client notification\n");

        emit this->twitterNotify();
    }
    else if (app_name == "irssi") /* This is irssi mqtt notification */
    {
        printf("irssi notification\n");

        emit this->irssiNotify();
    }
    else if (app_name == "commhistoryd")
    {
        QString category = hints.value("category", "").toString();
        if (category == "x-nemo.messaging.im")
        {
            printf("im notification\n");

            emit this->imNotify();
        }
        else
        {
            printf("Other commhistoryd notification: category=%s\n", qPrintable(category));
        }
    }
    else
    {
        QString subject = hints.value("x-nemo-preview-summary", "").toString();
        QString data = hints.value("x-nemo-preview-body", "").toString();
        QString category = hints.value("category", "").toString();

        printf("Other notification: subject=%s data=%s category=%s\n", qPrintable(subject), qPrintable(data), qPrintable(category));

        emit this->otherNotify();
    }

    return 0;
}
