#include <QApplication>
#include "mainwindow.h"
#include "../3rdparty/MersenneTwister.h"
#include "globalsettings.h"
#include "debugtimer.h"
MTRand _mtrand;
MTRand &mtRand()
{
    return _mtrand;
}

// stuff from globalsettings.cpp in order to compile the lightroom
int _loglevel=0;
 // true, if detailed debug information is logged
bool logLevelDebug()
{
    return _loglevel<1;
}

// true, if only important aggreate info is logged
bool logLevelInfo()
{
    return _loglevel<2;
}

// true if only severe warnings/errors are logged.
bool logLevelWarning()
{
    return _loglevel<3;
}
void setLogLevel(int loglevel)
{
    _loglevel=loglevel;
    switch (loglevel) {
    case 0: qDebug() << "Loglevel set to Debug."; break;
    case 1: qDebug() << "Loglevel set to Info."; break;
    case 2: qDebug() << "Loglevel set to Warning."; break;
    case 3: qDebug() << "Loglevel set to Error/Quiet."; break;
    default: qDebug() << "invalid log level" << loglevel; break;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
