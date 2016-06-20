//////////////////////////////////////////////////////////////////////////
//                                                                      //
// EngineMonitor, a graphical gauge to monitor an aircraft's engine     //
// Copyright (C) 2012 Tobias Rad                                        //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program. If not, see <http://www.gnu.org/licenses/>. //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <QtWidgets>
#include "enginemonitor.h"
#include "rdacconnect.h"
#include "nmeaconnect.h"

void messageToFileHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
	QFile debugfile("EngineMon.log");
	if(debugfile.open(QIODevice::Append | QIODevice::Text))
	{
		QString debugString = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz").append(' ');
		switch (type)
		{
		case QtDebugMsg:
			debugString.append("Debug: ");
			break;
		case QtWarningMsg:
			debugString.append("Warning: ");
			break;
		case QtCriticalMsg:
			debugString.append("Critical: ");
			break;
		case QtFatalMsg:
			debugString.append("Fatal: ");
			abort();
		}
		debugfile.write(debugString.append(msg).replace('\n', ", ").append('\n').toLatin1());
		debugfile.close();
	}
	else
	{
		QMessageBox::warning(NULL, "No debug output", "Unable to open 'EngineMon.log', therefore no debug output available.");
	}
}

class SplashScreenDelay : public QThread
{
public:
	static void sleep(unsigned long secs)
	{
		QThread::sleep(secs);
	}
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef QT_NO_DEBUG
	QFile debugfile("EngineMon.log");
	if(debugfile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		debugfile.write(QString("EngineMonitor started at: ").append(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")).append('\n').toLatin1());
		debugfile.close();
		qInstallMessageHandler(messageToFileHandler);
	}
	else
	{
		QMessageBox::warning(NULL, "No debug output", "Unable to open 'EngineMon.log', therefore no debug output available.");
	}
#endif

	//Create splashscreen and show it
	QPixmap pixmap(":/splashscreen.png");
	QSplashScreen splash(pixmap);
	splash.show();
	a.processEvents();

	//Create the engine monitor and show after splashscreen delay
	EngineMonitor engineMonitor;
#ifndef QT_DEBUG
	SplashScreenDelay::sleep(5);
	engineMonitor.showFullScreen();
#else
	engineMonitor.show();
	engineMonitor.move(0, 0);
	engineMonitor.resize(800, 480);
#endif
	splash.finish(&engineMonitor);

	//Create the RDAC connector
	RDACconnect rdacConnect;
	a.connect(&rdacConnect, SIGNAL(updateDataMessage1(double, double)), &engineMonitor, SLOT(setDataMessage1(double, double)));
	a.connect(&rdacConnect, SIGNAL(updateDataMessage2(double,double,double,double,double,double,double)), &engineMonitor, SLOT(setDataMessage2(double,double,double,double,double,double,double)));
	a.connect(&rdacConnect, SIGNAL(updateDataMessage3(double)), &engineMonitor, SLOT(setDataMessage3(double)));
	a.connect(&rdacConnect, SIGNAL(updateDataMessage4egt(quint16,quint16,quint16,quint16)), &engineMonitor, SLOT(setDataMessage4egt(quint16,quint16,quint16,quint16)));
	a.connect(&rdacConnect, SIGNAL(updateDataMessage4cht(quint16,quint16,quint16,quint16)), &engineMonitor, SLOT(setDataMessage4cht(quint16,quint16,quint16,quint16)));
	a.connect(&rdacConnect, SIGNAL(userMessage(QString,QString,bool)), &engineMonitor, SLOT(userMessageHandler(QString,QString,bool)));
	a.connect(&rdacConnect, SIGNAL(statusMessage(QString,QColor)), &engineMonitor, SLOT(showStatusMessage(QString,QColor)));
#ifndef QT_DEBUG
	rdacConnect.start();
#endif

	NMEAconnect nmeaConnect;
	a.connect(&nmeaConnect, SIGNAL(userMessage(QString,QString,bool)), &engineMonitor, SLOT(userMessageHandler(QString,QString,bool)));
	a.connect(&nmeaConnect, SIGNAL(newTimeToDestination(double)), &engineMonitor, SLOT(setTimeToDestination(double)));
#ifndef QT_DEBUG
	nmeaConnect.start();
#endif

	return a.exec();
}
