/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/

#include <QCoreApplication>
#include <stdio.h>
#include <QDebug>
#include <QDateTime>
#include <QStringList>
#include "version.h"
#include "exception.h"
#include <stdexcept>
#include <QTimer>
#include <QString>

#include "consoleshell.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    printf("iLand console (%s - #%s)\n", currentVersion(), svnRevision());
    printf("This is the console version of iLand, the individual based\nlandscape and disturbance forest model.\n");
    printf("More at: https://iland-model.org \n");
    printf("(c) Werner Rammer, Rupert Seidl, 2009-2020 \n");
    printf("version: %s\n", verboseVersion().toLocal8Bit().data());
    printf("****************************************\n\n");
    if (a.arguments().count()<3) {
        printf("Usage: \n");
        printf("ilandc.exe <xml-project-file> <years> <...other options>\n");
        printf("Options:\n");
        printf("you specify a number key=value pairs, and *after* loading of the project\n");
        printf("the 'key' settings are set to 'value'. E.g.: ilandc project.xml 100 output.stand.enabled=false output.stand.landscape=false\n");
        printf("See also https://iland-model.org/iLand+console\n.");
        return 0;
    }
    ConsoleShell iland_shell;

    QTimer::singleShot(0, &iland_shell, SLOT(run()));
    //a.installEventFilter(&iland_shell);
    return a.exec();
}
