/** \file MainWindow.cpp
  * \author darren Hatherley
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
  * \par Current Changes
  * - (2012-06-21) now warns the user if the document root is changed while the
  *   server is running that the change will not take effect until the next server
  *   restart.
  * - (2012-06-19) file documentation created.
  */

#include "MainWindow.h"

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
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STARTSERVER_ICON QIcon(":/icons/menu/startserver")
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STOPSERVER_ICON QIcon(":/icons/menu/stopserver")
#define EQITWEBSERVER_MAINWINDOW_MENU_SERVER_EXIT_ICON QIcon::fromTheme("application-exit", QIcon(":/icons/menu/exit"))
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_ALLOWUNKNOWNIPS_ICON QIcon(":/icons/connectionpolicies/accept")
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_FORBIDUNKNOWNIPS_ICON QIcon(":/icons/connectionpolicies/reject")
#define EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_CLEARIPLIST_ICON QIcon::fromTheme("edit-clear-list", QIcon(":/icons/menus/clearipaccesslist"))
#define EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUT_ICON QIcon::fromTheme("help-about", QIcon(":/icons/menu/about"))
#define EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUTQT_ICON QIcon(":/icons/menu/aboutqt")

#define EQITWEBSERVER_MAINWINDOW_BUTTON_START_ICON QIcon(":/icons/buttons/startserver")
#define EQITWEBSERVER_MAINWINDOW_BUTTON_STOP_ICON QIcon(":/icons/buttons/stopserver")
#define EQITWEBSERVER_MAINWINDOW_BUTTON_QUIT_ICON QIcon::fromTheme("application-exit", QIcon(":/icons/buttons/exit"))

#endif

EquitWebServer::MainWindow::MainWindow(Server * server, QWidget * parent)
: QMainWindow(parent),
  m_server(server),
  m_statusBar(0),
  m_menuBar(0),
  m_serverMenu(0),
  m_recentConfigsMenu(0),
  m_controller(0),
  m_requestReceivedCountLabel(0),
  m_requestAcceptedCountLabel(0),
  m_requestRejectedCountLabel(0),
  m_requestReceivedCount(0),
  m_requestAcceptedCount(0),
  m_requestRejectedCount(0),
  m_startStopServer(0) {
	/* generic, re-usable widget ptrs */
	QLabel * myLabel;
	//	QFrame * myFrame;

	/* main container and layout */
	QWidget * centralWidget = new QWidget();
	QVBoxLayout * vLayout = new QVBoxLayout();

	/* logo and controller */
	QHBoxLayout * hLayout = new QHBoxLayout();
	hLayout->addWidget(myLabel = new QLabel(), 0, Qt::AlignTop);
	myLabel->setPixmap(QPixmap(":/pixmaps/applogo"));

	m_controller = new ConfigurationWidget(server);
	hLayout->addWidget(m_controller);
	vLayout->addLayout(hLayout);

	/* button box */
	QDialogButtonBox * bBox = new QDialogButtonBox();
	m_startStopServer = new QPushButton(EQITWEBSERVER_MAINWINDOW_BUTTON_START_ICON, tr("Start"));
	bBox->addButton(m_startStopServer, QDialogButtonBox::AcceptRole);
	connect(m_startStopServer, SIGNAL(clicked()), this, SLOT(startServer()));
	QPushButton * myButton = new QPushButton(EQITWEBSERVER_MAINWINDOW_BUTTON_QUIT_ICON, tr("Quit"));
	bBox->addButton(myButton, QDialogButtonBox::RejectRole);
	connect(myButton, SIGNAL(clicked()), this, SLOT(close()));
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

	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_OPENCONFIG_ICON, tr("&Open Configuration..."), this, SLOT(loadConfiguration()));
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_SAVECONFIG_ICON, tr("&Save Configuration..."), this, SLOT(saveConfiguration()));
	m_serverMenu->addAction(tr("Save &Default Configuration..."), this, SLOT(saveConfigurationAsDefault()));
	m_serverMenu->addSeparator();
	m_recentConfigsMenu = new QMenu(tr("Recent Configurations"));
	m_serverMenu->addMenu(m_recentConfigsMenu);
	m_serverMenu->addSeparator();
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_CHOOSEDOCROOT_ICON, tr("Document Root..."), m_controller, SLOT(selectDocumentRoot()));
	m_serverMenu->addAction(tr("Listen on localhost"), m_controller, SLOT(bindToLocalhost()));
	m_serverMenu->addAction(tr("Listen on host address"), m_controller, SLOT(bindToHostAddress()));
	m_serverMenu->addSeparator();
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STARTSERVER_ICON, tr("Start"), this, SLOT(startServer()));
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_STOPSERVER_ICON, tr("Stop"), this, SLOT(stopServer()));
	m_serverMenu->addSeparator();
	m_serverMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_SERVER_EXIT_ICON, tr("&Quit"), this, SLOT(close()));

	m_accessMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_ALLOWUNKNOWNIPS_ICON, tr("Allow Unknown IPs"), m_controller, SLOT(setLiberalDefaultConnectionPolicy()));
	m_accessMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_FORBIDUNKNOWNIPS_ICON, tr("Forbid Unknown IPs"), m_controller, SLOT(setRestrictedDefaultConnectionPolicy()));
	m_accessMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_ACCESS_CLEARIPLIST_ICON, tr("Clear IP Access List"), m_controller, SLOT(clearIPConnectionPolicies()));

	m_contentMenu->addAction(tr("Clear all MIME type associations"), m_controller, SLOT(clearAllFileExtensionMIMETypes()));
	m_contentMenu->addAction(tr("Clear all actions"), m_controller, SLOT(clearAllActions()));

	helpMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUT_ICON, tr("About"), this, SLOT(about()));
	helpMenu->addAction(EQITWEBSERVER_MAINWINDOW_MENU_HELP_ABOUTQT_ICON, tr("About Qt"), qApp, SLOT(aboutQt()));

	setMenuBar(m_menuBar);
	setStatusBar(m_statusBar);

	setWindowTitle(qApp->applicationName());
	setWindowIcon(QIcon(":/pixmaps/applogo"));

	connect(m_server, SIGNAL(connectionReceived(QString, quint16)), m_requestReceivedCountLabel, SLOT(increment()));
	connect(m_server, SIGNAL(connectionRejected(QString, quint16)), m_requestRejectedCountLabel, SLOT(increment()));
	connect(m_server, SIGNAL(connectionAccepted(QString, quint16)), m_requestAcceptedCountLabel, SLOT(increment()));
	connect(m_controller, SIGNAL(documentRootChanged(QString)), this, SLOT(slotDocumentRootChanged()));
	readRecentConfigs();
}


EquitWebServer::MainWindow::~MainWindow(void) {
	if(m_server)
		delete m_server;
	if(m_controller)
		m_controller->setServer(0);
	m_server = 0;
	saveRecentConfigs();
}


void EquitWebServer::MainWindow::ensureUserConfigDir(void) {
	QDir home(QDir::home());
	if(!home.cd(".equit")) {
		home.mkdir(".equit");
		if(!home.cd(".equit"))
			return;
	}
}


void EquitWebServer::MainWindow::readRecentConfigs(void) {
	m_recentConfigs.clear();
	m_recentConfigsMenu->clear();

	ensureUserConfigDir();
	QDir home(QDir::home());
	if(!home.cd(".equit"))
		return;
	QFile f(home.absoluteFilePath("WebServerRecentConfigs"));
	if(!f.open(QIODevice::ReadOnly))
		return;
	QString line;

	while(!f.atEnd()) {
		line = QString::fromUtf8(f.readLine().trimmed());
		if(line.isEmpty())
			continue;
		m_recentConfigs.append(line);
		QAction * a = m_recentConfigsMenu->addAction(line, this, SLOT(loadRecentConfiguration()));
		a->setData(line);
		a->setCheckable(true);
	}

	qDebug() << "read" << m_recentConfigs.size() << "recent configurations";
}


void EquitWebServer::MainWindow::saveRecentConfigs(void) {
	ensureUserConfigDir();
	QDir home(QDir::home());

	if(!home.cd(".equit")) {
		qDebug() << "failed to update recent configs file: .equit dir does not exist";
		return;
	}

	QFile f(home.absoluteFilePath("WebServerRecentConfigs"));

	if(!f.open(QIODevice::WriteOnly)) {
		qDebug() << "failed to update recent configs file: couldn't open file for writing";
		return;
	}

	f.write(m_recentConfigs.join("\n").toUtf8());
	f.close();
}


void EquitWebServer::MainWindow::loadRecentConfiguration(void) {
	QAction * action = qobject_cast<QAction *>(sender());
	if(!action)
		return;
	loadConfiguration(action->data().toString());
	QMenu * menu = action->menu();

	if(menu) {
		foreach(QAction * a, menu->actions())
			a->setChecked(false);

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
	if(fileName.isEmpty())
		return;

	if(!QFile::exists(fileName) || QMessageBox::Yes == QMessageBox::question(this, tr("Save Webserver Configuration"), "The file already exists. Are you sure you wish to overwrite it with the webserver configuration?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
		if(!m_server->configuration().save(fileName)) {
			QMessageBox::warning(this, tr("Save Webserver Configuration"), tr("Could not save the configuration."));
		}
	}
}


void EquitWebServer::MainWindow::saveConfigurationAsDefault(void) {
	QDir homeDir(QDir::home());

	if(!homeDir.cd(".equit")) {
		homeDir.mkdir(".equit");

		if(!homeDir.cd(".equit")) {
			QMessageBox d(QMessageBox::Warning, tr("Save Webserver Configuration"), tr("The current configuration could not be saved as the default configuration."), QMessageBox::Ok, this);
			d.setDetailedText(tr("The folder \"%1\" in which the default configuration file is stored could not be found or created.").arg(homeDir.absolutePath()));
			d.exec();
			return;
		}
	}

	QString defaultPath = homeDir.absoluteFilePath("WebServerDefaults.ewcx");
	qDebug() << "attempting to save to" << defaultPath;

	if(homeDir == QDir::root() || !homeDir.exists() || !m_server->configuration().save(defaultPath)) {
		QMessageBox d(QMessageBox::Warning, tr("Save Webserver Configuration"), tr("The current configuration could not be saved as the default configuration."), QMessageBox::Ok, this);
		d.setDetailedText(tr("It was not possible to write to the file \"%1\".").arg(defaultPath));
		d.exec();
		return;
	}
	else
		QMessageBox::information(this, tr("Save Webserver Configuration"), tr("The current configuration was saved as the default."));
}


void EquitWebServer::MainWindow::loadConfiguration(void) {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Webserver Configuration"), QString(), "bpWebServer Configuration Files (*.ewcx)");

	if(fileName.isEmpty()) {
		qDebug() << "bpWebServer::bpWebServerController::loadConfiguration() - user cancelled file dialog";
		return;
	}

	loadConfiguration(fileName);
}


void EquitWebServer::MainWindow::loadConfiguration(const QString & fileName) {
	EquitWebServer::Configuration newConfig;

	if(newConfig.load(fileName)) {
		foreach(QAction * a, m_recentConfigsMenu->actions())
			a->setChecked(false);

		if(!m_recentConfigs.contains(fileName)) {
			m_recentConfigs.append(fileName);
			QAction * a = m_recentConfigsMenu->addAction(fileName, this, SLOT(loadRecentConfiguration()));
			a->setData(fileName);
			a->setCheckable(true);
			a->setChecked(true);
		}
		else {
			/* if the file name matches a recent config, tick it */
			foreach(QAction * a, m_recentConfigsMenu->actions()) {
				if(a->data().toString() == fileName)
					a->setChecked(true);
			}
		}

		m_server->setConfiguration(newConfig);
		m_controller->readConfiguration();
	}
	else {
		qDebug() << "bpWebServer::bpWebServerController::loadConfiguration() - failed to load the configuration";
		QMessageBox::warning(this, tr("Load Webserver Configuration"), "The configuration could not be loaded.");
	}
}


bool EquitWebServer::MainWindow::startServer(void) {
	if(m_server->isListening())
		return true;

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
	if(!m_server->isListening())
		return true;
	m_server->close();

	if(m_server->isListening()) {
		statusBar()->showMessage(tr("The server could not be stopped. The server is listening on port %1.").arg(m_server->configuration().port()));
		m_controller->disableWidgets();
		disconnect(m_startStopServer, SIGNAL(clicked()), this, SLOT(startServer()));
		connect(m_startStopServer, SIGNAL(clicked()), this, SLOT(stopServer()));
		m_startStopServer->setIcon(QIcon(":/icons/buttons/stopserver"));
		m_startStopServer->setText(tr("Stop"));
	}
	else {
		statusBar()->showMessage(tr("The server currently offline."));
		m_controller->enableWidgets();
		disconnect(m_startStopServer, SIGNAL(clicked()), this, SLOT(stopServer()));
		connect(m_startStopServer, SIGNAL(clicked()), this, SLOT(startServer()));
		m_startStopServer->setIcon(QIcon(":/icons/buttons/startserver"));
		m_startStopServer->setText(tr("Start"));
	}

	return !m_server->isListening();
}


void EquitWebServer::MainWindow::about(void) {
	QMessageBox::about(this, tr("About %1").arg(qApp->applicationName()),
							 QString("<p><big><strong>") + qApp->applicationName() + " v" + qApp->applicationVersion() + "</strong></big></p>"

																																						"<p style=\"font-weight: normal;\"><small>A simple web server for desktop use.</small></p>"

																																						"<p style=\"font-weight: normal;\"><small>Written by Darren Hatherley for <strong>&Eacute;quit</strong> (<a href=\"http://www.equituk.net\">http://www.www.equituk.net/</a>)</small></p>"

																																						"<p style=\"font-weight: normal;\"><small>This program is intended for short-term use on the desktop. <strong>It is not a production-strength webserver and should not"
																																						" be used as one.</strong></small></p>"

																																						"<p style=\"font-weight: normal;\"><small>" +
								qApp->applicationName() + " uses the Qt toolkit (<a href=\"http://qt.nokia.com/\">http://qt.nokia.com/</a>).</small></p>"

																  "<p style=\"font-weight: normal;\"><small>" +
								qApp->applicationName() + " uses some icons from the KDE <a href=\"http://www.oxygen-icons.org/\">Oxygen</a> icon project, licensed under the <a href=\"http://www.gnu.org/licenses/lgpl-3.0.txt\">LGPLv3</a>.</small></p>");
}


void EquitWebServer::MainWindow::serverStarted(void) {
	disconnect(m_startStopServer, SIGNAL(clicked()), m_controller, SLOT(startServer()));
	connect(m_startStopServer, SIGNAL(clicked()), m_controller, SLOT(stopServer()));
	m_startStopServer->setIcon(QIcon(":/icons/buttons/stopserver"));
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
