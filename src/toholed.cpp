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


    /* path=/CommHistoryModel; interface=com.nokia.commhistory; member=eventsAdded */

    static const QString commhistoryservice = "org.nemomobile.CommHistory";
    static const QString commhistorypath = "/";
    static const QString commhistoryinterface = "com.nokia.commhistory";
    static const QString commhistoryname = "eventsAdded";

    static QDBusConnection commhistoryconn = QDBusConnection::sessionBus();
    commhistoryconn.connect(commhistoryservice, commhistorypath, commhistoryinterface, commhistoryname, &toholed, SLOT(handleEventsAdded(const QDBusMessage&)));

    if(commhistoryconn.isConnected())
        writeToLog("commhistory Connected");
    else
        writeToLog("commhistory Not connected");

    /* ofono MessageManager */

    static const QString ofonoservice = "org.ofono";
    static const QString ofonopath = "/ril_0";
    static const QString ofonointerface = "org.ofono.MessageManager";
    static const QString ofononame = "IncomingMessage";

    static QDBusConnection ofonoconn = QDBusConnection::systemBus();
    ofonoconn.connect(ofonoservice, ofonopath, ofonointerface, ofononame, &toholed, SLOT(handleSMS(const QDBusMessage&)));

    if(ofonoconn.isConnected())
        writeToLog("Ofono Connected");
    else
        writeToLog("Ofono Not connected");

    /* Jolla Lipstick coverstatus */

    static const QString lipstickservice = "com.jolla.lipstick";
    static const QString lipstickpath = "/com/jolla/lipstick";
    static const QString lipstickinterface = "com.jolla.lipstick";
    static const QString lipstickname = "coverstatus";

    static QDBusConnection lipstickconn = QDBusConnection::sessionBus();
    lipstickconn.connect(lipstickservice, lipstickpath, lipstickinterface, lipstickname, &toholed, SLOT(handleCoverStatus(const QDBusMessage&)));

    if(lipstickconn.isConnected())
        writeToLog("Lipstick Connected");
    else
        writeToLog("Lipstick Not connected");

    /* jotain */




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
