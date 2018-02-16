/** \file main.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Main entry point for the EquitWebServer application.
  *
  * \todo
  * - decide on application license.
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#include <memory>
#include <iostream>
#include <cstdlib>

#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QtCore>
#include <QString>

#include "server.h"
#include "configuration.h"
#include "mainwindow.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::WebServerAction);
Q_DECLARE_METATYPE(EquitWebServer::Configuration::ConnectionPolicy);


int main(int argc, char * argv[]) {
	QApplication app(argc, argv);
	app.setOrganizationName("Equit");
	app.setOrganizationDomain("www.equituk.net");
	app.setApplicationName("equitwebserver");
	app.setApplicationDisplayName(QApplication::tr("Équit Web Server"));
	app.setApplicationVersion("0.9.9");

	using namespace EquitWebServer;

	qRegisterMetaType<Configuration::ConnectionPolicy>();
	qRegisterMetaType<Configuration::WebServerAction>();

	EquitWebServer::Configuration opts;
	bool autoStart = false;
	QString arg;

	/* load default options */
	QString configFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).absoluteFilePath("defaultsettings.ewcx");

	if(!opts.load(configFile)) {
		qWarning() << "failed to load user default configuration.";

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX) || defined(Q_OS_SOLARIS) || defined(Q_OS_BSD4) || defined(Q_OS_MACX)
		if(!opts.load("/etc/equitwebserverrc")) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load system default configuration.\n";
		}
		else {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: loaded system default configuration.\n";
		}
#endif
	}
	else {
		qDebug() << "loaded user default configuration.";
	}

	/* update options based on command-line */
	for(int i = 1; i < argc; i++) {
		arg = argv[i];

		if(arg.left(2) == "-a" || arg == "--address") {
			if(arg.size() > 2 && arg != "--address") {
				opts.setListenAddress(arg.right(arg.size() - 2));
			}
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
			if(arg.size() > 2 && arg != "--port") {
				opts.setPort(arg.right(arg.size() - 2).toInt());
			}
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
			if(arg.size() > 2 && arg != "--docroot") {
				opts.setDocumentRoot(arg.right(arg.size() - 2));
			}
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

	opts.setCgiBin("/");
	EquitWebServer::MainWindow mainWindow(std::make_unique<EquitWebServer::Server>(opts));

	if(autoStart) {
		mainWindow.startServer();
	}

	mainWindow.show();
	return app.exec();
}
