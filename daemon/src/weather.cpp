#include "weather.h"

const QString Weather::_weatherSailfish = "/home/nemo/.local/share/sailfish-weather/weather.json";
const QString Weather::_weatherMeecast = "/home/nemo/.cache/harbour-meecast/current.xml";

Weather::Weather(QObject *parent) :
    QObject(parent), watching(false)
{
    _timestamp = 0;

    _weatherSailfishNotifier = 0;
    _weatherMeecastNotifier = 0;

    _weatherSailfishFile.setFileName(_weatherSailfish);
    _weatherMeecastFile.setFileName(_weatherMeecast);
}

Weather::~Weather()
{
    stopWatching();
}

bool Weather::startWatching()
{
    if (watching)
        return true;

    if (_weatherSailfishFile.exists()) {
        _weatherSailfishNotifier = new QFileSystemWatcher(this);
        _weatherSailfishNotifier->addPath(_weatherSailfish);
        connect(_weatherSailfishNotifier, SIGNAL(fileChanged(QString)), this, SLOT(processSailfish(QString)));
        processSailfish(_weatherSailfish);
        printf("Started watching Sailfish weather changes.\n");

        watching = true;
    }

    if (_weatherMeecastFile.exists()) {
        _weatherMeecastNotifier = new QFileSystemWatcher(this);
        _weatherMeecastNotifier->addPath(_weatherMeecast);
        connect(_weatherMeecastNotifier, SIGNAL(fileChanged(QString)), this, SLOT(processMeecast(QString)));
        processMeecast(_weatherMeecast);
        printf("Started watching Meecast weather changes.\n");

        watching = true;
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
    if (watching) {
        processSailfish(_weatherSailfish);
        processMeecast(_weatherMeecast);
    }
}

void Weather::processSailfish(const QString &)
{
    if (!_weatherSailfishFile.exists() || !_weatherSailfishFile.open(QFile::ReadOnly | QFile::Text))
    {
        return;
    }

    QTextStream in(&_weatherSailfishFile);
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

    _weatherSailfishFile.close();
}

void Weather::processMeecast(const QString &)
{
    if (!_weatherMeecastFile.exists() || !_weatherMeecastFile.open(QFile::ReadOnly | QFile::Text))
    {
        return;
    }

    QXmlQuery query;
    query.setFocus(&_weatherMeecastFile);
    QString timestamp;
    query.setQuery("/meecast/station/period[@itemnumber=\"1\"]/@start/string()");
    query.evaluateTo(&timestamp);
    timestamp = timestamp.trimmed();

    if (timestamp.toUInt() > _timestamp) {
        _timestamp = timestamp.toUInt();

        QString temperature;
        query.setQuery("/meecast/station/period[@itemnumber=\"1\"]/temperature/string()");
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

    _weatherMeecastFile.close();
}

