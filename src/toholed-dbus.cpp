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

extern "C"
{
    #include "iphbd/libiphb.h"
}


static char screenBuffer[SCREENBUFFERSIZE] = { 0 };

/* Main */
Toholed::Toholed()
{
    reloadSettings();

    timer = new QTimer(this);
    timer->setInterval(30000);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    timer->start();

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

    timeUpdateOverride = false;
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
}

/* Timer routine to update OLED clock */
void Toholed::timerTimeout()
{

    QTime current = QTime::currentTime();

    /* Update only if minute has changed and oled is powered and initialized */

    if (((current.minute() != prevTime.minute()) || timeUpdateOverride) && vddEnabled && oledInitDone)
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
        drawTime(baNow.data(), screenBuffer);
        drawBatteryLevel(babatNow.data(), screenBuffer);

        if ((iconSMS && iconMITAKUULUU && (timerCount & 1)) || (iconSMS && !iconMITAKUULUU))
        {
            drawIcon(MESSAGE, screenBuffer);
        }
        else if ((iconSMS && iconMITAKUULUU && !(timerCount & 1)) || (!iconSMS && iconMITAKUULUU))
        {
            drawIcon(MITAKUULUU, screenBuffer);
        }

        if (iconCALL)
            drawIcon(CALL, screenBuffer);
        if (iconEMAIL)
            drawIcon(MAIL, screenBuffer);
        if (iconTWEET)
            drawIcon(TWEET, screenBuffer);
        if (iconIRC)
            drawIcon(IRC, screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        if (!timeUpdateOverride)
            printf("Time now: %s Battery: %s\n", baNow.data(), babatNow.data() );

        timeUpdateOverride = false;
    }

}

/* iphb wakeup stuff */

void Toholed::heartbeatReceived(int sock)
{
    Q_UNUSED(sock);

    iphbStop();
    timeUpdateOverride = true;
    timerCount++;
    printf("Wakywaky by iphb (%d)\n", timerCount);
    timerTimeout();
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

QString Toholed::testSomething()
{

    return QString("Sorry, there is nothing to test...");
}

void Toholed::reloadSettings()
{
    QSettings settings(QSettings::SystemScope, "harbour-toholed", "toholed");

    settings.beginGroup("basic");
    blinkOnNotification = settings.value("blink", true).toBool();
    proximityEnabled = settings.value("proximity", true).toBool();
    alsEnabled = settings.value("als", true).toBool();
    settings.endGroup();
}

QString Toholed::setSettings(const QDBusMessage &msg)
{
    QList<QVariant> args = msg.arguments();

    if (args.size() != 4)
        return QString("Failed");

    blinkOnNotification = QString::localeAwareCompare( args.at(0).toString(), "on") ? false : true;
    alsEnabled = QString::localeAwareCompare( args.at(1).toString(), "on") ? false : true;
    proximityEnabled = QString::localeAwareCompare( args.at(2).toString(), "on") ? false : true;
    /* chargemon args.at(3) */

    QSettings settings(QSettings::SystemScope, "harbour-toholed", "toholed");

    settings.beginGroup("basic");
    settings.setValue("blink", blinkOnNotification);
    settings.setValue("proximity", proximityEnabled);
    settings.setValue("als", alsEnabled);
    settings.endGroup();

    int fd = tsl2772_initComms(0x39);
    tsl2772_enableInterrupts(fd);
    tsl2772_closeComms(fd);

    if (!proximityEnabled && !oledInitDone)
    {
        initOled(prevBrightness);
        oledInitDone = true;
        timeUpdateOverride = true;
    }

    if (!alsEnabled)
    {
        setContrastOled(BRIGHTNESS_MED);
        prevBrightness = BRIGHTNESS_MED;
    }

    timerTimeout();

    return QString("Yep");
}

QString Toholed::draw(const QDBusMessage& msg)
{
    int x, y, r, offset, height, width, rowsize;
    // int size;
    bool invert;

    QList<QVariant> args = msg.arguments();

    if (args.count() == 0)
        return QString("Draw fail; missing arguments");

    if (!QString::localeAwareCompare( args.at(0).toString(), "pixel"))
    {
        if (args.count() != 3)
            return QString("Pixel fail; expecting int32:x, int32:y");

        x = args.at(1).toInt();
        y = args.at(2).toInt();

        drawPixel(x, y, 1, screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("Pixel x:%1 y:%2").arg(x).arg(y);
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "circle"))
    {
        if (args.count() != 4)
            return QString("Circle fail; expecting int32:x, int32:y, int32:radius");

        x = args.at(1).toInt();
        y = args.at(2).toInt();
        r = args.at(3).toInt();

        drawCircle(x, y, r, 1, screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("Circle x:%1 y:%2 r:%3").arg(x).arg(y).arg(r);
    }
    else if (!QString::localeAwareCompare( args.at(0).toString(), "bitmap"))
    {
        if (args.count() != 4)
            return QString("Bitmap fail; expecting int32:x, int32:y, array of bytes:data");

        x = args.at(1).toInt();
        y = args.at(2).toInt();

        QByteArray byteArray = args.at(3).toByteArray();
        const char* bitmapData = byteArray.constData();

        if (byteArray.count() < 50)
            return QString("Invalid bitmap - too few bytes");
        /* Checks BM, header size 40, 1 bit/pixel */
        if ( (bitmapData[0] == 0x42) && (bitmapData[1] == 0x4d) && (bitmapData[14] == 0x28) && (bitmapData[28] == 0x01))
        {
            //size   = bitmapData[2]  + (bitmapData[3]<<8)  + (bitmapData[4]<<16)  + (bitmapData[5]<<24);
            offset = bitmapData[10] + (bitmapData[11]<<8) + (bitmapData[12]<<16) + (bitmapData[13]<<24);
            width  = bitmapData[18] + (bitmapData[19]<<8) + (bitmapData[20]<<16) + (bitmapData[21]<<24);
            height = bitmapData[22] + (bitmapData[23]<<8) + (bitmapData[24]<<16) + (bitmapData[25]<<24);
            invert = (bitmapData[54] == 0xff);
            rowsize = 4*((width+31)/32);
            /* printf("Bitmap size %d == %d, offset %d, w:%d h:%d, rowsize %d\n", size, byteArray.count(), offset, width, height, rowsize); */
            if ((height > 64) || (width > 128))
                return QString("Invalid bitmap - too large");
        }
        else
            return QString("Invalid bitmap - Wrong format");

        drawBitmap(x, y, height, width, offset, rowsize, invert, bitmapData, screenBuffer);

        if (oledInitDone)
            updateOled(screenBuffer);

        return QString("Bitmap x:%1 y:%2 len:%3").arg(x).arg(y).arg(byteArray.count());
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

/* New SMS */
void Toholed:: handleSMS(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    printf("New SMS: %s\n", qPrintable(args.at(0).toString()));

    mutex.lock();
    drawIcon(MESSAGE, screenBuffer);
    if (oledInitDone)
    {
        updateOled(screenBuffer);
        blinkOled(10);
    }
    mutex.unlock();

    iconSMS = true;
}

/* Tweetian handler */

void Toholed::handleTweetian(const QDBusMessage& msg)
{
    Q_UNUSED(msg);
    printf("You have been mentioned in a Tweet\n");

    mutex.lock();
    drawIcon(TWEET, screenBuffer);
    if (oledInitDone)
    {
        updateOled(screenBuffer);
        blinkOled(5);
    }
    mutex.unlock();

    iconTWEET = true;
}

/* Communi IRC handler */

void Toholed::handleCommuni(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    int ah = args.at(0).toInt();
    printf("IRC: Active highlights %d\n", ah);

    if (ah > activeHighlights) /* Number of active highlights increased */
    {
        mutex.lock();
        drawIcon(IRC, screenBuffer);
        if (oledInitDone)
        {
            updateOled(screenBuffer);
            blinkOled(5);
        }
        iconIRC = true;
        mutex.unlock();

    }
    else if ((ah == 0) && iconIRC) /* Active highlights all read */
    {
        mutex.lock();
        clearIcon(IRC, screenBuffer);
        if (oledInitDone)
            updateOled(screenBuffer);
        iconIRC = false;
        mutex.unlock();
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
        mutex.lock();
        drawIcon(CALL, screenBuffer);
        if (oledInitDone)
        {
            updateOled(screenBuffer);
            blinkOled(10);
        }
        iconCALL = true;

        mutex.unlock();
    }
    else if ( iconCALL && !(QString::localeAwareCompare( args.at(0).toString(), "active")) )
    {
        printf("Call answered or placing new call when missed call indicated\n");
        mutex.lock();
        clearIcon(CALL, screenBuffer);
        if (oledInitDone)
            updateOled(screenBuffer);

        iconCALL = false;

        mutex.unlock();
    }

}

void Toholed::handleDisplayStatus(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    int fd;
    int tmp;
    unsigned long reg;

    printf("Display status changed to %s\n", qPrintable(args.at(0).toString()));

    if (!(QString::localeAwareCompare( args.at(0).toString(), "on")))
    {
        iphbStop();

        tmp = checkOled();
        if (tmp == -1)
            printf("Failed to check OLED status\n");
        else if (tmp == 1)
            printf("Display is active\n");
        else
            printf("Display is off (%02x)\n", tmp);


        fd = tsl2772_initComms(0x39);
        if (fd <0)
        {
            printf("failed to start communication with TSL2772\n");
            tsl2772_closeComms(fd);
            return;
        }
        reg = tsl2772_getReg(fd, 0x00);
        tsl2772_closeComms(fd);

        if (reg == 0xffff)
            printf("Failed to read register from TSL2772\n");
        else
            printf("TSL2772 enable register = %02x\n", (int)(reg & 0xff));

    }
    else if (!(QString::localeAwareCompare( args.at(0).toString(), "off")))
    {
        timer->start();
        iphbStart();
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

    mutex.lock();
    /* Clear all icons and their status flags */
    clearIcons(screenBuffer);
    if (oledInitDone)
        updateOled(screenBuffer);

    iconSMS = false;
    iconEMAIL = false;
    iconCALL = false;
    iconTWEET = false;
    iconIRC = false;

    mutex.unlock();
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
            timeUpdateOverride = true;
            timerTimeout();
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
        timeUpdateOverride = true;
        timerTimeout();
    }
    else if (!(QString::localeAwareCompare( tmp, "charger_disconnected")) && chargerConnected)
    {
        printf("Charger disconnected\n");
        chargerConnected = false;
        timeUpdateOverride = true;
        timerTimeout();
    }

}


void Toholed::handleMitakuuluu(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    int mkUnread = args.at(0).toInt();
    printf("Mitakuuluu: total unread %d\n", mkUnread);

    if (mkUnread > mitakuuluuUnread) /* Number of total unread increased */
    {
        mutex.lock();
        drawIcon(MITAKUULUU, screenBuffer);
        if (oledInitDone)
        {
            updateOled(screenBuffer);
            blinkOled(2);
        }
        iconMITAKUULUU = true;
        mutex.unlock();

    }
    else if ((mkUnread == 0) && iconMITAKUULUU) /* All read */
    {
        mutex.lock();
        clearIcon(MITAKUULUU, screenBuffer);
        if (oledInitDone)
            updateOled(screenBuffer);
        iconMITAKUULUU = false;
        mutex.unlock();
    }

    mitakuuluuUnread = mkUnread;

}

void Toholed::handleProfileChanged(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    QString tmp = args.at(2).toString();

    printf("Profile changed to %s\n", qPrintable(tmp));

    if (tmp == "silent")
    {
        silentProfile = true;
        timeUpdateOverride = true;
        timerTimeout();
    }
    else
    {
        silentProfile = false;
        timeUpdateOverride = true;
        timerTimeout();
    }
}

QString Toholed::getCurrentProfile()
{
    QDBusInterface getProfileCall("com.nokia.profiled", "/com/nokia/profiled", "com.nokia.profiled");
    getProfileCall.setTimeout(2);

    QDBusMessage getProfileCallReply = getProfileCall.call(QDBus::AutoDetect, "get_profile");

    return getProfileCallReply.arguments().at(0).toString();
}

/* Slots for Notificationsmanager */

void Toholed::handleEmailNotify()
{
    printf("email notification\n");

    mutex.lock();
    drawIcon(MAIL, screenBuffer);
    if (oledInitDone)
    {
        updateOled(screenBuffer);
        blinkOled(3);
    }
    iconEMAIL = true;
    mutex.unlock();

}

void Toholed::handleTwitterNotify()
{
    printf("twitter notification\n");

    mutex.lock();
    drawIcon(TWEET, screenBuffer);
    if (oledInitDone)
    {
        updateOled(screenBuffer);
        blinkOled(3);
    }
    iconTWEET = true;
    mutex.unlock();
}

void Toholed::handleFacebookNotify()
{
    printf("facebook notification.\n");
}

void Toholed::handleIrssiNotify()
{
    printf("irssi notification.\n");

    mutex.lock();
    drawIcon(IRC, screenBuffer);
    if (oledInitDone)
    {
        updateOled(screenBuffer);
        blinkOled(3);
    }
    iconIRC = true;
    mutex.unlock();
}

void Toholed::handleImNotify()
{
    printf("im notification.\n");

    mutex.lock();
    drawIcon(MESSAGE, screenBuffer);
    if (oledInitDone)
    {
        updateOled(screenBuffer);
        blinkOled(2);
    }
    iconSMS = true;
    mutex.unlock();
}

void Toholed::handleOtherNotify()
{
    printf("other notification\n");

    mutex.lock();
    if (oledInitDone)
    {
        blinkOled(1);
    }
    mutex.unlock();
}
