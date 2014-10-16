/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * toholed daemon, d-bus server call method functions.
 *
 *
 *
 *
 */


#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusArgument>
#include <QtCore/QTimer>
#include <QTime>
#include <QThread>

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <poll.h>

#include "toholed-dbus.h"
#include "toholed.h"
#include "toh.h"
#include "oled.h"
#include "charger.h"
#include "icons.h"
#include "tsl2772.h"

#include <QImage>
#include <QPainter>
#include <QColor>

extern "C"
{
    #include "iphbd/libiphb.h"
}


static char screenBuffer[SCREENBUFFERSIZE] = { 0 };

/* Main */
Toholed::Toholed()
{
    oledInitDone = false;
    vddEnabled = false;
    interruptsEnabled = false;
    timerCount = 0;
    prevBrightness = BRIGHTNESS_MED;
    prevProx = 0;
    iconCALL = false;
    iconSMS = false;
    iconEMAIL = false;
    iconTWEET = false;
    iconIRC = false;
    systemUpdate = false;
    iconMITAKUULUU = false;
    ScreenCaptureOnProximity = false;
    activeHighlights = 0;
    ssNotifyReplacesId = 0;
    chargerConnected = false;
    mitakuuluuUnread = 0;
    silentProfile = false;
    wifiPowered = false;
    bluetoothPowered = false;
    cellularPowered = false;
    wifiConnected = false;
    bluetoothConnected = false;
    cellularConnected = false;
    offlineModeActive = false;
    lockDrawingMode = false;
    lockDrawingModeAppName = QString();
    lockDrawingModeTimeout = 2;

    reloadSettings();

    blinkTimer = new QTimer(this);
    blinkTimer->setInterval(200);
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blinkTimerTimeout( )));
    blinkTimerCount = 0;
    blinkNow = false;

    iphbRunning = false;
    iphbdHandler = iphb_open(0);

    if (!iphbdHandler)
        printf("Error opening iphb\n");

    iphb_fd = iphb_get_fd(iphbdHandler);

    iphbNotifier = new QSocketNotifier(iphb_fd, QSocketNotifier::Read);

    if (!QObject::connect(iphbNotifier, SIGNAL(activated(int)), this, SLOT(heartbeatReceived(int))))
    {
        delete iphbNotifier, iphbNotifier = 0;
        printf("failed to connect iphbNotifier\n");
    }
    else
    {
        iphbNotifier->setEnabled(false);
    }

    if (iphbNotifier)
        printf("iphb initialized succesfully\n");

    prevTime = QTime::currentTime();

    memset(screenBuffer, 0x00, SCREENBUFFERSIZE);

    thread = new QThread();
    worker = new Worker();

    worker->moveToThread(thread);
    connect(worker, SIGNAL(gpioInterruptCaptured()), this, SLOT(handleGpioInterrupt()));
    connect(worker, SIGNAL(proxInterruptCaptured()), this, SLOT(handleProxInterrupt()));
    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    interruptsEnabled = false;

    /* Get the current profile status */
    silentProfile = (getCurrentProfile() == "silent");

    /* do this automatically at startup */
    setVddState(true);
    enableOled();
    setInterruptEnable(true);

    /* Update screen and start iphb */
    heartbeatReceived(0);

    printf("initialisation complete\n");

    getCurrentNetworkConnectionStates();

    updateDisplay(true);
}

/* Timer routine to update OLED clock */
void Toholed::updateDisplay(bool timeUpdateOverride, int blinks)
{

    QTime current = QTime::currentTime();

    if (!timeUpdateOverride && lockDrawingMode)
    {
        /* Display is locked in drawing mode, and we arrived here due timer */
        lockDrawingModeTimeout--;

        if (lockDrawingModeTimeout == 0)
        {
            printf("Drawing mode released due timeout. Was locked by %s\n", qPrintable(lockDrawingModeAppName));

            lockDrawingMode = false;
            lockDrawingModeAppName = QString();
            timeUpdateOverride = true;
        }
    }

    /* Update only if minute has changed and oled is powered and initialized */

    if (((current.minute() != prevTime.minute()) || timeUpdateOverride) && vddEnabled && oledInitDone &&
            !lockDrawingMode && !systemUpdate)
    {
        prevTime = current;

        QString tNow = QString("%1:%2")
                        .arg((int) current.hour(), 2, 10, QLatin1Char(' '))
                        .arg((int) current.minute(), 2, 10, QLatin1Char('0'));
        QByteArray baNow = tNow.toLocal8Bit();

        int chargeLevel = chargerGetCapacity();
        QString firstCharIndicator;
        if ((chargerConnected && silentProfile && (timerCount & 1)) || (chargerConnected && !silentProfile))
        {
            firstCharIndicator = (chargeLevel < 100) ? "*" : ""; /* Charger icon */
        }
        else if ((chargerConnected && silentProfile && !(timerCount & 1)) || (!chargerConnected && silentProfile))
        {
            firstCharIndicator = (chargeLevel < 100) ? "!" : ""; /* Silent profile icon */
        }
        QString batNow = QString("%1%2%").arg(firstCharIndicator).arg(chargeLevel, 2, 10, QLatin1Char(' '));
        QByteArray babatNow = batNow.toLocal8Bit();

        clearOled(screenBuffer);

        if (analogClockFace) /* Experimental */
        {
            drawAnalogClock(current.hour(), current.minute(), screenBuffer);

            if ((iconSMS && iconMITAKUULUU && (timerCount & 1)) || (iconSMS && !iconMITAKUULUU))
            {
                drawIcon(110, 0, MESSAGE, screenBuffer);
            }
            else if ((iconSMS && iconMITAKUULUU && !(timerCount & 1)) || (!iconSMS && iconMITAKUULUU))
            {
                drawIcon(110, 0, MITAKUULUU, screenBuffer);
            }

            if (iconCALL)
                drawIcon(95, 0, CALL, screenBuffer);
            if (iconEMAIL)
                drawIcon(110, 16, MAIL, screenBuffer);
            if (iconTWEET)
                drawIcon(110, 33, TWEET, screenBuffer);
            if (iconIRC)
                drawIcon(110, 50, IRC, screenBuffer);

            /* Network type indicator is coded in pienifontti, g=2G, u=3G, l=4G (gms, umts, lte) */
            /* Wifi active = w */
            /* Bluetooth device connected = b */
            /* Flightmode = F */
            QString tmp = QString("%1")
                    .arg(offlineModeActive ? 'F' : (cellularPowered ? (cellularConnected ? networkType.at(0).toUpper() : networkType.at(0).toLower()) : ' '));
            drawSmallText(2, 0, tmp.toLocal8Bit().data(), screenBuffer);
            tmp = QString("%1")
                    .arg(wifiPowered ? (wifiConnected ? 'W' : 'w') : ' ');
            drawSmallText(0, 15, tmp.toLocal8Bit().data(), screenBuffer);
            tmp = QString("%1")
                    .arg(bluetoothPowered ? (bluetoothConnected ? 'B' : 'b') : ' ');
            drawSmallText(4, 31, tmp.toLocal8Bit().data(), screenBuffer);
        }
        else
        { /* Digital clock face */
            drawTime(0, 0, baNow.data(), screenBuffer);

            if ((iconSMS && iconMITAKUULUU && (timerCount & 1)) || (iconSMS && !iconMITAKUULUU))
            {
                drawIcon(iconPos[MESSAGE], 50, MESSAGE, screenBuffer);
            }
            else if ((iconSMS && iconMITAKUULUU && !(timerCount & 1)) || (!iconSMS && iconMITAKUULUU))
            {
                drawIcon(iconPos[MITAKUULUU], 50, MITAKUULUU, screenBuffer);
            }

            if (iconCALL)
                drawIcon(iconPos[CALL], 50, CALL, screenBuffer);
            if (iconEMAIL)
                drawIcon(iconPos[MAIL], 50, MAIL, screenBuffer);
            if (iconTWEET)
                drawIcon(iconPos[TWEET], 50, TWEET, screenBuffer);
            if (iconIRC)
                drawIcon(iconPos[IRC], 50, IRC, screenBuffer);

            if (!iconCALL && !iconEMAIL && !iconIRC && !iconMITAKUULUU && !iconSMS && !iconTWEET)
            {
                /* No notifications shown, we can show other stuff instead */
                /* Network type indicator is coded in pienifontti, g=2G, u=3G, l=4G (gms, umts, lte) */
                /* Wifi active = w */
                /* Bluetooth device connected = b */
                /* Flightmode = F */
                QString tmp = QString("%1 %2 %3")
                        .arg(offlineModeActive ? 'F' : (cellularPowered ? (cellularConnected ? networkType.at(0).toUpper() : networkType.at(0).toLower()) : ' '))
                        .arg(wifiPowered ? (wifiConnected ? 'W' : 'w') : ' ')
                        .arg(bluetoothPowered ? (bluetoothConnected ? 'B' : 'b') : ' ');
                drawSmallText(45, 50, tmp.toLocal8Bit().data(), screenBuffer);
            }
        }

        drawSmallText(0, 50, babatNow.data(), screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        if (!timeUpdateOverride)
            printf("Time now: %s Battery: %s (%d)\n", baNow.data(), babatNow.data(), timerCount );

        if ((blinks > 0) && blinkOnNotification)
        {
            blinkTimerCount = blinks;
            blinkTimer->start();
        }

        emit displayUpdated();
    }

}

/* iphb wakeup stuff */

void Toholed::heartbeatReceived(int sock)
{
    Q_UNUSED(sock);

    iphbStop();
    timerCount++;
    updateDisplay();
    iphbStart();
}

void Toholed::iphbStart()
{
    if (iphbRunning)
        return;

    if (!(iphbdHandler && iphbNotifier))
    {
        printf("iphbStart iphbHandler not ok\n");
        return;
    }

    time_t unixTime = iphb_wait(iphbdHandler, 25, 35 , 0);

    if (unixTime == (time_t)-1)
    {
        printf("iphbStart timer failed\n");
        return;
    }

    iphbNotifier->setEnabled(true);
    iphbRunning = true;

}

void Toholed::iphbStop()
{
    if (!iphbRunning)
        return;

    if (!(iphbdHandler && iphbNotifier))
    {
        printf("iphbStop iphbHandler not ok\n");
        return;
    }

    iphbNotifier->setEnabled(false);

    (void)iphb_discard_wakeups(iphbdHandler);

    iphbRunning = false;

}



/* DBus Exposed call methods */

QString Toholed::getVersion()
{
    return QString(APPVERSION);
}

void Toholed::reloadSettings()
{
    QSettings settings(QSettings::SystemScope, "harbour-toholed", "toholed");

    settings.beginGroup("basic");
    blinkOnNotification = settings.value("blink", true).toBool();
    proximityEnabled = settings.value("proximity", true).toBool();
    alsEnabled = settings.value("als", true).toBool();
    displayOffWhenMainActive = settings.value("displayOffWhenMainActive", false).toBool();
    analogClockFace = settings.value("analogClockFace", false).toBool();
    settings.endGroup();
}

QString Toholed::setSettings(const QDBusMessage &msg)
{
    QList<QVariant> args = msg.arguments();

    if (args.size() != 5)
        return QString("Failed");

    /* This must match with message sent from settings-ui */
    /* https://github.com/kimmoli/toholed-settings-ui.git */

    blinkOnNotification = QString::localeAwareCompare( args.at(0).toString(), "on") ? false : true;
    alsEnabled = QString::localeAwareCompare( args.at(1).toString(), "on") ? false : true;
    proximityEnabled = QString::localeAwareCompare( args.at(2).toString(), "on") ? false : true;
    displayOffWhenMainActive = QString::localeAwareCompare( args.at(3).toString(), "on") ? false : true;
    analogClockFace = QString::localeAwareCompare( args.at(4).toString(), "on") ? false : true;

    QSettings settings(QSettings::SystemScope, "harbour-toholed", "toholed");

    settings.beginGroup("basic");
    settings.setValue("blink", blinkOnNotification);
    settings.setValue("proximity", proximityEnabled);
    settings.setValue("als", alsEnabled);
    settings.setValue("displayOffWhenMainActive", displayOffWhenMainActive);
    settings.setValue("analogClockFace", analogClockFace);
    settings.endGroup();

    if (proximityEnabled || alsEnabled)
    {
        int fd = tsl2772_initComms(0x39);
        tsl2772_enableInterrupts(fd);
        tsl2772_closeComms(fd);
    }

    if (!proximityEnabled && !alsEnabled)
    {
        int fd = tsl2772_initComms(0x39);
        tsl2772_disableInterrupts(fd);
        tsl2772_closeComms(fd);
    }

    if (!alsEnabled)
    {
        setContrastOled(BRIGHTNESS_MED);
        prevBrightness = BRIGHTNESS_MED;
    }

    if (!proximityEnabled && !oledInitDone)
    {
        initOled(prevBrightness);
        oledInitDone = true;
    }

    updateDisplay(true);

    return QString("Yep");
}

QString Toholed::draw(const QDBusMessage& msg)
{
    int x, y, x1, y1, r, c;

    QList<QVariant> args = msg.arguments();

    if (args.count() == 0)
        return QString("Draw fail; missing arguments");

    if (!QString::localeAwareCompare( args.at(0).toString(), "clear"))
    {
        clearOled(screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("Clear done");
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "lock"))
    {
        if (args.count() != 3)
            return QString("Lock fail; expecting bool:(true/false) string:appname");

        if (!args.at(1).canConvert(QVariant::Bool) || !args.at(2).canConvert(QVariant::String))
            return QString("Lock fail; expecting bool:(true/false) string:appname");

        if (!lockDrawingMode && args.at(1).toBool())
        {
            /* Lock request, can be locked for this appname */
            lockDrawingMode = true;
            lockDrawingModeAppName = args.at(2).toString();
            lockDrawingModeTimeout = 2;
            printf("Drawing mode locked by %s\n", qPrintable(lockDrawingModeAppName));

            return QString("ok");
        }
        if (!lockDrawingMode && !args.at(1).toBool())
        {
            printf("Drawing mode lock already released\n");

            return QString("ok");
        }
        else if (lockDrawingMode && args.at(1).toBool() && lockDrawingModeAppName == args.at(2).toString())
        {
            /* Refresh lock request */
            lockDrawingModeTimeout = 2;
            printf("Drawing mode lock refresh by %s\n", qPrintable(lockDrawingModeAppName));

            return QString("ok");
        }
        else if (lockDrawingMode && !args.at(1).toBool() && lockDrawingModeAppName == args.at(2).toString())
        {
            /* Release request */
            lockDrawingMode = false;
            lockDrawingModeAppName = QString();
            lockDrawingModeTimeout = 2;
            updateDisplay(true);
            printf("Drawing mode lock released\n");

            return QString("ok");
        }

        printf("Drawing mode is already locked by %s -- ", qPrintable(lockDrawingModeAppName));
        printf("Attempt to lock/release by %s\n", qPrintable(args.at(2).toString()));

        return QString("Lock failed. Drawing mode is already locked by %1").arg(lockDrawingModeAppName);
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "invert"))
    {
        if (args.count() != 2)
            return QString("Invert fail; expecting bool:(true/false)");

        if (!args.at(1).canConvert(QVariant::Bool))
            return QString("Invert fail; expecting bool:(true/false)");

        invertOled(args.at(1).toBool());

        return QString(args.at(1).toBool() ? "Display inverted" : "Display normal");
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "derp"))
    {
        drawDerp(screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("derp");
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "pixel"))
    {
        if ((args.count() != 3) && (args.count() != 4))
            return QString("Pixel fail; expecting int32:x, int32:y {int32:color}");

        x = args.at(1).toInt();
        y = args.at(2).toInt();
        c = 1;

        if ( x < 0 || x >= OLEDWIDTH || y < 0 || y >=  OLEDHEIGHT )
            return QString("Pixel fail; x or y exceeds display area.");

        if (args.count() == 4)
            c = args.at(3).toInt();

        drawPixel(x, y, c, screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("Pixel x:%1 y:%2").arg(x).arg(y);
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "circle"))
    {
        if ((args.count() != 4) && (args.count() != 5))
            return QString("Circle fail; expecting int32:x, int32:y, int32:radius {int32:color}");

        x = args.at(1).toInt();
        y = args.at(2).toInt();
        r = args.at(3).toInt();
        c = 1;

        if ( x < 0 || x >= OLEDWIDTH || y < 0 || y >=  OLEDHEIGHT || r < 0 || r > OLEDWIDTH )
            return QString("Circle fail; x, y or r exceeds display area.");

        if (args.count() == 5)
            c = args.at(4).toInt();

        drawCircle(x, y, r, c, screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("Circle x:%1 y:%2 r:%3").arg(x).arg(y).arg(r);
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "line"))
    {
        if ((args.count() != 5) && (args.count() != 6))
            return QString("Line fail; expecting int32:x0, int32:y0, int32:x1, int32:y1 {int32:color}");

        x = args.at(1).toInt();
        y = args.at(2).toInt();
        x1 = args.at(3).toInt();
        y1 = args.at(4).toInt();
        c = 1;

        if ( x < 0 || x >= OLEDWIDTH || y < 0 || y >=  OLEDHEIGHT ||  x1 < 0 || x1 >= OLEDWIDTH || y1 < 0 || y1 >=  OLEDHEIGHT )
            return QString("Line fail; x, x1, y or y1 exceeds display area.");

        if (args.count() == 6)
            c = args.at(5).toInt();

        drawLine(x, y, x1, y1, c, screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("Line x0:%1 y0:%2 x1:%3 y1:%4").arg(x).arg(y).arg(x1).arg(y1);
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "time"))
    {
        if (args.count() != 4)
            return QString("time fail; expecting int32:x int32:y string:something");

        x = args.at(1).toInt();
        y = args.at(2).toInt();

        if ( x < 0 || x >= OLEDWIDTH || y < 0 || y >=  OLEDHEIGHT )
            return QString("time fail; x or y exceeds display area.");

        if (!args.at(3).canConvert(QVariant::String))
            return QString("time fail: string conversion failed");

        QString t = args.at(3).toString();

        if ((t.contains(':') &&  t.length() > 5) || (!t.contains(':') && t.length() > 4))
            return QString("time fail: too long");

        QByteArray baNow = t.toLocal8Bit();

        drawTime(x ,y , baNow.data(), screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("time %1").arg(t);
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "clock"))
    {
        if ((args.count() != 3) && (args.count() != 4))
            return QString("clock fail; expecting int32:hours(0..23) int32:minutes(0..59) {bool:clear}");

        if (args.at(1).toInt() >= 0 && args.at(1).toInt() < 24 && args.at(2).toInt() >= 0 && args.at(2).toInt() < 60)
        {
            if (args.at(3).toBool())
                clearOled(screenBuffer);

            drawAnalogClock(args.at(1).toInt(), args.at(2).toInt(), screenBuffer);

            if (oledInitDone)
                updateOled(screenBuffer);
        }
        else
            return QString("clock fail; expecting int32:hours(0..23) int32:minutes(0..59)");

        return QString("clock %1:%2").arg(args.at(1).toInt()).arg(args.at(2).toInt());
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "smalltext"))
    {
        if (args.count() != 4)
            return QString("smalltext fail; expecting int32:x int32:y string:something");

        x = args.at(1).toInt();
        y = args.at(2).toInt();

        if ( x < 0 || x >= OLEDWIDTH || y < 0 || y >=  OLEDHEIGHT )
            return QString("small fail; x or y exceeds display area.");

        if (!args.at(3).canConvert(QVariant::String))
            return QString("smalltext fail: string conversion failed");

        drawSmallText(x, y, args.at(3).toString().toLocal8Bit().data(), screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("smalltext done");
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "bitmap"))
    {
        return QString("draw \"bitmap\" is obsolete. Use drawPicture method.");
    }

    return QString("Draw fail; Unsupported function");
}


QString Toholed::setScreenCaptureOnProximity(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    if (args.count() != 2)
        return QString("SSP Failed, requires on|off path");


    QString turn = args.at(0).toString();
    ScreenCaptureOnProximityStorePath = args.at(1).toString();

    ScreenCaptureOnProximity =  QString::localeAwareCompare( turn, "on") ? false : true;

    if (ScreenCaptureOnProximity)
    {
        printf("Screen capture on proximity interrupt enabled\n");
        return QString("Screen capture on proximity interrupt enabled");
    }
    else
    {
        printf("Screen capture on proximity interrupt disabled\n");
        return QString("Screen capture on proximity interrupt disabled");
    }
}


/* Function to set VDD (3.3V for OH) */
void Toholed::setVddState(bool turn)
{
    printf("VDD Control request turn %s : ", (turn ? "on" : "off"));

    usleep(100000);

    if (vddEnabled == turn)
    {
        printf("Already in that state\n");
        return;
    }

    if (controlVdd( ( turn ? 1 : 0) ) < 0)
    {
        vddEnabled = false;
        printf("FAILED\n");
    }
    else
    {
        vddEnabled = turn;
        printf("OK\n");
    }

    usleep(350000); /* Wait for the reset ic RT9818C-29GV to release reset */
}

/* Initialze and clear oled */
void Toholed::enableOled()
{
    if (vddEnabled)
    {
        clearOled(screenBuffer);
        updateOled(screenBuffer);
        initOled(0);
        drawDerp(screenBuffer);
        updateOled(screenBuffer);
        sleep(2);

        oledInitDone = true;
        printf("OLED Display initialized and cleared\n");
    }
    else
    {
        oledInitDone = false;
        printf("OLED Display initialize failed (VDD Not enabled)\n");
    }

}


/*
 *    Interrupt stuff
 */


int Toholed::setInterruptEnable(bool turn)
{
    int fd;

    if(turn)
    {
        mutex.lock();

        printf("Enabling interrupts\n");

        fd = tsl2772_initComms(0x39);
        if (fd <0)
        {
            printf("Failed to start communication with TSL2772\n");
            mutex.unlock();
            return -1;
        }
        tsl2772_initialize(fd);
        tsl2772_clearInterrupt(fd);
        tsl2772_closeComms(fd);

        printf("TSL2772 initialised succesfully\n");

        gpio_fd = getTohInterrupt();

        if (gpio_fd > -1)
            printf("TOH Interrupt registered\n");

        proximity_fd = getProximityInterrupt();

        if (proximity_fd > -1)
            printf("Proximity Interrupt registered\n");

        if ((gpio_fd > -1) && (proximity_fd > -1))
        {
            worker->abort();
            thread->wait(); // If the thread is not running, this will immediately return.

            worker->requestWork(gpio_fd, proximity_fd);

            printf("Worker started\n");

            interruptsEnabled = true;
            mutex.unlock();

            return 1;
        }
        else
        {
            printf("Failed to register TOH or proximity interrupt\n");
            interruptsEnabled = false;
            mutex.unlock();
            return -1;
        }
    }
    else
    {

        printf("Disabling interrupts\n");

        interruptsEnabled = false;

        worker->abort();
        thread->wait();
        delete thread;
        delete worker;

        releaseTohInterrupt(gpio_fd);
        releaseProximityInterrupt(proximity_fd);

        mutex.unlock();
        return 1;
    }

}

/* Tweetian handler */

void Toholed::handleTweetian(const QDBusMessage& msg)
{
    Q_UNUSED(msg);
    printf("You have been mentioned in a Tweet\n");

    iconTWEET = true;

    updateDisplay(true, 2);
}

/* Communi IRC handler */

void Toholed::handleCommuni(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    int ah = args.at(0).toInt();
    printf("IRC: Active highlights %d\n", ah);

    if (ah > activeHighlights) /* Number of active highlights increased */
    {
        iconIRC = true;
        updateDisplay(true, 2);
    }
    else if ((ah == 0) && iconIRC) /* Active highlights all read */
    {
        iconIRC = false;
        updateDisplay(true, 0);
    }

    activeHighlights = ah;
}


/* Incoming phonecall */
void Toholed::handleCall(const QDBusMessage& msg)
{

    QList<QVariant> args = msg.arguments();

    printf("sig_call_state_ind says: %s\n", qPrintable(args.at(0).toString()));

    if ( !(QString::localeAwareCompare( args.at(0).toString(), "ringing")) )
    {
        printf("Incoming call\n");
        iconCALL = true;

        updateDisplay(true, 0);
        blinkTimerCount = 1000;
        blinkTimer->start(); /* Use the blinkTimer to blink the screen while phone is ringing */
    }
    else if ( iconCALL && !(QString::localeAwareCompare( args.at(0).toString(), "active")) )
    {
        printf("Call answered or placing new call when missed call indicated\n");
        iconCALL = false;

        updateDisplay(true, 0);
        blinkTimerCount = 0;
    }
    else if (!(QString::localeAwareCompare( args.at(0).toString(), "none")) )
    {
        /* Just stop blinking */
        blinkTimerCount = 0;
    }
}

void Toholed::handleDisplayStatus(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    int tmp;

    printf("Display status changed to %s\n", qPrintable(args.at(0).toString()));

    if (!(QString::localeAwareCompare( args.at(0).toString(), "on")))
    {
        tmp = checkOled();

        if (tmp == -1)
        {
            printf("Failed to check OLED status\n");
        }
        else if (tmp != 1)
        {
            printf("Display is off (%02x) - Reinitializing.\n", tmp);

            initOled(prevBrightness);
            oledInitDone = true;
            updateDisplay(true);

            int fd = tsl2772_initComms(0x39);
            tsl2772_initialize(fd);
            tsl2772_closeComms(fd);
        }

        if (displayOffWhenMainActive)
        {
            deinitOled();
            oledInitDone = false;
            int fd = tsl2772_initComms(0x39);
            tsl2772_disableInterrupts(fd);
            tsl2772_closeComms(fd);
        }
    }
    else if (!(QString::localeAwareCompare( args.at(0).toString(), "off")))
    {
        if (displayOffWhenMainActive)
        {
            initOled(prevBrightness);
            oledInitDone = true;
            updateDisplay(true);

            int fd = tsl2772_initComms(0x39);
            tsl2772_enableInterrupts(fd);
            tsl2772_closeComms(fd);
        }
    }
}


void Toholed::handleNotificationClosed(const QDBusMessage& msg)
{

    QList<QVariant> outArgs = msg.arguments();

    unsigned int notificationId = outArgs.at(0).toInt();

    /* Manage the screenshot notification id clear to zero.
     * if id is nonzero and non-existent, it will not appear
     */

    if (notificationId == ssNotifyReplacesId)
    {
        printf("Screenshot notification id %d closed\n", notificationId);
        ssNotifyReplacesId = 0;
    }

    iconSMS = false;
    iconEMAIL = false;
    iconCALL = false;
    iconTWEET = false;
    iconIRC = false;
    systemUpdate = false;

    blinkTimerCount = 0;

    updateDisplay(true);
}

/* GPIO interrupt handler */

void Toholed::handleGpioInterrupt()
{
    int fd;
    unsigned long alsC0, alsC1, prox;
    unsigned int newBrightness = BRIGHTNESS_MED;

    mutex.lock();

    fd = tsl2772_initComms(0x39);
    if (fd <0)
    {
        printf("failed to start communication with TSL2772\n");
        mutex.unlock();
        return;
    }
    tsl2772_disableInterrupts(fd);
    alsC0 = tsl2772_getADC(fd, 0);
    alsC1 = tsl2772_getADC(fd, 1);
    prox = tsl2772_getADC(fd, 2);

    tsl2772_clearInterrupt(fd);
    tsl2772_closeComms(fd);

    if (alsEnabled)
    {
        if (alsC0 < ALSLIM_BRIGHTNESS_LOW)
            newBrightness = BRIGHTNESS_LOW;
        else if (alsC0 > ALSLIM_BRIGHTNESS_HIGH)
            newBrightness = BRIGHTNESS_HIGH;
        else
            newBrightness = BRIGHTNESS_MED;

        if (newBrightness != prevBrightness)
        {
            printf("Interrupt: Auto brightness adjust: ALS C0 %5lu C1 %5lu prox %5lu\n", alsC0, alsC1, prox);

            /* set new interrupt thresholds */
            fd = tsl2772_initComms(0x39);

            if (newBrightness == BRIGHTNESS_LOW)
                tsl2772_setAlsThresholds(fd, ALSLIM_BRIGHTNESS_LOW + ALS_HYST_LOW, 0);
            else if (newBrightness == BRIGHTNESS_HIGH)
                tsl2772_setAlsThresholds(fd, 0xffff, ALSLIM_BRIGHTNESS_HIGH - ALS_HYST_HIGH);
            else
                tsl2772_setAlsThresholds(fd, ALSLIM_BRIGHTNESS_HIGH, ALSLIM_BRIGHTNESS_LOW);

            tsl2772_closeComms(fd);

            setContrastOled(newBrightness);
            prevBrightness = newBrightness;
        }
    }

    if (proximityEnabled)
    {
        if ((prox > prox_limit) && (prevProx < prox_limit))
        {
            printf("Interrupt: Proximity detect:       ALS C0 %5lu C1 %5lu prox %5lu\n", alsC0, alsC1, prox);
            fd = tsl2772_initComms(0x39);
            tsl2772_setProxThresholds(fd, 0xFFFF, prox_limit);
            tsl2772_closeComms(fd);
            deinitOled();
            oledInitDone = false;
        }
        else if ((prox < prox_limit) && (prevProx > prox_limit))
        {
            printf("Interrupt: Proximity cleared:      ALS C0 %5lu C1 %5lu prox %5lu\n", alsC0, alsC1, prox);

            fd = tsl2772_initComms(0x39);
            tsl2772_setProxThresholds(fd, prox_limit, 0);
            tsl2772_closeComms(fd);
            initOled(prevBrightness);
            oledInitDone = true;

            updateDisplay(true);
        }

        prevProx = prox;
    }

    fd = tsl2772_initComms(0x39);
    tsl2772_enableInterrupts(fd);
    tsl2772_closeComms(fd);

    mutex.unlock();

}

/* Front proximity interrupt */

void Toholed::handleProxInterrupt()
{
    bool prox = getProximityStatus();

    if (prox && ScreenCaptureOnProximity) /* If enabled, we save screen-capture on front proximity interrupt */
    {
        printf("Proximity interrupt - proximity, taking screenshot\n");


        QDate ssDate = QDate::currentDate();
        QTime ssTime = QTime::currentTime();

        ssFilename = QString("%8/ss%1%2%3-%4%5%6-%7.png")
                        .arg((int) ssDate.day(),    2, 10, QLatin1Char('0'))
                        .arg((int) ssDate.month(),  2, 10, QLatin1Char('0'))
                        .arg((int) ssDate.year(),   2, 10, QLatin1Char('0'))
                        .arg((int) ssTime.hour(),   2, 10, QLatin1Char('0'))
                        .arg((int) ssTime.minute(), 2, 10, QLatin1Char('0'))
                        .arg((int) ssTime.second(), 2, 10, QLatin1Char('0'))
                        .arg((int) ssTime.msec(),   3, 10, QLatin1Char('0'))
                        .arg(ScreenCaptureOnProximityStorePath);


        QDBusMessage m = QDBusMessage::createMethodCall("org.nemomobile.lipstick",
                                                        "/org/nemomobile/lipstick/screenshot",
                                                        "",
                                                        "saveScreenshot" );

        QList<QVariant> args;
        args.append(ssFilename);
        m.setArguments(args);

        if (QDBusConnection::sessionBus().send(m))
            printf("Screenshot success to %s\n", qPrintable(ssFilename));
        else
            printf("Screenshot failed\n");

        notificationSend("Screenshot saved", ssFilename);

    }
}


void Toholed::notificationSend(QString summary, QString body)
{

    QDBusInterface notifyCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "");

    QVariantHash hints;
    hints.insert("x-nemo-preview-summary", summary);

    QList<QVariant> args;
    args.append("toholed");
    args.append(ssNotifyReplacesId);
    args.append("icon-m-notifications");
    args.append(summary);
    args.append(body);
    args.append((QStringList() << "default" << ""));
    args.append(hints);
    args.append(-1);

    QDBusMessage notifyCallReply = notifyCall.callWithArgumentList(QDBus::AutoDetect, "Notify", args);

    QList<QVariant> outArgs = notifyCallReply.arguments();

    ssNotifyReplacesId = outArgs.at(0).toInt();

    printf("Notification sent, got id %d\n", ssNotifyReplacesId);

}

void Toholed::handleNotificationActionInvoked(const QDBusMessage& msg)
{

    QList<QVariant> outArgs = msg.arguments();

    unsigned int notificationId = outArgs.at(0).toInt();

    /* Manage the screenshot notification id action. */
    if (notificationId == ssNotifyReplacesId)
    {
        printf("Screenshot notification id %d Action invoked - opening image %s\n", notificationId, qPrintable(ssFilename));

        QDBusMessage m = QDBusMessage::createMethodCall("com.jolla.gallery",
                                                        "/com/jolla/gallery/ui",
                                                        "com.jolla.gallery.ui",
                                                        "showImages" );

        QList<QVariant> args;
        args.append((QStringList() << ssFilename));
        m.setArguments(args);

        if (!QDBusConnection::sessionBus().send(m))
            printf("Failed to invoke gallery to show %s\n", qPrintable(ssFilename));
    }


}


/* Charger */

void Toholed::handleChargerStatus(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    QString tmp = args.at(0).toString();

    if (!(QString::localeAwareCompare( tmp, "charger_connected")) && !chargerConnected)
    {
        printf("Charger connected\n");
        chargerConnected = true;

        updateDisplay(true, 0);
    }
    else if (!(QString::localeAwareCompare( tmp, "charger_disconnected")) && chargerConnected)
    {
        printf("Charger disconnected\n");
        chargerConnected = false;

        updateDisplay(true, 0);
    }

}


void Toholed::handleMitakuuluu(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    int mkUnread = args.at(0).toInt();
    printf("Mitakuuluu: total unread %d\n", mkUnread);

    if (mkUnread > mitakuuluuUnread) /* Number of total unread increased */
    {
        iconMITAKUULUU = true;

        updateDisplay(true, 2);
    }
    else if ((mkUnread == 0) && iconMITAKUULUU) /* All read */
    {
        iconMITAKUULUU = false;

        updateDisplay(true, 0);
    }

    mitakuuluuUnread = mkUnread;
}

void Toholed::handleProfileChanged(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    bool changed = args.at(0).toBool();

    if (changed)
    { 
        QString tmp = args.at(2).toString();
        printf("Profile changed to %s\n", qPrintable(tmp));

        if (tmp == "silent")
        {
            silentProfile = true;

            updateDisplay(true);
        }
        else
        {
            silentProfile = false;

            updateDisplay(true);
        }
    }
}

QString Toholed::getCurrentProfile()
{
    QDBusInterface getProfileCall("com.nokia.profiled", "/com/nokia/profiled", "com.nokia.profiled");
    getProfileCall.setTimeout(2);

    QDBusMessage getProfileCallReply = getProfileCall.call(QDBus::AutoDetect, "get_profile");

    return getProfileCallReply.arguments().at(0).toString();
}

void Toholed::handleAlarm(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    int tmp = args.at(0).toInt();

    if (tmp == 0) /* Alarm dialog on screen */
    {
        updateDisplay(true);
        blinkTimerCount = 1000;
        blinkTimer->start();
        printf("Alarm activated.\n");
    }
    else if (tmp == 1) /* Alarm dialog not on screen */
    {
        blinkTimerCount = 0;
        printf("Alarm cleared.\n");
    }
}

void Toholed::blinkTimerTimeout( )
{
    if (!oledInitDone || !blinkOnNotification)
    {
        blinkTimer->stop();
        return;
    }

    blinkNow = !blinkNow;

    if (blinkNow)
    {
        mutex.lock();
        invertOled(true);
        setContrastOled(BRIGHTNESS_HIGH);
        mutex.unlock();
    }
    else
    {
        mutex.lock();
        blinkTimerCount--;
        invertOled(false);
        setContrastOled(prevBrightness);
        mutex.unlock();
    }

    if (blinkTimerCount <= 0 && !blinkNow)
    {
        blinkTimer->stop();
    }
}

void Toholed::handleNetworkRegistration(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    QVariant val = args.at(1).value<QDBusVariant>().variant();

    printf("NetworkRegistration: %s changed: ", qPrintable(args.at(0).toString()) );
    if (val.type() == 10)
    {
        printf("%s\n", qPrintable(val.toString()));
    }
    else
    {
        printf("%d\n", val.toInt());
    }

    if (args.at(0).toString() == "Technology" && networkType != val.toString())
    {
        networkType = val.toString();

        updateDisplay(true);
    }
}

void Toholed::handleBluetooth(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    QVariant val = args.at(1).value<QDBusVariant>().variant();

    printf("Bluetooth: %s changed: ", qPrintable(args.at(0).toString()) );

    printf("%s (%d)\n", qPrintable(val.toString()), val.type());

    if (args.at(0).toString() == "Powered" && bluetoothPowered != val.toBool())
    {
        bluetoothPowered = val.toBool();

        updateDisplay(true);
    }
    if (args.at(0).toString() == "Connected" && bluetoothConnected != val.toBool())
    {
        bluetoothConnected = val.toBool();

        updateDisplay(true);
    }

}

void Toholed::handleWifi(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    QVariant val = args.at(1).value<QDBusVariant>().variant();

    printf("Wifi: %s changed: ", qPrintable(args.at(0).toString()) );

    printf("%s (%d)\n", qPrintable(val.toString()), val.type());

    if (args.at(0).toString() == "Powered" && wifiPowered != val.toBool())
    {
        wifiPowered = val.toBool();

        updateDisplay(true);
    }
    if (args.at(0).toString() == "Connected" && wifiConnected != val.toBool())
    {
        wifiConnected = val.toBool();

        updateDisplay(true);
    }

}

void Toholed::handleCellular(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    QVariant val = args.at(1).value<QDBusVariant>().variant();

    printf("Cellular: %s changed: ", qPrintable(args.at(0).toString()) );

    printf("%s (%d)\n", qPrintable(val.toString()), val.type());

    if (args.at(0).toString() == "Powered" && cellularPowered != val.toBool())
    {
        cellularPowered = val.toBool();

        updateDisplay(true);
    }
    if (args.at(0).toString() == "Connected" && cellularConnected != val.toBool())
    {
        cellularConnected = val.toBool();

        updateDisplay(true);
    }
}

void Toholed::handleConnmanManager(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    QVariant val = args.at(1).value<QDBusVariant>().variant();

    if (args.at(0).toString() == "OfflineMode" && offlineModeActive != val.toBool())
    {
        offlineModeActive = val.toBool();

        updateDisplay(true);
    }
}


void Toholed::getCurrentNetworkConnectionStates()
{
    /* Network type*/

    QVariantMap m = getDbusProperties("org.ofono", "/ril_0", "org.ofono.NetworkRegistration", QDBusConnection::systemBus());

    networkType = m.value("Technology", "NONE").toString();

    /* Bluetooth */

    QVariantMap b = getDbusProperties("net.connman", "/net/connman/technology/bluetooth", "net.connman.Technology", QDBusConnection::systemBus());

    bluetoothPowered = b.value("Powered", false).toBool();
    bluetoothConnected = b.value("Connected", false).toBool();

    QVariantMap w = getDbusProperties("net.connman", "/net/connman/technology/wifi", "net.connman.Technology", QDBusConnection::systemBus());

    wifiPowered = w.value("Powered", false).toBool();
    wifiConnected = w.value("Connected", false).toBool();

    QVariantMap c = getDbusProperties("net.connman", "/net/connman/technology/cellular", "net.connman.Technology", QDBusConnection::systemBus());

    cellularPowered = c.value("Powered", false).toBool();
    cellularConnected = c.value("Connected", false).toBool();

    QVariantMap f = getDbusProperties("net.connman", "/", "net.connman.Manager", QDBusConnection::systemBus());

    offlineModeActive = f.value("OfflineMode", false).toBool();

    printf("Current network Technology is '%s'\n", qPrintable(networkType));
    printf("Bluetooth is %s and %s\n", bluetoothPowered ? "Powered" : "Not powered", bluetoothConnected ? "Connected" : "Not connected");
    printf("Wifi is %s and %s\n", wifiPowered ? "Powered" : "Not powered", wifiConnected ? "Connected" : "Not connected");
    printf("Cellular is %s and %s\n", cellularPowered ? "Powered" : "Not powered", cellularConnected ? "Connected" : "Not connected");
    printf("Flightmode is %s\n", offlineModeActive ? "Active" : "Not active");
}


QVariantMap Toholed::getDbusProperties(const QString & service, const QString & path, const QString & interface, const QDBusConnection & connection)
{
    QDBusInterface getProperties(service, path, interface, connection);
    getProperties.setTimeout(2000);

    QDBusMessage getPropertiesReply = getProperties.call(QDBus::AutoDetect, "GetProperties");

    const QDBusArgument myArgs = getPropertiesReply.arguments().at(0).value<QDBusArgument>();

    QVariantMap map;

    myArgs.beginMap();

    while ( ! myArgs.atEnd())
    {
        QString key;
        QDBusVariant value;
        myArgs.beginMapEntry();
        myArgs >> key >> value;
        myArgs.endMapEntry();
        map.insert(key, value.variant());
    }
    myArgs.endMap();

    return map;
}

/* Slots for Notificationsmanager */

void Toholed::handleEmailNotify()
{
    printf("email notification\n");

    iconEMAIL = true;

    updateDisplay(true, 2);
}

void Toholed::handleTwitterNotify()
{
    printf("twitter notification\n");

    iconTWEET = true;

    updateDisplay(true, 2);
}

void Toholed::handleFacebookNotify()
{
    printf("facebook notification.\n");
}

void Toholed::handleIrssiNotify()
{
    printf("irssi notification.\n");

    iconIRC = true;

    updateDisplay(true, 2);
}

void Toholed::handleImNotify()
{
    printf("im notification.\n");

    iconSMS = true;

    updateDisplay(true, 2);
}

void Toholed::handleSmsNotify()
{
    printf("sms notification\n");

    iconSMS = true;

    updateDisplay(true, 2);
}

void Toholed::handleOtherNotify()
{
    printf("other notification\n");

    updateDisplay(true, 2);
}

void Toholed::handleSystemUpdateNotify()
{
    printf("system update notification\n");

    systemUpdate = true;

    clearOled(screenBuffer);
    drawUpdateTime(screenBuffer);
    if (oledInitDone)
        updateOled(screenBuffer);

    blinkTimerCount = 1000;
    blinkTimer->start();
}

QByteArray Toholed::captureOled()
{
    char * sb = screenBuffer;

    QImage pm(OLEDWIDTH, OLEDHEIGHT, QImage::Format_RGB32);
    pm.fill(Qt::black);
    QPainter p;
    p.begin(&pm);
    p.setPen(Qt::white);

    int i, n;

    for (n=0; n<OLEDHEIGHT ; n++)
    {
        for (i=0 ; i<OLEDWIDTH ; i++)
        {
            if (((*(sb+(n/8)+(i*8))) & ( 0x01 << (n%8) )) == ( 0x01 << (n%8) ) )
                p.drawPoint(i, n);
        }
    }

    p.end();

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pm.save(&buffer, "PNG"); // writes image into ba in PNG format

    return ba;
}


QString Toholed::drawPicture(const QDBusMessage &msg)
{
    char * sb = screenBuffer;
    int x0, y0, x, y;

    QList<QVariant> args = msg.arguments();

    if (args.count() != 3)
        return QString("drawPicture failed. Expecting; int32:x int32:y bytearray:data");

    x0 = args.at(0).toInt();
    y0 = args.at(1).toInt();

    if ((x0 >= OLEDWIDTH) || (y0 >= OLEDHEIGHT))
        return QString("drawPicture failed. x or y exceeds display area.");

    QImage image;

    if (!image.loadFromData(args.at(2).toByteArray()))
        return QString("drawPicture failed. Unable to decode input data.");

    if (x0 < -image.size().width() || y0 < -image.size().height())
        return QString("drawPicture failed. Image negative offset too large");

    for (y = 0; y<image.size().height(); y++)
    {
        for (x = 0; x<image.size().width(); x++)
        {
            int x1 = x0 + x;
            int y1 = y0 + y;

            if ((x1 >= 0) && (y1 >= 0) && (x1 < OLEDWIDTH) && (y1 < OLEDHEIGHT))
            {
                if ( qAlpha(image.pixel(x, y)) > 127) /* Pixel is opaque, draw it */
                {
                    if ( qGray(image.pixel(x, y)) > 127) /* > Gray 50% */
                        *(sb+(y1/8)+(x1*8)) |= (0x01 << (y1%8));
                    else
                        *(sb+(y1/8)+(x1*8)) &= ~(0x01 << (y1%8));
                }
            }
        }
    }

    if (oledInitDone)
        updateOled(screenBuffer);

    return QString("ok");
}


void Toholed::handleAlarmPresentChanged()
{
    printf("hoplaa\n");
}
