#include <QtGlobal>

void daemonize();
void signalHandler(int sig);
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
