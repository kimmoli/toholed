#ifndef WEATHER_H
#define WEATHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QXmlQuery>

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
    void processSailfish(const QString &);
    void processMeecast(const QString &);

private:
    QFile _weatherSailfishFile;
    QFile _weatherMeecastFile;
    static const QString _weatherSailfish;
    static const QString _weatherMeecast;
    QFileSystemWatcher *_weatherSailfishNotifier;
    QFileSystemWatcher *_weatherMeecastNotifier;

    uint _timestamp;

};

#endif // WEATHER_H
