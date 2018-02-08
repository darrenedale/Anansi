/** \file MainWindow.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the MainWindow class for EquitWebServer
  *
  * \todo
  * - add "restart now" button to configuration-changed-while-running warning
  *   dialogues.
  * - do we need to do anything special in saveConfigurationAsDefault()
  *   regarding platform?
  * - decide on application license
  *
  * \par Changes
  * - (2012-06-21) now warns the user if the document root is changed while the
  *   server is running that the change will not take effect until the next server
  *   restart.
  * - (2012-06-19) file documentation created.
  */

#include "MainWindow.h"

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

#include "ConfigurationWidget.h"
#include "ConnectionCountLabel.h"

#if defined(Q_OS_MACX)
/* no menu icons on OSX */
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_OPENCONFIG_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_SAVECONFIG_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_CHOOSEDOCROOT_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STARTSERVER_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STOPSERVER_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_EXIT_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_ALLOWUNKNOWNIPS_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_FORBIDUNKNOWNIPS_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_CLEARIPLIST_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUT_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUTQT_ICON QIcon()

#define EQITWEBSERVER_MAINWINDOW_BUTTON_START_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_BUTTON_STOP_ICON QIcon()
#define EQITWEBSERVER_MAINWINDOW_BUTTON_QUIT_ICON QIcon()

#else

#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_OPENCONFIG_ICON QIcon::fromTheme("document-open", QIcon(":/icons/menu/openconfig"))
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_SAVECONFIG_ICON QIcon::fromTheme("document-save", QIcon(":/icons/menu/saveconfig"))
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_CHOOSEDOCROOT_ICON QIcon::fromTheme("document-open-folder", QIcon(":/icons/menu/choosedocumentroot"))
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STARTSERVER_ICON QIcon::fromTheme("media-playback-start", QIcon(":/icons/menu/startserver"))
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STOPSERVER_ICON QIcon::fromTheme("media-playback-stop", QIcon(":/icons/menu/stopserver"))
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_EXIT_ICON QIcon::fromTheme("application-exit", QIcon(":/icons/menu/exit"))
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_ALLOWUNKNOWNIPS_ICON QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/connectionpolicies/accept"))
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_FORBIDUNKNOWNIPS_ICON QIcon::fromTheme("dialog-cancel", QIcon(":/icons/connectionpolicies/reject"))
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_CLEARIPLIST_ICON QIcon::fromTheme("edit-clear-list", QIcon(":/icons/menus/clearipaccesslist"))
#define EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUT_ICON QIcon::fromTheme("help-about", QIcon(":/icons/menu/about"))
#define EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUTQT_ICON QIcon(":/icons/menu/aboutqt")

#define EQITWEBSERVER_MAINWINDOW_BUTTON_START_ICON QIcon::fromTheme("media-playback-start", QIcon(":/icons/buttons/startserver"))
#define EQITWEBSERVER_MAINWINDOW_BUTTON_STOP_ICON QIcon::fromTheme("media-playback-stop", QIcon(":/icons/buttons/stopserver"))
#define EQITWEBSERVER_MAINWINDOW_BUTTON_QUIT_ICON QIcon::fromTheme("application-exit", QIcon(":/icons/buttons/exit"))

#endif

EquitWebServer::MainWindow::MainWindow(Server * server, QWidget * parent)
: QMainWindow(parent),
  m_server(server),
  m_statusBar(nullptr),
  m_menuBar(nullptr),
  m_serverMenu(nullptr),
  m_recentConfigsMenu(nullptr),
  m_controller(nullptr),
  m_requestReceivedCountLabel(nullptr),
  m_requestAcceptedCountLabel(nullptr),
  m_requestRejectedCountLabel(nullptr),
  m_requestReceivedCount(0),
  m_requestAcceptedCount(0),
  m_requestRejectedCount(0),
  m_startStopServer(nullptr) {
	/* generic, re-usable widget ptrs */
	QLabel * myLabel;
	//	QFrame * myFrame;

	/* main container and layout */
	QWidget * centralWidget = new QWidget;
	QVBoxLayout * vLayout = new QVBoxLayout;

	/* logo and controller */
	QHBoxLayout * hLayout = new QHBoxLayout;
	hLayout->addWidget(myLabel = new QLabel, 0, Qt::AlignTop);
	myLabel->setPixmap(QPixmap(":/pixmaps/applogo"));

	m_controller = new ConfigurationWidget(server);
	hLayout->addWidget(m_controller);
	vLayout->addLayout(hLayout);

	/* button box */
	QDialogButtonBox * bBox = new QDialogButtonBox;
	m_startStopServer = new QPushButton(EQITWEBSERVER_MAINWINDOW_BUTTON_START_ICON, tr("Start"));
	bBox->addButton(m_startStopServer, QDialogButtonBox::AcceptRole);
	connect(m_startStopServer, &QPushButton::clicked, this, &MainWindow::startServer);
	QPushButton * myButton = new QPushButton(EQITWEBSERVER_MAINWINDOW_BUTTON_QUIT_ICON, tr("Quit"));
	bBox->addButton(myButton, QDialogButtonBox::RejectRole);
	connect(myButton, &QPushButton::clicked, this, &MainWindow::close);
	vLayout->addWidget(bBox);

	centralWidget->setLayout(vLayout);
	setCentralWidget(centralWidget);

	/* status bar */
	m_statusBar = new QStatusBar();
	m_requestReceivedCountLabel = new ConnectionCountLabel(tr("Requests Received: %1"));
	m_requestAcceptedCountLabel = new ConnectionCountLabel(tr("Requests Accepted: %1"));
	m_requestRejectedCountLabel = new ConnectionCountLabel(tr("Requests Rejected: %1"));
	m_statusBar->addPermanentWidget(m_requestReceivedCountLabel);
	//	m_statusBar->addPermanentWidget(myFrame = new QFrame());
	//	myFrame->setFrameShape(QFrame::VLine);
	//	myFrame->setFrameShadow(QFrame::Sunken);
	m_statusBar->addPermanentWidget(m_requestAcceptedCountLabel);
	//	m_statusBar->addPermanentWidget(myFrame = new QFrame());
	//	myFrame->setFrameShape(QFrame::VLine);
	//	myFrame->setFrameShadow(QFrame::Sunken);
	m_statusBar->addPermanentWidget(m_requestRejectedCountLabel);
	//	m_statusBar->addPermanentWidget(myFrame = new QFrame());
	//	myFrame->setFrameShape(QFrame::VLine);
	//	myFrame->setFrameShadow(QFrame::Sunken);

	/* menu bar */
	m_menuBar = new QMenuBar();
	m_serverMenu = new QMenu(tr("&Server"));
	m_accessMenu = new QMenu(tr("Access"));
	m_contentMenu = new QMenu(tr("Content"));
	QMenu * helpMenu = new QMenu(tr("Help"));

	m_menuBar->addMenu(m_serverMenu);
	m_menuBar->addMenu(m_accessMenu);
	m_menuBar->addMenu(m_contentMenu);
	m_menuBar->addMenu(helpMenu);

	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_OPENCONFIG_ICON, tr("&Open Configuration..."), this, qOverload<>(&MainWindow::loadConfiguration));
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_SAVECONFIG_ICON, tr("&Save Configuration..."), this, &MainWindow::saveConfiguration);
	m_serverMenu->addAction(tr("Save &Default Configuration..."), this, &MainWindow::saveConfigurationAsDefault);
	m_serverMenu->addSeparator();
	m_recentConfigsMenu = new QMenu(tr("Recent Configurations"));
	m_serverMenu->addMenu(m_recentConfigsMenu);
	m_serverMenu->addSeparator();
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_CHOOSEDOCROOT_ICON, tr("Document Root..."), m_controller, &ConfigurationWidget::chooseDocumentRoot);
	m_serverMenu->addAction(tr("Listen on localhost"), m_controller, &ConfigurationWidget::bindToLocalhost);
	m_serverMenu->addAction(tr("Listen on host address"), m_controller, &ConfigurationWidget::bindToHostAddress);
	m_serverMenu->addSeparator();
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STARTSERVER_ICON, tr("Start"), this, &MainWindow::startServer);
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STOPSERVER_ICON, tr("Stop"), this, &MainWindow::stopServer);
	m_serverMenu->addSeparator();
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_EXIT_ICON, tr("&Quit"), this, &MainWindow::close);

	m_accessMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_ALLOWUNKNOWNIPS_ICON, tr("Allow Unknown IPs"), m_controller, &ConfigurationWidget::setLiberalDefaultConnectionPolicy);
	m_accessMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_FORBIDUNKNOWNIPS_ICON, tr("Forbid Unknown IPs"), m_controller, &ConfigurationWidget::setRestrictedDefaultConnectionPolicy);
	m_accessMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_CLEARIPLIST_ICON, tr("Clear IP Access List"), m_controller, &ConfigurationWidget::clearIPConnectionPolicies);

	m_contentMenu->addAction(tr("Clear all MIME type associations"), m_controller, &ConfigurationWidget::clearAllFileExtensionMIMETypes);
	m_contentMenu->addAction(tr("Clear all actions"), m_controller, &ConfigurationWidget::clearAllActions);

	helpMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUT_ICON, tr("About"), this, &MainWindow::about);
	helpMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUTQT_ICON, tr("About Qt"), qApp, &QApplication::aboutQt);

	setMenuBar(m_menuBar);
	setStatusBar(m_statusBar);

	setWindowTitle(qApp->applicationDisplayName());
	setWindowIcon(QIcon(":/pixmaps/applogo"));

	connect(m_server, &Server::connectionReceived, m_requestReceivedCountLabel, qOverload<>(&ConnectionCountLabel::increment));
	connect(m_server, &Server::connectionRejected, m_requestRejectedCountLabel, qOverload<>(&ConnectionCountLabel::increment));
	connect(m_server, &Server::connectionAccepted, m_requestAcceptedCountLabel, qOverload<>(&ConnectionCountLabel::increment));
	connect(m_controller, &ConfigurationWidget::documentRootChanged, this, &MainWindow::slotDocumentRootChanged);
	readRecentConfigs();
}


EquitWebServer::MainWindow::~MainWindow(void) {
	if(m_controller) {
		m_controller->setServer(nullptr);
	}

	delete m_server;
	m_server = nullptr;
	saveRecentConfigs();
}


void EquitWebServer::MainWindow::ensureUserConfigDir(void) {
	QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

	if(!configDir.exists()) {
		QDir("/").mkpath(configDir.absolutePath());
	}
}


void EquitWebServer::MainWindow::readRecentConfigs(void) {
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

		m_recentConfigs.append(line);
		QAction * a = m_recentConfigsMenu->addAction(line, this, &MainWindow::loadRecentConfiguration);
		a->setData(line);
		a->setCheckable(true);
	}

	qDebug() << "read" << m_recentConfigs.size() << "recent configurations";
}


void EquitWebServer::MainWindow::saveRecentConfigs(void) {
	QFile recentConfigsFile(QStandardPaths::locate(QStandardPaths::AppConfigLocation, "recentconfigs"));

	if(!recentConfigsFile.open(QIODevice::WriteOnly)) {
		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to update recent configs file (couldn't open \"" << qPrintable(recentConfigsFile.fileName()) << "\" for writing)\n";
		return;
	}

	recentConfigsFile.write(m_recentConfigs.join("\n").toUtf8());
	recentConfigsFile.close();
}


void EquitWebServer::MainWindow::loadRecentConfiguration(void) {
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


void EquitWebServer::MainWindow::slotDocumentRootChanged(void) {
	if(m_server->isListening())
		QMessageBox::warning(this, tr("Set document root"), tr("The document root was changed and the server is currently running. This means that the actual document root being used to serve content will not be altered until the server is restarted. Content will continue to be served from the document root that was set when the server was last started."));
}


void EquitWebServer::MainWindow::saveConfiguration(void) {
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


void EquitWebServer::MainWindow::saveConfigurationAsDefault(void) {
	QString configFilePath = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "defaultsettings.ewcx");
	std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: attempting to save to \"" << qPrintable(configFilePath) << "\"\n";

	if(!m_server->configuration().save(configFilePath)) {
		QMessageBox d(QMessageBox::Warning, tr("Save Webserver Configuration"), tr("The current configuration could not be saved as the default configuration."), QMessageBox::Ok, this);
		d.setDetailedText(tr("It was not possible to write to the file \"%1\".").arg(configFilePath));
		d.exec();
		return;
	}
	else {
		// TODO use notifications
		QMessageBox::information(this, tr("Save Webserver Configuration"), tr("The current configuration was saved as the default."));
	}
}


void EquitWebServer::MainWindow::loadConfiguration(void) {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Webserver Configuration"), QString(), "bpWebServer Configuration Files (*.ewcx)");

	if(fileName.isEmpty()) {
		return;
	}

	loadConfiguration(fileName);
}


void EquitWebServer::MainWindow::loadConfiguration(const QString & fileName) {
	EquitWebServer::Configuration newConfig;

	if(newConfig.load(fileName)) {
		for(auto * action : m_recentConfigsMenu->actions()) {
			action->setChecked(false);
		}

		if(!m_recentConfigs.contains(fileName)) {
			m_recentConfigs.append(fileName);
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


bool EquitWebServer::MainWindow::startServer(void) {
	if(m_server->isListening()) {
		return true;
	}

	if(m_server->listen()) {
		statusBar()->showMessage(tr("The server is listening on %1:%2.").arg(m_server->configuration().listenAddress()).arg(m_server->configuration().port()));
		m_controller->disableWidgets();
		disconnect(m_startStopServer, SIGNAL(clicked()), this, SLOT(startServer()));
		connect(m_startStopServer, SIGNAL(clicked()), this, SLOT(stopServer()));
		m_startStopServer->setIcon(EQITWEBSERVER_MAINWINDOW_BUTTON_STOP_ICON);
		m_startStopServer->setText(tr("Stop"));
	}
	else {
		statusBar()->showMessage(tr("The server could not be started."));
		m_controller->enableWidgets();
		disconnect(m_startStopServer, SIGNAL(clicked()), this, SLOT(stopServer()));
		connect(m_startStopServer, SIGNAL(clicked()), this, SLOT(startServer()));
		m_startStopServer->setIcon(EQITWEBSERVER_MAINWINDOW_BUTTON_START_ICON);
		m_startStopServer->setText(tr("Start"));
	}

	return false;
}


bool EquitWebServer::MainWindow::stopServer() {
	if(!m_server->isListening()) {
		return true;
	}

	m_server->close();

	if(m_server->isListening()) {
		statusBar()->showMessage(tr("The server could not be stopped. The server is listening on port %1.").arg(m_server->configuration().port()));
		m_controller->disableWidgets();
		disconnect(m_startStopServer, SIGNAL(clicked()), this, SLOT(startServer()));
		connect(m_startStopServer, SIGNAL(clicked()), this, SLOT(stopServer()));
		m_startStopServer->setIcon(EQITWEBSERVER_MAINWINDOW_BUTTON_STOP_ICON);
		m_startStopServer->setText(tr("Stop"));
	}
	else {
		statusBar()->showMessage(tr("The server currently offline."));
		m_controller->enableWidgets();
		disconnect(m_startStopServer, SIGNAL(clicked()), this, SLOT(stopServer()));
		connect(m_startStopServer, SIGNAL(clicked()), this, SLOT(startServer()));
		m_startStopServer->setIcon(EQITWEBSERVER_MAINWINDOW_BUTTON_START_ICON);
		m_startStopServer->setText(tr("Start"));
	}

	return !m_server->isListening();
}


void EquitWebServer::MainWindow::about(void) {
	QMessageBox::about(this, tr("About %1").arg(qApp->applicationDisplayName()),
							 QString("<p><big><strong>") + qApp->applicationDisplayName() + " v" + qApp->applicationVersion() + "</strong></big></p>"

																																								 "<p style=\"font-weight: normal;\"><small>A simple web server for desktop use.</small></p>"

																																								 "<p style=\"font-weight: normal;\"><small>Written by Darren Edale for <strong>&Eacute;quit</strong> (<a href=\"http://www.equituk.net\">http://www.equituk.net/</a>)</small></p>"

																																								 "<p style=\"font-weight: normal;\"><small>This program is intended for short-term use on the desktop. <strong>It is not a production-strength webserver and should not"
																																								 " be used as one.</strong></small></p>"

																																								 "<p style=\"font-weight: normal;\"><small>" +
								qApp->applicationDisplayName() + " uses the Qt toolkit (<a href=\"http://www.qt.io/\">http://www.qt.io/</a>).</small></p>"

																			"<p style=\"font-weight: normal;\"><small>" +
								qApp->applicationDisplayName() + " uses some icons from the KDE <a href=\"https://github.com/KDE/oxygen-icons/\">Oxygen</a> icon project, licensed under the <a href=\"http://www.gnu.org/licenses/lgpl-3.0.txt\">LGPLv3</a>.</small></p>");
}


void EquitWebServer::MainWindow::serverStarted(void) {
	disconnect(m_startStopServer, SIGNAL(clicked()), m_controller, SLOT(startServer()));
	connect(m_startStopServer, SIGNAL(clicked()), m_controller, SLOT(stopServer()));
	m_startStopServer->setIcon(EQITWEBSERVER_MAINWINDOW_BUTTON_STOP_ICON);
	m_startStopServer->setText(tr("Stop"));
}


void EquitWebServer::MainWindow::serverStopped(void) {
}


void EquitWebServer::MainWindow::incrementRequestReceivedCount(void) {
	setRequestReceivedCount(m_requestReceivedCount + 1);
}


void EquitWebServer::MainWindow::incrementRequestAcceptedCount(void) {
	setRequestAcceptedCount(m_requestAcceptedCount + 1);
}


void EquitWebServer::MainWindow::incrementRequestRejectedCount(void) {
	setRequestRejectedCount(m_requestRejectedCount + 1);
}


void EquitWebServer::MainWindow::setRequestReceivedCount(int c) {
	m_requestReceivedCount = c;
	m_requestReceivedCountLabel->setText(tr("Requests Received: %1").arg(m_requestReceivedCount));
}


void EquitWebServer::MainWindow::setRequestAcceptedCount(int c) {
	m_requestAcceptedCount = c;
	m_requestAcceptedCountLabel->setText(tr("Requests Accepted: %1").arg(m_requestAcceptedCount));
}


void EquitWebServer::MainWindow::setRequestRejectedCount(int c) {
	m_requestRejectedCount = c;
	m_requestRejectedCountLabel->setText(tr("Requests Rejected: %1").arg(m_requestRejectedCount));
}


void EquitWebServer::MainWindow::resetRequestReceivedCount(void) {
	setRequestReceivedCount(0);
}


void EquitWebServer::MainWindow::resetRequestAcceptedCount(void) {
	setRequestAcceptedCount(0);
}


void EquitWebServer::MainWindow::resetRequestRejectedCount(void) {
	setRequestRejectedCount(0);
}


void EquitWebServer::MainWindow::resetAllRequestCounts(void) {
	resetRequestReceivedCount();
	resetRequestAcceptedCount();
	resetRequestRejectedCount();
}


void EquitWebServer::MainWindow::setStatusMessage(QString msg) {
	m_statusBar->showMessage(msg);
}
