#include "application.h"

#include <iostream>

#include <QDir>
#include <QStandardPaths>

#include "configuration.h"
#include "server.h"
#include "mainwindow.h"

namespace EquitWebServer {


	Application::Application(int & argc, char ** argv)
	: QApplication(argc, argv),
	  m_mainWindow(nullptr) {
		setOrganizationName(QStringLiteral("Equit"));
		setOrganizationDomain(QStringLiteral("www.equituk.net"));
		setApplicationName(QStringLiteral("equitwebserver"));
		setApplicationDisplayName(QApplication::tr("Équit Web Server"));
		setApplicationVersion("0.9.9");
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
		m_mainWindow = std::make_unique<MainWindow>(std::make_unique<Server>(*opts));

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


}  // namespace EquitWebServer
