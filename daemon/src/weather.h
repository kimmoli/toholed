#ifndef WEATHER_H
#define WEATHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

class Weather : public QObject
{
    Q_OBJECT
public:
    explicit Weather(QObject *parent = 0);
    ~Weather();
    bool watching;

signals:
    void weatherUpdated(QString weatherNow);

public slots:
    bool startWatching();
    void stopWatching();
    void triggerUpdate();

private slots:
    void processFile(QString filename);

private:
    static const QString _weatherFile;
    QFileSystemWatcher *_weatherFileNotifier;

};

#endif // WEATHER_H
