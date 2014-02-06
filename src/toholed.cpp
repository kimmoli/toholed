#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "toholed.h"
#include "tca8424.h"
#include "toh.h"
#include "oled.h"
#include "frontled.h"

#include <QtCore/QCoreApplication>
//#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <QDBusMessage>

#include "toholed-dbus.h"


bool Toholed::oledInitDone = false;
bool Toholed::oledAutoUpdate = false;
bool Toholed::vddEnabled = false;
bool Toholed::interruptsEnabled = false;
int Toholed::timerCount = 0;
unsigned int Toholed::prevBrightness = BRIGHTNESS_MED;
unsigned int Toholed::prevProx = 0;
bool Toholed::iconCALL = false;
bool Toholed::iconSMS = false;
bool Toholed::iconEMAIL = false;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    fprintf(stdout, "Starting toholed daemon.\n");
    daemonize();

    writeToLog("toholed daemon started.");


    if (!QDBusConnection::systemBus().isConnected())
    {
        fprintf(stderr, "Cannot connect to the D-Bus systemBus\n");
        writeToLog("Cannot connect to the D-Bus systemBus");
        writeToLog(qPrintable(QDBusConnection::systemBus().lastError().message()));
        exit(EXIT_FAILURE);
    }

    writeToLog(qPrintable(getenv ("DBUS_SESSION_BUS_ADDRESS")));

    if (!QDBusConnection::sessionBus().isConnected())
    {
        fprintf(stderr, "Cannot connect to the D-Bus sessionBus\n");
        writeToLog("Cannot connect to the D-Bus sessionBus");
        writeToLog(qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(EXIT_FAILURE);
    }

    if (!QDBusConnection::systemBus().registerService(SERVICE_NAME))
    {
        fprintf(stderr, "%s\n", qPrintable(QDBusConnection::systemBus().lastError().message()));
        writeToLog("Cannot register service to systemBus");
        writeToLog(qPrintable(QDBusConnection::systemBus().lastError().message()));
        exit(EXIT_FAILURE);
    }

    Toholed toholed;

    QDBusConnection::systemBus().registerObject("/", &toholed, QDBusConnection::ExportAllSlots);



    /* Ofono MessageManager IncomingMessage
     * This signal is emitted when new SMS message arrives */

    static QDBusConnection ofonoSMSconn = QDBusConnection::systemBus();
    ofonoSMSconn.connect("org.ofono", "/ril_0", "org.ofono.MessageManager", "IncomingMessage",
                      &toholed, SLOT(handleSMS(const QDBusMessage&)));

    if(ofonoSMSconn.isConnected())
        writeToLog("Ofono.MessageManager.IncomingMessage Connected");
    else
    {
        writeToLog("Ofono.MessageManager.IncomingMessage Not connected");
        writeToLog(qPrintable(QDBusConnection::systemBus().lastError().message()));
    }

    /* Ofono VoiceCallManager CallAdded
     *
     */
    static QDBusConnection ofonoCallconn = QDBusConnection::systemBus();
    ofonoCallconn.connect("org.ofono", "/ril_0", "org.ofono.VoiceCallManager", "CallAdded",
                      &toholed, SLOT(handleCall(const QDBusMessage&)));

    if(ofonoCallconn.isConnected())
        writeToLog("Ofono.VoiceCallManager.CallAdded Connected");
    else
    {
        writeToLog("Ofono.VoiceCallManager.CallAdded Not connected");
        writeToLog(qPrintable(QDBusConnection::systemBus().lastError().message()));
    }



    /* Nokia MCE display_status_ind
     * No actual use with this, just make log entry. Display status returns string: "on", "dimmed" or "off"  */

    static QDBusConnection mceSignalconn = QDBusConnection::systemBus();
    mceSignalconn.connect("com.nokia.mce", "/com/nokia/mce/signal", "com.nokia.mce.signal", "display_status_ind",
                          &toholed, SLOT(handleDisplayStatus(const QDBusMessage&)));

    if(mceSignalconn.isConnected())
        writeToLog("com.nokia.mce.signal.display_status_ind Connected");
    else
    {
        writeToLog("com.nokia.mce.signal.display_status_ind Not connected");
        writeToLog(qPrintable(QDBusConnection::systemBus().lastError().message()));
    }



    /* Freedesktop Notifications NotificationClosed
     * This signal is emitted when notification is closed. We can then remove icons from screen */

    static QDBusConnection freeNotifconn = QDBusConnection::sessionBus();
    freeNotifconn.connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "NotificationClosed",
                          &toholed, SLOT(handleNotificationClosed(const QDBusMessage&)));

    if(freeNotifconn.isConnected())
        writeToLog("freedesktop.Notifications.NotificationClosed Connected");
    else
    {
        writeToLog("freedesktop.Notifications.NotificationClosed Not connected");
        writeToLog(qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }


    /* TODO
     *
     * Connect to something that indicates new email
     * Connect to something that indicates missed/or incoming call ? show phone number?
     * Connect to somethinf that indicates new IM message
     */



    return app.exec();

}


void writeToLog(const char *buf)
{
	int logFile;
	char ts[20];
    char tmp[1024];
	
	time_t t;
	struct tm *tnow;

	t = time(NULL);
	tnow = localtime(&t);	
	
	strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tnow);
	
	snprintf(tmp, sizeof(tmp), "%s :: %s\r\n", ts, buf);
	
	logFile = open("toholog", O_WRONLY | O_CREAT | O_APPEND, 0666);

	if (logFile != -1)
		write(logFile, tmp, strlen(tmp));

	close(logFile);
}


void daemonize()
{
	/* Change the file mode mask */
	umask(0);

	/* Change the current working directory */
	if ((chdir("/tmp")) < 0) 
		exit(EXIT_FAILURE);

	/* Close out the standard file descriptors */
//	close(STDIN_FILENO);
//	close(STDOUT_FILENO);
//	close(STDERR_FILENO);

	/* register signals to monitor / ignore */
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signalHandler); /* catch hangup signal */
	signal(SIGTERM,signalHandler); /* catch kill signal */
}


void signalHandler(int sig) /* signal handler function */
{
	switch(sig)
	{
		case SIGHUP:
			/* rehash the server */
			writeToLog("Received signal SIGHUP");
			break;		
		case SIGTERM:
			/* finalize the server */
			writeToLog("Received signal SIGTERM");
            deinitOled();
            controlVdd(0);
            controlFrontLed(0, 0, 0);
			exit(0);
			break;		
	}	
}
