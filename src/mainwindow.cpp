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

/// \file mainwindow.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MainWindow class for Anansi..
///
/// \dep
/// - mainwindow.h
/// - mainwindow.ui
/// - <iostream>
/// - <QString>
/// - <QMessageBox>
/// - <QFileDialog>
/// - <QStandardPaths>
/// - types.h
/// - application.h
/// - server.h
/// - configurationwidget.h
/// - mainwindowstatusbar.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include "types.h"
#include "application.h"
#include "server.h"
#include "configurationwidget.h"
#include "mainwindowstatusbar.h"


namespace Anansi {


	MainWindow::MainWindow(QWidget * parent)
	: Window(parent),
	  m_server(nullptr),
	  m_ui(std::make_unique<Ui::MainWindow>()),
	  m_recentConfigActionGroup(std::make_unique<QActionGroup>(nullptr)) {
		m_ui->setupUi(this);
		setEnabled(false);
		m_ui->actionRecentConfigurations->setMenu(new QMenu);

		setWindowTitle(qApp->applicationDisplayName());
		setWindowIcon(QIcon(QStringLiteral(":/logo/app256")));

		connect(m_ui->startStop, &StartStopButton::startClicked, m_ui->actionStart, &QAction::trigger);
		connect(m_ui->startStop, &StartStopButton::stopClicked, m_ui->actionStop, &QAction::trigger);
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

		connect(m_recentConfigActionGroup.get(), &QActionGroup::triggered, [this](QAction * action) {
			loadConfiguration(action->data().value<QString>());
		});

		readRecentConfigurations();
	}


	MainWindow::MainWindow(std::unique_ptr<Server> server, QWidget * parent)
	: MainWindow(parent) {
		setServer(std::move(server));
	}


	MainWindow::~MainWindow() {
		m_ui->configuration->setServer(nullptr);
		saveRecentConfigurations();
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


	void MainWindow::readRecentConfigurations() {
		for(const auto & action : m_recentConfigActions) {
			m_recentConfigActionGroup->removeAction(action.get());
		}

		m_recentConfigActions.clear();
		auto * recentConfigsMenu = m_ui->actionRecentConfigurations->menu();
		Q_ASSERT_X(recentConfigsMenu, __PRETTY_FUNCTION__, "recent configurations menu is null");
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

			addRecentConfiguration(line);
		}
	}


	void MainWindow::saveRecentConfigurations() {
		QFile recentConfigsFile(QStandardPaths::locate(QStandardPaths::AppConfigLocation, "recentconfigs"));

		if(!recentConfigsFile.open(QIODevice::WriteOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to update recent configs file (couldn't open \"" << qPrintable(recentConfigsFile.fileName()) << "\" for writing)\n";
			return;
		}

		for(const auto & action : m_recentConfigActions) {
			recentConfigsFile.write(action->data().value<QString>().toUtf8());
			recentConfigsFile.putChar('\n');
		}

		recentConfigsFile.close();
	}


	void MainWindow::saveConfiguration() {
		static QString lastFileName;
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save Webserver Configuration"), lastFileName, tr("%1 configuration files (*.awcx)").arg(qApp->applicationDisplayName()));

		if(fileName.isEmpty()) {
			return;
		}

		if(!QFile::exists(fileName) || QMessageBox::Yes == QMessageBox::question(this, tr("Save Webserver Configuration"), "The file already exists. Are you sure you wish to overwrite it with the webserver configuration?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
			if(!m_server->configuration().saveAs(fileName)) {
				showInlineNotification(tr("Save Webserver Configuration"), tr("Could not save the configuration."), NotificationType::Error);
			}

			lastFileName = fileName;
		}
	}


	void MainWindow::saveConfigurationAsDefault() {
		QString configFilePath = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "defaultsettings.awcx");

		if(!m_server->configuration().saveAs(configFilePath)) {
			showInlineNotification(tr("The current configuration could not be saved as the default configuration.\nIt was not possible to write to the file \"%1\".").arg(configFilePath), NotificationType::Error);
			return;
		}

		showTransientInlineNotification(tr("The current configuration was saved as the default."));
	}


	void MainWindow::loadConfiguration() {
		static QString lastFileName;
		QString fileName = QFileDialog::getOpenFileName(this, tr("Load Webserver Configuration"), lastFileName, tr("%1 configuration files (*.awcx)").arg(qApp->applicationDisplayName()));

		if(fileName.isEmpty()) {
			return;
		}

		loadConfiguration(fileName);
		lastFileName = fileName;
	}


	void MainWindow::loadConfiguration(const QString & fileName) {
		const auto newConfig = Configuration::loadFrom(fileName);

		if(!newConfig) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to load the configuration\n";
			showInlineNotification(tr("Load Webserver Configuration"), tr("The configuration could not be loaded."), NotificationType::Error);
			return;
		}

		const auto & end = m_recentConfigActions.cend();

		auto actionIt = std::find_if(m_recentConfigActions.cbegin(), end, [&fileName](const std::unique_ptr<QAction> & action) -> bool {
			return action->data().value<QString>() == fileName;
		});

		QAction * action = nullptr;

		if(end == actionIt) {
			action = addRecentConfiguration(fileName);
		}
		else {
			action = (*actionIt).get();
		}

		Q_ASSERT_X(action, __PRETTY_FUNCTION__, "found null action for recent config item");

		QSignalBlocker block(m_recentConfigActionGroup.get());
		action->setChecked(true);
		m_server->setConfiguration(std::move(*newConfig));

		// force the UI to re-read the configuration
		m_ui->configuration->readConfiguration();
	}


	MainWindowStatusBar * MainWindow::statusBar() const {
		return m_ui->statusbar;
	}


	QAction * MainWindow::addRecentConfiguration(const QString & path) {
		auto * action = m_recentConfigActions.emplace_back(std::make_unique<QAction>(path)).get();  //std::make_unique<QAction>(path);
		action->setCheckable(true);
		action->setData(path);
		m_recentConfigActionGroup->addAction(action);
		m_ui->actionRecentConfigurations->menu()->addAction(action);
		return action;
	}


	bool MainWindow::startServer() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		if(m_server->isListening()) {
			return true;
		}

		if(!m_server->listen()) {
			showInlineNotification(tr("The server could not be started."), NotificationType::Error);
			m_ui->statusbar->showMessage({});
			m_ui->startStop->setState(StartStopButton::State::Start);
			return false;
		}

		showTransientInlineNotification(tr("Server started listening on %1:%2.").arg(m_server->configuration().listenAddress()).arg(m_server->configuration().port()));
		m_ui->statusbar->showMessage(tr("The server is listening on %1:%2.").arg(m_server->configuration().listenAddress()).arg(m_server->configuration().port()));
		m_ui->startStop->setState(StartStopButton::State::Stop);
		return true;
	}


	bool MainWindow::stopServer() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		if(!m_server->isListening()) {
			return true;
		}

		m_server->close();

		if(m_server->isListening()) {
			showInlineNotification(tr("The server could not be stopped."), NotificationType::Error);
			m_ui->startStop->setState(StartStopButton::State::Stop);
			return false;
		}

		showTransientInlineNotification(tr("The server was stopped successfully."));
		m_ui->statusbar->showMessage(tr("The server is currently offline."));
		m_ui->startStop->setState(StartStopButton::State::Start);
		return true;
	}


	void MainWindow::about() {
		const auto displayName = qApp->applicationDisplayName();
		const auto msg = tr(
								 "<p><big><strong>%1 v%2</strong></big></p><p style=\"font-weight: normal;\"><small>A simple web server for desktop use.</small></p>"
								 "<p style=\"font-weight: normal;\"><small>Written by Darren Edale for <strong>%3</strong> (<a href=\"https://%4\">https://%4/</a>)</small></p>"
								 "<p style=\"font-weight: normal;\"><small>This program is intended for short-term use on the desktop. <strong>It is not a production-strength webserver and should not be used as one.</strong></small></p>"
								 "<p style=\"font-weight: normal;\"><small>%1 uses the Qt toolkit (<a href=\"https://www.qt.io/\">https://www.qt.io/</a>).</small></p>"
								 "<p style=\"font-weight: normal;\"><small>%1 uses some icons from the KDE <a href=\"https://github.com/KDE/oxygen-icons/\">Oxygen</a> icon project, licensed under the <a href=\"http://www.gnu.org/licenses/lgpl-3.0.txt\">LGPLv3</a>.</small></p>")
								 .arg(displayName, qApp->applicationVersion(), qApp->organizationName(), qApp->organizationDomain());
		QMessageBox::about(this, tr("About %1").arg(displayName), msg);
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


}  // namespace Anansi
