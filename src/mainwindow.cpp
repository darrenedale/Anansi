/// \file MainWindow.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the MainWindow class for EquitWebServer.
///
/// \todo
/// - add "restart now" button to configuration-changed-while-running warning dialogues.
/// - do we need to do anything special in saveConfigurationAsDefault() regarding platform?
/// - decide on application license
///
/// \par Changes
/// - (2012-06) now warns the user if the document root is changed while the server is running
///   that the change will not take effect until the next server restart.
/// - (2012-06) file documentation created.

#include "mainwindow.h"

#include <iostream>

#include <QDebug>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QPoint>
#include <QDir>
#include <QStandardPaths>

#include "configurationwidget.h"
#include "counterlabel.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::ConnectionPolicy);
Q_DECLARE_METATYPE(EquitWebServer::Configuration::WebServerAction);


namespace EquitWebServer {

	static bool iconsInitialised = false;
	static QIcon StartButtonIcon;
	static QIcon StopButtonIcon;
	static QIcon QuitButtonIcon;

#if defined(Q_OS_MACX)

	/* no menu icons on OSX */
	static const QIcon OpenConfigMenuIcon = {};
	static const QIcon & SaveConfigMenuIcon = OpenConfigMenuIcon;
	static const QIcon & ChooseDocRootMenuIcon = OpenConfigMenuIcon;
	static const QIcon & StartServerMenuIcon = OpenConfigMenuIcon;
	static const QIcon & StopServerMenuIcon = OpenConfigMenuIcon;
	static const QIcon & ExitMenuIcon = OpenConfigMenuIcon;
	static const QIcon & AllowUnknownIpsMenuIcon = OpenConfigMenuIcon;
	static const QIcon & ForbidUnknownIpsMenuIcon = OpenConfigMenuIcon;
	static const QIcon & ClearIpListMenuIcon = OpenConfigMenuIcon;
	static const QIcon & AboutMenuIcon = OpenConfigMenuIcon;
	static const QIcon & AboutQtMenuIcon = OpenConfigMenuIcon;

	static inline void staticInitialise() {
		qRegisterMetaType<Configuration::ConnectionPolicy>();
		qRegisterMetaType<Configuration::WebServerAction>();
		StartButtonIcon = QIcon::fromTheme("media-playback-start", QIcon(":/icons/buttons/startserver"));
		StopButtonIcon = QIcon::fromTheme("media-playback-stop", QIcon(":/icons/buttons/stopserver"));
		QuitButtonIcon = QIcon::fromTheme("application-exit", QIcon(":/icons/buttons/exit"));
		iconsInitialised = true;
	}

#else

	static QIcon OpenConfigMenuIcon;
	static QIcon SaveConfigMenuIcon;
	static QIcon ChooseDocRootMenuIcon;
	static QIcon StartServerMenuIcon;
	static QIcon StopServerMenuIcon;
	static QIcon ExitMenuIcon;
	static QIcon AllowUnknownIpsMenuIcon;
	static QIcon ForbidUnknownIpsMenuIcon;
	static QIcon ClearIpListMenuIcon;
	static QIcon AboutMenuIcon;
	static QIcon AboutQtMenuIcon;


	static void staticInitialise() {
		qRegisterMetaType<Configuration::ConnectionPolicy>();
		qRegisterMetaType<Configuration::WebServerAction>();
		StartButtonIcon = QIcon::fromTheme("media-playback-start", QIcon(":/icons/buttons/startserver"));
		StopButtonIcon = QIcon::fromTheme("media-playback-stop", QIcon(":/icons/buttons/stopserver"));
		QuitButtonIcon = QIcon::fromTheme("application-exit", QIcon(":/icons/buttons/exit"));

		OpenConfigMenuIcon = QIcon::fromTheme("document-open", QIcon(":/icons/menu/openconfig"));
		SaveConfigMenuIcon = QIcon::fromTheme("document-save", QIcon(":/icons/menu/saveconfig"));
		ChooseDocRootMenuIcon = QIcon::fromTheme("document-open-folder", QIcon(":/icons/menu/choosedocumentroot"));
		StartServerMenuIcon = QIcon::fromTheme("media-playback-start", QIcon(":/icons/menu/startserver"));
		StopServerMenuIcon = QIcon::fromTheme("media-playback-stop", QIcon(":/icons/menu/stopserver"));
		ExitMenuIcon = QIcon::fromTheme("application-exit", QIcon(":/icons/menu/exit"));
		AllowUnknownIpsMenuIcon = QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/connectionpolicies/accept"));
		ForbidUnknownIpsMenuIcon = QIcon::fromTheme("dialog-cancel", QIcon(":/icons/connectionpolicies/reject"));
		ClearIpListMenuIcon = QIcon::fromTheme("edit-clear-list", QIcon(":/icons/menus/clearipaccesslist"));
		AboutMenuIcon = QIcon::fromTheme("help-about", QIcon(":/icons/menu/about"));
		AboutQtMenuIcon = QIcon(":/icons/menu/aboutqt");
		iconsInitialised = true;
	}

#endif


	MainWindow::MainWindow(std::unique_ptr<Server> server, QWidget * parent)
	: QMainWindow(parent),
	  m_server(std::move(server)),
	  m_statusBar(new QStatusBar),
	  m_menuBar(new QMenuBar),
	  m_serverMenu(new QMenu(tr("&Server"))),
	  m_accessMenu(new QMenu(tr("Access"))),
	  m_contentMenu(new QMenu(tr("Content"))),
	  m_recentConfigsMenu(new QMenu(tr("Recent Configurations"))),
	  m_controller(new ConfigurationWidget(m_server.get())),
	  m_requestReceivedCountLabel(new CounterLabel(tr("Requests Received: %1"))),
	  m_requestAcceptedCountLabel(new CounterLabel(tr("Requests Accepted: %1"))),
	  m_requestRejectedCountLabel(new CounterLabel(tr("Requests Rejected: %1"))),
	  m_requestReceivedCount(0),
	  m_requestAcceptedCount(0),
	  m_requestRejectedCount(0),
	  m_startStopServer(new QPushButton(tr("Start"))) {
		if(!iconsInitialised) {
			staticInitialise();
		}

		m_startStopServer->setIcon(StartButtonIcon);

		/* generic, re-usable widget ptrs */
		QLabel * myLabel;

		/* main container and layout */
		QWidget * centralWidget = new QWidget;
		QVBoxLayout * vLayout = new QVBoxLayout;

		/* logo and controller */
		QHBoxLayout * hLayout = new QHBoxLayout;
		hLayout->addWidget(myLabel = new QLabel, 0, Qt::AlignTop);
		myLabel->setPixmap(QPixmap(":/pixmaps/applogo"));

		hLayout->addWidget(m_controller);
		vLayout->addLayout(hLayout);

		/* button box */
		QDialogButtonBox * bBox = new QDialogButtonBox;
		bBox->addButton(m_startStopServer, QDialogButtonBox::AcceptRole);
		connect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::startServer);
		QPushButton * myButton = new QPushButton(QuitButtonIcon, tr("Quit"));
		bBox->addButton(myButton, QDialogButtonBox::RejectRole);
		connect(myButton, &QPushButton::clicked, this, &MainWindow::close);
		vLayout->addWidget(bBox);

		centralWidget->setLayout(vLayout);
		setCentralWidget(centralWidget);

		/* status bar */
		m_statusBar->addPermanentWidget(m_requestReceivedCountLabel);
		m_statusBar->addPermanentWidget(m_requestAcceptedCountLabel);
		m_statusBar->addPermanentWidget(m_requestRejectedCountLabel);

		/* menu bar */
		QMenu * helpMenu = new QMenu(tr("Help"));

		m_menuBar->addMenu(m_serverMenu);
		m_menuBar->addMenu(m_accessMenu);
		m_menuBar->addMenu(m_contentMenu);
		m_menuBar->addMenu(helpMenu);

		m_serverMenu->addAction(OpenConfigMenuIcon, tr("&Open Configuration..."), this, qOverload<>(&MainWindow::loadConfiguration));
		m_serverMenu->addAction(SaveConfigMenuIcon, tr("&Save Configuration..."), this, &MainWindow::saveConfiguration);
		m_serverMenu->addAction(tr("Save &Default Configuration..."), this, &MainWindow::saveConfigurationAsDefault);
		m_serverMenu->addSeparator();
		m_serverMenu->addMenu(m_recentConfigsMenu);
		m_serverMenu->addSeparator();
		m_serverMenu->addAction(ChooseDocRootMenuIcon, tr("Document Root..."), m_controller, &ConfigurationWidget::chooseDocumentRoot);
		m_serverMenu->addAction(tr("Listen on localhost"), m_controller, &ConfigurationWidget::bindToLocalhost);
		m_serverMenu->addAction(tr("Listen on host address"), m_controller, &ConfigurationWidget::bindToHostAddress);
		m_serverMenu->addSeparator();
		m_serverMenu->addAction(StartServerMenuIcon, tr("Start"), this, &MainWindow::startServer);
		m_serverMenu->addAction(StopServerMenuIcon, tr("Stop"), this, &MainWindow::stopServer);
		m_serverMenu->addSeparator();
		m_serverMenu->addAction(ExitMenuIcon, tr("&Quit"), this, &MainWindow::close);

		m_accessMenu->addAction(AllowUnknownIpsMenuIcon, tr("Allow Unknown IPs"), m_controller, &ConfigurationWidget::setLiberalDefaultConnectionPolicy);
		m_accessMenu->addAction(ForbidUnknownIpsMenuIcon, tr("Forbid Unknown IPs"), m_controller, &ConfigurationWidget::setRestrictiveDefaultConnectionPolicy);
		m_accessMenu->addAction(ClearIpListMenuIcon, tr("Clear IP Access List"), m_controller, &ConfigurationWidget::clearIpConnectionPolicies);

		m_contentMenu->addAction(tr("Clear all MIME type associations"), m_controller, &ConfigurationWidget::clearAllFileExtensionMIMETypes);
		m_contentMenu->addAction(tr("Clear all actions"), m_controller, &ConfigurationWidget::clearAllActions);

		helpMenu->addAction(AboutMenuIcon, tr("About"), this, &MainWindow::about);
		helpMenu->addAction(AboutQtMenuIcon, tr("About Qt"), qApp, &QApplication::aboutQt);

		setMenuBar(m_menuBar);
		setStatusBar(m_statusBar);

		setWindowTitle(qApp->applicationDisplayName());
		setWindowIcon(QIcon(":/pixmaps/applogo"));

		connect(m_server.get(), &Server::connectionReceived, m_requestReceivedCountLabel, qOverload<>(&CounterLabel::increment));
		connect(m_server.get(), &Server::connectionRejected, m_requestRejectedCountLabel, qOverload<>(&CounterLabel::increment));
		connect(m_server.get(), &Server::connectionAccepted, m_requestAcceptedCountLabel, qOverload<>(&CounterLabel::increment));
		connect(m_controller, &ConfigurationWidget::documentRootChanged, this, &MainWindow::slotDocumentRootChanged);
		readRecentConfigs();
	}


	MainWindow::~MainWindow() {
		if(m_controller) {
			m_controller->setServer(nullptr);
		}

		saveRecentConfigs();
	}


	void MainWindow::ensureUserConfigDir() {
		QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

		if(!configDir.exists()) {
			QDir("/").mkpath(configDir.absolutePath());
		}
	}


	void MainWindow::readRecentConfigs() {
		m_recentConfigs.clear();
		m_recentConfigsMenu->clear();

		ensureUserConfigDir();
		QFile recentConfigsFile(QStandardPaths::locate(QStandardPaths::AppConfigLocation, "recentconfigs"));

		if(!recentConfigsFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to open recent configs file\n";
			return;
		}

		QString line;

		while(!recentConfigsFile.atEnd()) {
			line = QString::fromUtf8(recentConfigsFile.readLine().trimmed());

			if(line.isEmpty()) {
				continue;
			}

			m_recentConfigs.push_back(line);
			QAction * action = m_recentConfigsMenu->addAction(line, this, &MainWindow::loadRecentConfiguration);
			action->setData(line);
			action->setCheckable(true);
		}
	}


	void MainWindow::saveRecentConfigs() {
		QFile recentConfigsFile(QStandardPaths::locate(QStandardPaths::AppConfigLocation, "recentconfigs"));

		if(!recentConfigsFile.open(QIODevice::WriteOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to update recent configs file (couldn't open \"" << qPrintable(recentConfigsFile.fileName()) << "\" for writing)\n";
			return;
		}

		for(const auto & configFileName : m_recentConfigs) {
			recentConfigsFile.write(configFileName.toUtf8());
			recentConfigsFile.putChar('\n');
		}

		recentConfigsFile.close();
	}


	void MainWindow::loadRecentConfiguration() {
		QAction * action = qobject_cast<QAction *>(sender());

		if(!action) {
			return;
		}

		loadConfiguration(action->data().toString());
		QMenu * menu = action->menu();

		if(menu) {
			for(auto * menuAction : menu->actions()) {
				menuAction->setChecked(false);
			}

			action->setChecked(true);
		}
	}


	void MainWindow::slotDocumentRootChanged() {
		if(m_server->isListening()) {
			QMessageBox::warning(this, tr("Set document root"), tr("The document root was changed and the server is currently running. This means that the actual document root being used to serve content will not be altered until the server is restarted. Content will continue to be served from the document root that was set when the server was last started."));
		}
	}


	void MainWindow::saveConfiguration() {
		static QString lastFileName;
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save Webserver Configuration"), lastFileName, "bpWebServer Configuration Files (*.ewcx)");

		if(fileName.isEmpty()) {
			return;
		}

		if(!QFile::exists(fileName) || QMessageBox::Yes == QMessageBox::question(this, tr("Save Webserver Configuration"), "The file already exists. Are you sure you wish to overwrite it with the webserver configuration?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
			if(!m_server->configuration().save(fileName)) {
				QMessageBox::warning(this, tr("Save Webserver Configuration"), tr("Could not save the configuration."));
			}
		}
	}


	void MainWindow::saveConfigurationAsDefault() {
		QString configFilePath = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "defaultsettings.ewcx");
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: attempting to save to \"" << qPrintable(configFilePath) << "\"\n";

		if(!m_server->configuration().save(configFilePath)) {
			QMessageBox dilaogue(QMessageBox::Warning, tr("Save Webserver Configuration"), tr("The current configuration could not be saved as the default configuration."), QMessageBox::Ok, this);
			dilaogue.setDetailedText(tr("It was not possible to write to the file \"%1\".").arg(configFilePath));
			dilaogue.exec();
			return;
		}
		else {
			// TODO use notifications
			QMessageBox::information(this, tr("Save Webserver Configuration"), tr("The current configuration was saved as the default."));
		}
	}


	void MainWindow::loadConfiguration() {
		QString fileName = QFileDialog::getOpenFileName(this, tr("Load Webserver Configuration"), QString(), "bpWebServer Configuration Files (*.ewcx)");

		if(fileName.isEmpty()) {
			return;
		}

		loadConfiguration(fileName);
	}


	void MainWindow::loadConfiguration(const QString & fileName) {
		Configuration newConfig;

		if(newConfig.load(fileName)) {
			for(auto * action : m_recentConfigsMenu->actions()) {
				action->setChecked(false);
			}

			if(const auto & end = m_recentConfigs.cend(); end == std::find(m_recentConfigs.cbegin(), end, fileName)) {
				m_recentConfigs.push_back(fileName);
				QAction * newAction = m_recentConfigsMenu->addAction(fileName, this, &MainWindow::loadRecentConfiguration);
				newAction->setData(fileName);
				newAction->setCheckable(true);
				newAction->setChecked(true);
			}
			else {
				/* if the file name matches a recent config, tick it */
				for(auto * action : m_recentConfigsMenu->actions()) {
					if(action->data().toString() == fileName) {
						action->setChecked(true);
					}
				}
			}

			m_server->setConfiguration(newConfig);
			m_controller->readConfiguration();
		}
		else {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load the configuration\n";
			QMessageBox::warning(this, tr("Load Webserver Configuration"), "The configuration could not be loaded.");
		}
	}


	bool MainWindow::startServer() {
		if(m_server->isListening()) {
			return true;
		}

		// TODO custom button class for start/stop to remove repeated code
		if(m_server->listen()) {
			statusBar()->showMessage(tr("The server is listening on %1:%2.").arg(m_server->configuration().listenAddress()).arg(m_server->configuration().port()));
			m_controller->disableWidgets();
			disconnect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::startServer);
			connect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::stopServer, Qt::UniqueConnection);
			m_startStopServer->setIcon(StopButtonIcon);
			m_startStopServer->setText(tr("Stop"));
		}
		else {
			statusBar()->showMessage(tr("The server could not be started."));
			m_controller->enableWidgets();
			disconnect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::stopServer);
			connect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::startServer, Qt::UniqueConnection);
			m_startStopServer->setIcon(StartButtonIcon);
			m_startStopServer->setText(tr("Start"));
		}

		return false;
	}


	bool MainWindow::stopServer() {
		if(!m_server->isListening()) {
			return true;
		}

		m_server->close();

		if(m_server->isListening()) {
			statusBar()->showMessage(tr("The server could not be stopped. The server is listening on %1:%2.").arg(m_server->configuration().listenAddress()).arg(m_server->configuration().port()));
			m_controller->disableWidgets();
			disconnect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::startServer);
			connect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::stopServer, Qt::UniqueConnection);
			m_startStopServer->setIcon(StopButtonIcon);
			m_startStopServer->setText(tr("Stop"));
		}
		else {
			statusBar()->showMessage(tr("The server currently offline."));
			m_controller->enableWidgets();
			disconnect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::stopServer);
			connect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::startServer, Qt::UniqueConnection);
			m_startStopServer->setIcon(StartButtonIcon);
			m_startStopServer->setText(tr("Start"));
		}

		return !m_server->isListening();
	}


	void MainWindow::about() {
		QMessageBox::about(this, tr("About %1").arg(qApp->applicationDisplayName()), QString("<p><big><strong>") + qApp->applicationDisplayName() + " v" + qApp->applicationVersion() + "</strong></big></p><p style=\"font-weight: normal;\"><small>A simple web server for desktop use.</small></p><p style=\"font-weight: normal;\"><small>Written by Darren Edale for <strong>&Eacute;quit</strong> (<a href=\"http://www.equituk.net\">http://www.equituk.net/</a>)</small></p><p style=\"font-weight: normal;\"><small>This program is intended for short-term use on the desktop. <strong>It is not a production-strength webserver and should not be used as one.</strong></small></p><p style=\"font-weight: normal;\"><small>" + qApp->applicationDisplayName() + " uses the Qt toolkit (<a href=\"http://www.qt.io/\">http://www.qt.io/</a>).</small></p><p style=\"font-weight: normal;\"><small>" + qApp->applicationDisplayName() + " uses some icons from the KDE <a href=\"https://github.com/KDE/oxygen-icons/\">Oxygen</a> icon project, licensed under the <a href=\"http://www.gnu.org/licenses/lgpl-3.0.txt\">LGPLv3</a>.</small></p>");
	}


	//	void MainWindow::onServerStarted() {
	//	}


	//	void MainWindow::onServerStopped() {
	//	}


	void MainWindow::incrementRequestReceivedCount() {
		setRequestReceivedCount(m_requestReceivedCount + 1);
	}


	void MainWindow::incrementRequestAcceptedCount() {
		setRequestAcceptedCount(m_requestAcceptedCount + 1);
	}


	void MainWindow::incrementRequestRejectedCount() {
		setRequestRejectedCount(m_requestRejectedCount + 1);
	}


	void MainWindow::setRequestReceivedCount(int c) {
		m_requestReceivedCount = c;
		m_requestReceivedCountLabel->setText(tr("Requests Received: %1").arg(m_requestReceivedCount));
	}


	void MainWindow::setRequestAcceptedCount(int c) {
		m_requestAcceptedCount = c;
		m_requestAcceptedCountLabel->setText(tr("Requests Accepted: %1").arg(m_requestAcceptedCount));
	}


	void MainWindow::setRequestRejectedCount(int c) {
		m_requestRejectedCount = c;
		m_requestRejectedCountLabel->setText(tr("Requests Rejected: %1").arg(m_requestRejectedCount));
	}


	void MainWindow::resetRequestReceivedCount() {
		setRequestReceivedCount(0);
	}


	void MainWindow::resetRequestAcceptedCount() {
		setRequestAcceptedCount(0);
	}


	void MainWindow::resetRequestRejectedCount() {
		setRequestRejectedCount(0);
	}


	void MainWindow::resetAllRequestCounts() {
		resetRequestReceivedCount();
		resetRequestAcceptedCount();
		resetRequestRejectedCount();
	}


	void MainWindow::setStatusMessage(const QString & msg) {
		m_statusBar->showMessage(msg);
	}


}  // namespace EquitWebServer
