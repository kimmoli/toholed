#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <sailfishapp.h>
#include <QQuickImageProvider>
#include <QImage>
#include <QtDBus/QtDBus>
#include <QDBusArgument>


class imageProvider : public QQuickImageProvider
{
public:
    imageProvider() : QQuickImageProvider(QQuickImageProvider::Image)
    {
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    {
        qDebug() << id;

        int width = 128;
        int height = 64;

        if (size)
            *size = QSize(width, height);

        QDBusInterface getScreenShotCall("com.kimmoli.toholed", "/", "com.kimmoli.toholed", QDBusConnection::systemBus());
        getScreenShotCall.setTimeout(2000);

        QDBusMessage getScreenShotReply = getScreenShotCall.call(QDBus::AutoDetect, "captureOled");

        QImage screenShot(width, height, QImage::Format_RGB32);
        screenShot.fill(Qt::white);

        if (getScreenShotReply.type() == QDBusMessage::ErrorMessage)
        {
            qDebug() << "Error getting screenshot:" << getScreenShotReply.errorMessage();
        }
        else
        {
            QByteArray screenShot_ba = getScreenShotReply.arguments().at(0).toByteArray();

            screenShot.loadFromData(screenShot_ba, "PNG");
        }

        if (requestedSize.width() > 0 && requestedSize.height() > 0)
            screenShot = screenShot.scaled(requestedSize);

        return screenShot;
    }
};

#endif // IMAGEPROVIDER_H
