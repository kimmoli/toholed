#include "weather.h"

const QString Weather::_weatherFile = "/home/nemo/.local/share/sailfish-weather/weather.json";

Weather::Weather(QObject *parent) :
    QObject(parent), watching(false)
{
}

Weather::~Weather()
{
    stopWatching();
}

bool Weather::startWatching()
{
    if (watching)
        return true;

    QFile weather(_weatherFile);

    /* If weather file does not exists, return string */
    if (!weather.exists())
        return false;

    _weatherFileNotifier = new QFileSystemWatcher(QStringList() << _weatherFile, this);

    connect(_weatherFileNotifier, SIGNAL(fileChanged(QString)), this, SLOT(processFile(QString)));

    watching = true;
    printf("Started watching weather changes.\n");

    return true;
}

void Weather::stopWatching()
{
    if (!watching)
        return;

    watching = false;

    _weatherFileNotifier->disconnect(SIGNAL(fileChanged(QString)));
    delete _weatherFileNotifier;
    _weatherFileNotifier = 0;

    printf("Stopped watching weather changes.\n");
}

void Weather::triggerUpdate()
{
    processFile(_weatherFile);
}

void Weather::processFile(QString filename)
{
    QFile weather(filename);

    if (!weather.exists())
    {
        stopWatching();
        return;
    }

    if (!weather.open(QFile::ReadOnly | QFile::Text))
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

    printf("Temperature in %s is %d degrees (updated %s)\n", qPrintable(currentLocation["city"].toString()),
            currentWeather["temperature"].toInt(),
            qPrintable(timestamp.toLocalTime().toString(Qt::SystemLocaleLongDate)));

    emit weatherUpdated(QString("%1d").arg(currentWeather["temperature"].toInt()));
}

