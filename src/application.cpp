/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Anansi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Anansi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file application.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the Application class.
///
/// \dep
/// - application.h
/// - <iostream>
/// - <QDir>
/// - <QStandardPaths>
/// - <QString>
/// - configuration.h
/// - server.h
/// - mainwindow.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "application.h"

#include <iostream>

#include <QDir>
#include <QStandardPaths>
#include <QString>

#include "configuration.h"
#include "server.h"
#include "mainwindow.h"
#include "qtmetatypes.h"


namespace Anansi {


	Application::Application(int & argc, char ** argv)
	: QApplication(argc, argv),
	  m_mainWindow(nullptr) {
		setOrganizationName(QStringLiteral("Equit"));
		setOrganizationDomain(QStringLiteral("www.equituk.net"));
		setApplicationName(QStringLiteral("anansi"));
		setApplicationDisplayName(QApplication::tr("Anansi"));
		setApplicationVersion("1.0.0");

		// enable these types to be used in queued connections (e.g.
		// between threads)
		qRegisterMetaType<Anansi::ConnectionPolicy>();
		qRegisterMetaType<Anansi::WebServerAction>();

		bool autoStart = false;
		QString arg;

		/* load default options */
		QString configFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).absoluteFilePath("defaultsettings.awcx");
		auto config = Configuration::loadFrom(configFile);

		if(!config) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load user default configuration from \"" << qPrintable(configFile) << ".\n";
			configFile = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).absoluteFilePath("equitwebserversettings.awcx");
			config = Configuration::loadFrom(configFile);

			if(!config) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load system default configuration from \"" << qPrintable(configFile) << "\".\n";
				config = std::make_optional<Configuration>();
			}
		}

		/* update options based on command-line */
		for(int i = 1; i < argc; i++) {
			arg = argv[i];

			if(arg.left(2) == "-a" || arg == "--address") {
				if(arg.size() > 2 && arg != "--address") {
					config->setListenAddress(arg.right(arg.size() - 2));
				}
				else {
					if((++i) < argc) {
						config->setListenAddress(argv[i]);
					}
					else {
						std::cerr << qPrintable(arg) << " provided without a listen ip address.\n";
						exit(EXIT_FAILURE);
					}
				}
			}
			else if(arg.left(2) == "-p" || arg == "--port") {
				if(arg.size() > 2 && arg != "--port") {
					config->setPort(arg.right(arg.size() - 2).toInt());
				}
				else {
					if((++i) < argc) {
						config->setPort(QString(argv[i]).toInt());
					}
					else {
						std::cerr << qPrintable(arg) << " provided without a listen port.";
						exit(EXIT_FAILURE);
					}
				}
			}
			else if(arg.left(2) == "-d" || arg == "--docroot") {
				if(arg.size() > 2 && arg != "--docroot") {
					config->setDocumentRoot(arg.right(arg.size() - 2));
				}
				else {
					if((++i) < argc) {
						config->setDocumentRoot(argv[i]);
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

		m_mainWindow = std::make_unique<MainWindow>(std::make_unique<Server>(std::move(*config)));

		if(autoStart) {
			m_mainWindow->startServer();
		}

		m_mainWindow->show();
	}


	Application::~Application() = default;


	void Application::ensureUserConfigDir() {
		QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

		if(!configDir.exists()) {
			QDir("/").mkpath(configDir.absolutePath());
		}
	}


}  // namespace Anansi
