/** \file main.cpp
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Main entry point for the EquitWebServer application.
  *
  * \todo
  * - decide on application license.
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  *
  */

#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QtCore>
#include <QString>

#include <stdlib.h>

#include "Server.h"
#include "Configuration.h"
#include "MainWindow.h"


int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	app.setApplicationName(QString::fromUtf8("Équit Web Server"));
	app.setApplicationVersion("0.9.9");
	EquitWebServer::Configuration opts;
	bool autoStart = false;
	QString arg;
	
	/* load default options */
	QDir homeDir(QDir::home());

	if(homeDir == QDir::root() || !homeDir.exists() || !opts.load(homeDir.absoluteFilePath(".equit/WebServerDefaults.ewcx"))) {
		qWarning() << "failed to load user default configuration.";

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX) || defined(Q_OS_SOLARIS) || defined(Q_OS_BSD4) || defined(Q_OS_MACX)
		if(!opts.load("/etc/equitwebserverrc")) qWarning() << "failed to load system default configuration.";
		else qWarning() << "loaded system default configuration.";
#endif
	}
	else
		qDebug() << "loaded user default configuration.";

	/* update options based on command-line */
	for(int i = 1; i < argc; i++) {
		arg = argv[i];

		if(arg.left(2) == "-a" || arg == "--address") {
			if(arg.size() > 2 && arg != "--address")
				opts.setListenAddress(arg.right(arg.size() - 2));
			else {
				if((++i) < argc) {
					opts.setListenAddress(argv[i]);
				}
				else {
					qWarning() << arg << " provided without a listen ip address.";
					exit(EXIT_FAILURE);
				}
			}
		}
		else if(arg.left(2) == "-p" || arg == "--port") {
			if(arg.size() > 2 && arg != "--port")
				opts.setPort(arg.right(arg.size() - 2).toInt());
			else {
				if((++i) < argc) {
					opts.setPort(QString(argv[i]).toInt());
				}
				else {
					qWarning() << arg << " provided without a listen port.";
					exit(EXIT_FAILURE);
				}
			}
		}
		else if(arg.left(2) == "-d" || arg == "--docroot") {
			if(arg.size() > 2 && arg != "--docroot")
				opts.setDocumentRoot(arg.right(arg.size() - 2));
			else {
				if((++i) < argc) {
					opts.setDocumentRoot(argv[i]);
				}
				else {
					qWarning() << arg << " provided without a document root.";
					exit(EXIT_FAILURE);
				}
			}
		}
		else if(arg.left(2) == "-s" || arg == "--start") {
			autoStart = true;
		}
	}
	
	opts.setCGIBin("/");
	EquitWebServer::MainWindow mainWindow(new EquitWebServer::Server(opts));
	if(autoStart) mainWindow.startServer();
	mainWindow.show();
	return app.exec();
}
