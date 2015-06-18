#include "weather.h"

const QString Weather::_weatherSailfish = "/home/nemo/.local/share/sailfish-weather/weather.json";
const QString Weather::_weatherMeecast = "/home/nemo/.cache/harbour-meecast/current.xml";

Weather::Weather(QObject *parent) :
    QObject(parent), watching(false)
{
    _timestamp = 0;
}

Weather::~Weather()
{
    stopWatching();
}

bool Weather::startWatching()
{
    if (watching)
        return true;

    _weatherSailfishFile.setFileName(_weatherSailfish);

    /* If weather file does not exists, return string */
    if (_weatherSailfishFile.exists()) {
        _weatherSailfishNotifier = new QFileSystemWatcher(QStringList() << _weatherSailfish, this);

        connect(_weatherSailfishNotifier, SIGNAL(fileChanged(QString)), this, SLOT(processSailfish(QString)));

        watching = true;
        printf("Started watching Sailfish weather changes.\n");
    }

    _weatherMeecastFile.setFileName(_weatherMeecast);

    if (_weatherMeecastFile.exists()) {
        _weatherMeecastNotifier = new QFileSystemWatcher(QStringList() << _weatherMeecast, this);

        connect(_weatherMeecastNotifier, SIGNAL(fileChanged(QString)), this, SLOT(processMeecast(QString)));

        watching = true;
        printf("Started watching Meecast weather changes.\n");
    }

    return watching;
}

void Weather::stopWatching()
{
    if (!watching)
        return;

    watching = false;

    if (_weatherSailfishNotifier) {
        _weatherSailfishNotifier->disconnect(SIGNAL(fileChanged(QString)));
        delete _weatherSailfishNotifier;
        _weatherSailfishNotifier = 0;
    }

    if (_weatherMeecastNotifier) {
        _weatherMeecastNotifier->disconnect(SIGNAL(fileChanged(QString)));
        delete _weatherMeecastNotifier;
        _weatherMeecastNotifier = 0;
    }

    printf("Stopped watching weather changes.\n");
}

void Weather::triggerUpdate()
{
    processSailfish(_weatherSailfish);
    processMeecast(_weatherMeecast);
}

void Weather::processSailfish(QString filename)
{
    QFile weather(filename);

    if (!weather.exists() && !_weatherMeecastFile.exists())
    {
        stopWatching();
        return;
    }

    if (!weather.open(QFile::ReadOnly | QFile::Text) && !_weatherMeecastFile.exists())
    {
        stopWatching();
        return;
    }

    QTextStream in(&weather);
    QString weatherString = in.readAll();

    if (weatherString.isEmpty())
        return;

    QJsonDocument weatherJson = QJsonDocument::fromJson(weatherString.toUtf8());

    QJsonObject weatherObject = weatherJson.object();
    QJsonObject currentLocation = weatherObject["currentLocation"].toObject();
    QJsonObject currentWeather = currentLocation["weather"].toObject();

    QDateTime timestamp = QDateTime::fromString(currentWeather["timestamp"].toString(), Qt::ISODate);
    timestamp.setTimeSpec(Qt::UTC);

    if (timestamp.toTime_t() > _timestamp) {
        _timestamp = timestamp.toTime_t();

        printf("Temperature in %s is %d degrees (updated %s)\n", qPrintable(currentLocation["city"].toString()),
                currentWeather["temperature"].toInt(),
                qPrintable(timestamp.toLocalTime().toString(Qt::SystemLocaleLongDate)));

        emit weatherUpdated(QString("%1d").arg(currentWeather["temperature"].toInt()));
    }
}

void Weather::processMeecast(QString filename)
{
    QFile weather(filename);

    if (!weather.exists() && !_weatherSailfishFile.exists())
    {
        stopWatching();
        return;
    }

    if (!weather.open(QFile::ReadOnly | QFile::Text) && !_weatherSailfishFile.exists())
    {
        stopWatching();
        return;
    }

    QXmlQuery query;
    query.setFocus(&weather);
    QString endTimestamp;
    query.setQuery("/meecast/station/period[@current=\"true\"]/@end/string()");
    query.evaluateTo(&endTimestamp);
    endTimestamp = endTimestamp.trimmed();

    if (endTimestamp.toUInt() > _timestamp) {
        _timestamp = endTimestamp.toUInt();

        QString temperature;
        query.setQuery("/meecast/station/period[@current=\"true\"]/temperature/string()");
        query.evaluateTo(&temperature);
        temperature = temperature.trimmed();

        QString station;
        query.setQuery("/meecast/station/@name/string()");
        query.evaluateTo(&station);
        station = station.trimmed();

        printf("Temperature in %s is %d degrees (updated %s)\n", qPrintable(station),
                temperature.toInt(),
                qPrintable(QDateTime::fromTime_t(_timestamp).toString(Qt::SystemLocaleLongDate)));

        emit weatherUpdated(QString("%1d").arg(temperature.toInt()));
    }

    weather.close();
}

