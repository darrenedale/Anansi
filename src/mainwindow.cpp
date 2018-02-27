/// \file MainWindow.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the MainWindow class for EquitWebServer.
///
/// \todo add "restart now" button to configuration-changed-while-running warning dialogues.
/// \todo do we need to do anything special in saveConfigurationAsDefault() regarding platform?
///
/// \par Changes
/// - (2012-06) now warns the user if the document root is changed while the server is running
///   that the change will not take effect until the next server restart.
/// - (2012-06) file documentation created.

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include "application.h"
#include "configurationwidget.h"
#include "mainwindowstatusbar.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);
Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);


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
		StartButtonIcon = QIcon::fromTheme("media-playback-start", QIcon(":/icons/buttons/startserver"));
		StopButtonIcon = QIcon::fromTheme("media-playback-stop", QIcon(":/icons/buttons/stopserver"));
		QuitButtonIcon = QIcon::fromTheme("application-exit", QIcon(":/icons/buttons/exit"));
		iconsInitialised = true;
	}

#else

	// NOTE QIcon requires QGuiApplication instance so can't be statically initialised.
	// they are initialised on construction of first MainWindow object
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


	MainWindow::MainWindow(QWidget * parent)
	: QMainWindow(parent),
	  m_server(nullptr),
	  m_ui(std::make_unique<Ui::MainWindow>()) {
		if(!iconsInitialised) {
			staticInitialise();
		}

		m_ui->setupUi(this);
		setEnabled(false);
		m_ui->startStop->setIcon(StartButtonIcon);

		connect(m_ui->configuration, &ConfigurationWidget::documentRootChanged, [this](const QString &) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

			if(m_server->isListening()) {
				// TODO inline dialogue or notification
				QMessageBox::warning(this, tr("Set document root"), tr("The document root was changed while the server was running. This means that the actual document root being used to serve content will not be altered until the server is restarted. Content will continue to be served from the document root that was set when the server was last started."));
			}
		});

		connect(m_ui->startStop, &QPushButton::clicked, [this]() {
			if(m_server->isListening()) {
				m_ui->actionStop->trigger();
			}
			else {
				m_ui->actionStart->trigger();
			}
		});

		connect(m_ui->quit, &QPushButton::clicked, m_ui->actionQuit, &QAction::trigger);

		connect(m_ui->actionStart, &QAction::triggered, this, &MainWindow::startServer);
		connect(m_ui->actionStop, &QAction::triggered, this, &MainWindow::stopServer);

		connect(m_ui->actionOpenConfiguration, &QAction::triggered, this, qOverload<>(&MainWindow::loadConfiguration));
		connect(m_ui->actionSaveConfiguration, &QAction::triggered, this, &MainWindow::saveConfiguration);
		connect(m_ui->actionSaveDefaultConfiguration, &QAction::triggered, this, &MainWindow::saveConfigurationAsDefault);
		connect(m_ui->actionDocumentRoot, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::chooseDocumentRoot);
		connect(m_ui->actionListenOnLocalhost, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::bindToLocalhost);
		connect(m_ui->actionListenOnHostAddress, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::bindToHostAddress);
		connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

		connect(m_ui->actionAllowUnknownIps, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::setLiberalDefaultConnectionPolicy);
		connect(m_ui->actionForbidUnknownIps, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::setRestrictiveDefaultConnectionPolicy);
		connect(m_ui->actionClearIpPolicyList, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::clearIpConnectionPolicies);

		connect(m_ui->actionClearAllMimeAssociations, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::clearAllFileExtensionMIMETypes);
		connect(m_ui->actionClearAllMimeActions, &QAction::triggered, m_ui->configuration, &ConfigurationWidget::clearAllActions);

		connect(m_ui->actionResetDefaultMimeType, &QAction::triggered, [this]() {
			m_ui->configuration->setDefaultMimeType(QStringLiteral("application/octet-stream"));
		});

		connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);

		setWindowTitle(qApp->applicationDisplayName());
		setWindowIcon(QIcon(QStringLiteral(":/logo/app256")));

		m_ui->actionRecentConfigurations->setMenu(new QMenu);

		readRecentConfigs();
	}


	MainWindow::MainWindow(std::unique_ptr<Server> server, QWidget * parent)
	: MainWindow(parent) {
		setServer(std::move(server));
	}


	MainWindow::~MainWindow() {
		m_ui->configuration->setServer(nullptr);
		saveRecentConfigs();
	}

	void MainWindow::setServer(std::unique_ptr<Server> server) {
		Q_ASSERT_X(!m_server, __PRETTY_FUNCTION__, "server is already set");
		m_server = std::move(server);

		m_ui->configuration->setServer(m_server.get());

		connect(m_server.get(), &Server::connectionReceived, statusBar(), &MainWindowStatusBar::incrementReceived);
		connect(m_server.get(), &Server::connectionRejected, statusBar(), &MainWindowStatusBar::incrementRejected);
		connect(m_server.get(), &Server::connectionAccepted, statusBar(), &MainWindowStatusBar::incrementAccepted);

		setEnabled(true);
	}


	void MainWindow::readRecentConfigs() {
		m_recentConfigs.clear();
		auto * recentConfigsMenu = m_ui->actionRecentConfigurations->menu();
		recentConfigsMenu->clear();

		Application::ensureUserConfigDir();
		QFile recentConfigsFile(QStandardPaths::locate(QStandardPaths::AppConfigLocation, "recentconfigs"));

		if(!recentConfigsFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to open recent configs file\n";
			return;
		}

		while(!recentConfigsFile.atEnd()) {
			const auto line = QString::fromUtf8(recentConfigsFile.readLine().trimmed());

			if(line.isEmpty()) {
				continue;
			}

			m_recentConfigs.push_back(line);
			auto * action = recentConfigsMenu->addAction(line, this, &MainWindow::loadRecentConfiguration);
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


	// TODO use an action group for this
	void MainWindow::loadRecentConfiguration() {
		QAction * action = qobject_cast<QAction *>(sender());

		Q_ASSERT_X(action && action->menu() == m_ui->actionRecentConfigurations->menu(), __PRETTY_FUNCTION__, "sender to loadRecentConfiguration slot is not an action in the recent configurations submenu");

		if(!action) {
			return;
		}

		loadConfiguration(action->data().toString());
		auto * recentConfigsMenu = action->menu();

		if(recentConfigsMenu) {
			for(auto * menuAction : recentConfigsMenu->actions()) {
				menuAction->setChecked(false);
			}

			action->setChecked(true);
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
			// TODO use notification or inline dialogue
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

		// TODO re-enable recent configs menu
		if(newConfig.read(fileName)) {
			auto * const recentConfigsMenu = m_ui->actionRecentConfigurations->menu();

			for(auto * action : recentConfigsMenu->actions()) {
				action->setChecked(false);
			}

			if(const auto & end = m_recentConfigs.cend(); end == std::find(m_recentConfigs.cbegin(), end, fileName)) {
				m_recentConfigs.push_back(fileName);
				QAction * newAction = recentConfigsMenu->addAction(fileName, this, &MainWindow::loadRecentConfiguration);
				newAction->setData(fileName);
				newAction->setCheckable(true);
				newAction->setChecked(true);
			}
			else {
				/* if the file name matches a recent config, tick it */
				for(auto * action : recentConfigsMenu->actions()) {
					if(action->data().value<QString>() == fileName) {
						action->setChecked(true);
					}
				}
			}

			m_server->setConfiguration(newConfig);
			m_ui->configuration->readConfiguration();
		}
		else {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load the configuration\n";
			QMessageBox::warning(this, tr("Load Webserver Configuration"), "The configuration could not be loaded.");
		}
	}

	MainWindowStatusBar * MainWindow::statusBar() const {
		return m_ui->statusbar;
	}


	bool MainWindow::startServer() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		if(m_server->isListening()) {
			return true;
		}

		// TODO custom button class for start/stop to remove repeated code
		if(m_server->listen()) {
			// TODO configuration has setServer(), so it can listen for when server starts/stops
			m_ui->statusbar->showMessage(tr("The server is listening on %1:%2.").arg(m_server->configuration().listenAddress()).arg(m_server->configuration().port()));
			m_ui->configuration->disableWidgets();
			m_ui->startStop->setIcon(StopButtonIcon);
			m_ui->startStop->setText(tr("Stop"));
		}
		else {
			// TODO configuration has setServer(), so it can listen for when server starts/stops
			m_ui->statusbar->showMessage(tr("The server could not be started."));
			m_ui->configuration->enableWidgets();
			m_ui->startStop->setIcon(StartButtonIcon);
			m_ui->startStop->setText(tr("Start"));
		}

		return false;
	}


	bool MainWindow::stopServer() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		if(!m_server->isListening()) {
			return true;
		}

		m_server->close();

		// TODO custom button class for start/stop to remove repeated code
		if(m_server->isListening()) {
			// TODO configuration has setServer(), so it can listen for when server starts/stops
			m_ui->statusbar->showMessage(tr("The server could not be stopped. The server is listening on %1:%2.").arg(m_server->configuration().listenAddress()).arg(m_server->configuration().port()));
			m_ui->configuration->disableWidgets();
			m_ui->startStop->setIcon(StopButtonIcon);
			m_ui->startStop->setText(tr("Stop"));
		}
		else {
			// TODO configuration has setServer(), so it can listen for when server starts/stops
			m_ui->statusbar->showMessage(tr("The server currently offline."));
			m_ui->configuration->enableWidgets();
			m_ui->startStop->setIcon(StartButtonIcon);
			m_ui->startStop->setText(tr("Start"));
		}

		return !m_server->isListening();
	}


	void MainWindow::about() {
		QMessageBox::about(this, tr("About %1").arg(qApp->applicationDisplayName()), QString("<p><big><strong>") + qApp->applicationDisplayName() + " v" + qApp->applicationVersion() + "</strong></big></p><p style=\"font-weight: normal;\"><small>A simple web server for desktop use.</small></p><p style=\"font-weight: normal;\"><small>Written by Darren Edale for <strong>&Eacute;quit</strong> (<a href=\"http://www.equituk.net\">http://www.equituk.net/</a>)</small></p><p style=\"font-weight: normal;\"><small>This program is intended for short-term use on the desktop. <strong>It is not a production-strength webserver and should not be used as one.</strong></small></p><p style=\"font-weight: normal;\"><small>" + qApp->applicationDisplayName() + " uses the Qt toolkit (<a href=\"http://www.qt.io/\">http://www.qt.io/</a>).</small></p><p style=\"font-weight: normal;\"><small>" + qApp->applicationDisplayName() + " uses some icons from the KDE <a href=\"https://github.com/KDE/oxygen-icons/\">Oxygen</a> icon project, licensed under the <a href=\"http://www.gnu.org/licenses/lgpl-3.0.txt\">LGPLv3</a>.</small></p>");
	}


	void MainWindow::incrementRequestReceivedCount() {
		m_ui->statusbar->incrementReceived();
	}


	void MainWindow::incrementRequestAcceptedCount() {
		m_ui->statusbar->incrementAccepted();
	}


	void MainWindow::incrementRequestRejectedCount() {
		m_ui->statusbar->incrementRejected();
	}


	void MainWindow::resetRequestReceivedCount() {
		m_ui->statusbar->resetReceived();
	}


	void MainWindow::resetRequestAcceptedCount() {
		m_ui->statusbar->resetAccepted();
	}


	void MainWindow::resetRequestRejectedCount() {
		m_ui->statusbar->resetRejected();
	}


	void MainWindow::resetAllRequestCounts() {
		m_ui->statusbar->resetAllCounters();
	}


	void MainWindow::setStatusMessage(const QString & msg) {
		m_ui->statusbar->showMessage(msg);
	}


}  // namespace EquitWebServer
