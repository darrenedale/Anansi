/// \file main.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February 2018
///
/// \brief Main entry point for the EquitWebServer application.
///
/// \todo decide on application license.
///
/// \par Changes
/// - (2012-06-19) file documentation created.

#include <memory>
#include <iostream>
#include <cstdlib>

#include <QApplication>
#include <QString>
#include <QStandardPaths>
#include <QDir>

#include "server.h"
#include "configuration.h"
#include "mainwindow.h"


using EquitWebServer::Configuration;
using EquitWebServer::MainWindow;
using EquitWebServer::Server;


int main(int argc, char * argv[]) {
	QApplication app(argc, argv);
	app.setOrganizationName(QStringLiteral("Equit"));
	app.setOrganizationDomain(QStringLiteral("www.equituk.net"));
	app.setApplicationName(QStringLiteral("equitwebserver"));
	app.setApplicationDisplayName(QApplication::tr("Équit Web Server"));
	app.setApplicationVersion("0.9.9");

	bool autoStart = false;
	QString arg;

	/* load default options */
	QString configFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).absoluteFilePath("defaultsettings.ewcx");
	auto opts = Configuration::loadFrom(configFile);

	if(!opts) {
		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load user default configuration from \"" << qPrintable(configFile) << ".\n";
		configFile = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).absoluteFilePath("equitwebserversettings.ewcx");
		opts = Configuration::loadFrom(configFile);

		if(!opts) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load system default configuration from \"" << qPrintable(configFile) << "\".\n";
			opts = std::make_optional<Configuration>();
		}
	}

	/* update options based on command-line */
	for(int i = 1; i < argc; i++) {
		arg = argv[i];

		if(arg.left(2) == "-a" || arg == "--address") {
			if(arg.size() > 2 && arg != "--address") {
				opts->setListenAddress(arg.right(arg.size() - 2));
			}
			else {
				if((++i) < argc) {
					opts->setListenAddress(argv[i]);
				}
				else {
					std::cerr << qPrintable(arg) << " provided without a listen ip address.\n";
					exit(EXIT_FAILURE);
				}
			}
		}
		else if(arg.left(2) == "-p" || arg == "--port") {
			if(arg.size() > 2 && arg != "--port") {
				opts->setPort(arg.right(arg.size() - 2).toInt());
			}
			else {
				if((++i) < argc) {
					opts->setPort(QString(argv[i]).toInt());
				}
				else {
					std::cerr << qPrintable(arg) << " provided without a listen port.";
					exit(EXIT_FAILURE);
				}
			}
		}
		else if(arg.left(2) == "-d" || arg == "--docroot") {
			if(arg.size() > 2 && arg != "--docroot") {
				opts->setDocumentRoot(arg.right(arg.size() - 2));
			}
			else {
				if((++i) < argc) {
					opts->setDocumentRoot(argv[i]);
				}
				else {
					std::cerr << qPrintable(arg) << " provided without a document root.";
					exit(EXIT_FAILURE);
				}
			}
		}
		else if(arg.left(2) == "-s" || arg == "--start") {
			autoStart = true;
		}
	}

	opts->setCgiBin("/");
	MainWindow mainWindow(std::make_unique<Server>(*opts));

	if(autoStart) {
		mainWindow.startServer();
	}

	mainWindow.show();
	return app.exec();
}
