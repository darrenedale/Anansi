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

/// \file configurationwidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date 19th June, 2012
///
/// \brief Implementation of the ConfigurationWidget class for Anansi.
///
/// \dep
/// - configurationwidget.h
/// - configurationwidget.ui
/// - <QtGlobal>
/// - <QString>
/// - <QStringBuilder>
/// - <QPixmap>
/// - <QIcon>
/// - <QVector>
/// - <QList>
/// - <QStringList>
/// - <QFileInfo>
/// - <QDir>
/// - <QHostAddress>
/// - <QAbstractSocket>
/// - <QItemDelegate>
/// - <QMessageBox>
/// - <QFileDialog>
/// - <QStandardPaths>
/// - <QNetworkInterface>
/// - <QStyledItemDelegate>
/// - window.h
/// - serverdetailswidget.h
/// - accesscontrolwidget.h
/// - fileassociationswidget.h
/// - mimeactionswidget.h
/// - accesslogwidget.h
/// - connectionpolicycombo.h
/// - webserveractioncombo.h
/// - directorylistingsortordercombo.h
/// - mimecombo.h
/// - mimeicons.h
/// - strings.h
/// - notifications.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "configurationwidget.h"
#include "ui_configurationwidget.h"

#include <iostream>
#include <unordered_set>

#include <QtGlobal>
#include <QString>
#include <QStringBuilder>
#include <QPixmap>
#include <QIcon>
#include <QVector>
#include <QList>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QItemDelegate>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QNetworkInterface>
#include <QStyledItemDelegate>

#include "window.h"
#include "server.h"
#include "configuration.h"
#include "serverdetailswidget.h"
#include "accesscontrolwidget.h"
#include "fileassociationswidget.h"
#include "mimeactionswidget.h"
#include "accesslogwidget.h"
#include "connectionpolicycombo.h"
#include "webserveractioncombo.h"
#include "directorylistingsortordercombo.h"
#include "mimecombo.h"
#include "mimeicons.h"
#include "strings.h"
#include "notifications.h"
#include "qtmetatypes.h"


namespace Anansi {


	using Equit::starts_with;


	ConfigurationWidget::ConfigurationWidget(QWidget * parent)
	: QWidget(parent),
	  m_server(nullptr),
	  m_ui(std::make_unique<Ui::ConfigurationWidget>()) {
		m_ui->setupUi(this);
		m_ui->picker->setCurrentRow(0);
		m_ui->splitter->setStretchFactor(0, 0);
		m_ui->splitter->setStretchFactor(1, 1);

		// server config slots
		connect(m_ui->serverDetails, &ServerDetailsWidget::documentRootChanged, [this](const QString & docRoot) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			if(!m_server->configuration().setDocumentRoot(docRoot)) {
				showNotification(this, tr("<p>The document root could not be set to <strong>%1</strong>.</p>").arg(docRoot), NotificationType::Error);
			}
		});

		connect(m_ui->serverDetails, &ServerDetailsWidget::cgiBinChanged, [this](const QString & cgiBin) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			if(!m_server->configuration().setCgiBin(cgiBin)) {
				showNotification(this, tr("<p>The cgi-bin directory could not be set to <strong>%1</strong>.</p>").arg(cgiBin), NotificationType::Error);
			}

			auto cgiBinInfo = QFileInfo(cgiBin);
			auto docRootInfo = QFileInfo(m_server->configuration().documentRoot());

			// if path does not exist, absoluteFilePath() returns empty which could result
			// in false positives
			if(cgiBinInfo.exists() && docRootInfo.exists() && starts_with(cgiBinInfo.absoluteFilePath(), docRootInfo.absoluteFilePath())) {
				showNotification(this, tr("<p>The cgi-bin directory is inside the document root.</p><p><small>This can be a security risk in some circumstances.</small></p>"), NotificationType::Warning);
			}

			// NEXTRELEASE warn if system program location (e.g. /usr/bin, C:\Program Files)
		});

		connect(m_ui->serverDetails, &ServerDetailsWidget::listenIpAddressChanged, [this](const QString & addr) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			if(!m_server->configuration().setListenAddress(addr)) {
				showNotification(this, tr("<p>The listen address could not be set to <strong>%1</strong>.</p><p><small>This is likely because it's not a valid dotted-decimal IPv4 address.</small></p>").arg(addr), NotificationType::Error);
				m_ui->serverDetails->setListenAddress(m_server->configuration().listenAddress());
			}
			else if(m_server->isListening()) {
				showNotification(this, tr("<p>The listen address was changed while the server was running. This will not take effect until the server is restarted.</p><p><small>The server will continue to listen on the previous address until it is restarted.</small></p>"), NotificationType::Warning);
			}
		});

		connect(m_ui->serverDetails, &ServerDetailsWidget::listenPortChanged, [this](quint16 port) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			if(!m_server->configuration().setPort(port)) {
				showNotification(this, tr("<p>The listen port could not be set to <strong>%1</strong>.</p><p><small>The port must be between 1 and 65535.</small></p>").arg(port), NotificationType::Error);
				auto oldPort = m_server->configuration().port();

				if(-1 == oldPort) {
					m_ui->serverDetails->setListenPort(Configuration::DefaultPort);
				}
				else {
					m_ui->serverDetails->setListenPort(static_cast<uint16_t>(oldPort));
				}
			}
			else if(m_server->isListening()) {
				showNotification(this, tr("<p>The listen port was changed while the server was running. This will not take effect until the server is restarted.</p><p><small>The server will continue to listen on the previous port until it is restarted.</small></p>"), NotificationType::Warning);
			}
		});

		connect(m_ui->serverDetails, &ServerDetailsWidget::administratorEmailChanged, [this](const QString & adminEmail) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setAdministratorEmail(adminEmail);
		});

		connect(m_ui->allowServingCgiBin, &QCheckBox::toggled, [this](bool allow) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setAllowServingFilesFromCgiBin(allow);

			if(allow) {
				showTransientNotification(this, tr("<p>Allowing direct access to files inside your CGI bin directory is considered a security risk. This option should be used sparingly and with caution.</p><p><small>This option only has any effect if your CGI bin directory is inside your document root. If it is outside your document root, files in your CGI bin directory are not directly accessible.</small></p>"), NotificationType::Warning);
			}
		});

		connect(m_ui->allowDirectoryListings, &QCheckBox::toggled, [this](bool allow) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_ui->sortOrder->setEnabled(allow);
			m_ui->sortOrderLabel->setEnabled(allow);
			m_ui->showHiddenFiles->setEnabled(allow);
			m_server->configuration().setDirectoryListingsAllowed(allow);
		});

		connect(m_ui->showHiddenFiles, &QCheckBox::toggled, [this](bool show) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setShowHiddenFilesInDirectoryListings(show);
		});

		connect(m_ui->sortOrder, &DirectoryListingSortOrderCombo::sortOrderChanged, [this](DirectoryListingSortOrder order) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setDirectoryListingSortOrder(order);
		});
	}


	ConfigurationWidget::ConfigurationWidget(Server * server, QWidget * parent)
	: ConfigurationWidget(parent) {
		setServer(server);
	}


	ConfigurationWidget::~ConfigurationWidget() = default;


	void ConfigurationWidget::setServer(Server * server) {
		m_ui->fileAssociations->setServer(server);
		m_ui->mimeActions->setServer(server);
		m_ui->accessControl->setServer(server);
		m_server = server;

		if(m_server) {
			for(const auto & mimeType : m_server->configuration().registeredMimeTypes()) {
				m_ui->fileAssociations->addAvailableMimeType(mimeType);
			}

			readConfiguration();

			// prevent editing of listen address/port while server is listening
			connect(m_server, &Server::listeningStateChanged, m_ui->serverDetails, &QWidget::setDisabled);

			connect(m_server, &Server::requestConnectionPolicyDetermined, m_ui->accessLog, qOverload<const QString &, uint16_t, ConnectionPolicy>(&AccessLogWidget::addPolicyEntry), Qt::QueuedConnection);
			connect(m_server, &Server::requestActionTaken, m_ui->accessLog, qOverload<const QString &, uint16_t, const QString &, WebServerAction>(&AccessLogWidget::addActionEntry), Qt::QueuedConnection);
		}
		else {
			setEnabled(false);
		}
	}


	void ConfigurationWidget::readConfiguration() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		std::array<QSignalBlocker, 9> blockers = {
		  {
			 QSignalBlocker(m_ui->serverDetails),
			 QSignalBlocker(m_ui->accessControl),
			 QSignalBlocker(m_ui->allowDirectoryListings),
			 QSignalBlocker(m_ui->allowServingCgiBin),
			 QSignalBlocker(m_ui->showHiddenFiles),
			 QSignalBlocker(m_ui->sortOrder),
			 QSignalBlocker(m_ui->fileAssociations),
			 QSignalBlocker(m_ui->mimeActions),
			 QSignalBlocker(m_ui->accessLog),
		  }};

		const Configuration & opts = m_server->configuration();
		m_ui->serverDetails->setDocumentRoot(opts.documentRoot());
		m_ui->serverDetails->setListenAddress(opts.listenAddress());
		m_ui->serverDetails->setCgiBin(opts.cgiBin());
		m_ui->serverDetails->setAdministratorEmail(opts.administratorEmail());

		int port = opts.port();

		if(port >= 0 && port <= 65535) {
			m_ui->serverDetails->setListenPort(static_cast<uint16_t>(port));
		}
		else {
			m_ui->serverDetails->setListenPort(Configuration::DefaultPort);
		}

		m_ui->allowServingCgiBin->setChecked(opts.allowServingFilesFromCgiBin());
		m_ui->allowDirectoryListings->setChecked(opts.directoryListingsAllowed());
		m_ui->showHiddenFiles->setChecked(opts.showHiddenFilesInDirectoryListings());
		m_ui->sortOrder->setSortOrder(opts.directoryListingSortOrder());

		setEnabled(true);
	}


	void ConfigurationWidget::clearAllActions() {
		m_ui->mimeActions->clear();
	}


	void ConfigurationWidget::clearAllFileExtensionMIMETypes() {
		QSignalBlocker block(m_ui->fileAssociations);
		m_ui->fileAssociations->clear();
	}


	void ConfigurationWidget::chooseDocumentRoot() {
		m_ui->serverDetails->chooseDocumentRoot();
	}


	void ConfigurationWidget::setListenAddress(const QString & addr) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		if(addr.isEmpty()) {
			return;
		}

		if(addr != m_ui->serverDetails->listenIpAddress()) {
			m_ui->serverDetails->setListenAddress(addr);
		}

		m_server->configuration().setListenAddress(addr);
	}


	void ConfigurationWidget::bindToLocalhost() {
		setListenAddress(QStringLiteral("127.0.0.1"));
	}


	void ConfigurationWidget::bindToHostAddress() {
		QString addr;

		for(const auto & hostAddress : QNetworkInterface::allAddresses()) {
			if(hostAddress.isLoopback()) {
				continue;
			}

			if(QAbstractSocket::IPv4Protocol != hostAddress.protocol()) {
				continue;
			}

			addr = hostAddress.toString();
			break;
		}

		if(addr.isNull()) {
			showNotification(this, tr("<p>This computer does not appear to have any IPv4 addresses.</p>"), NotificationType::Error);
			return;
		}

		setListenAddress(addr);
	}


	void ConfigurationWidget::setLiberalDefaultConnectionPolicy() {
		setDefaultConnectionPolicy(ConnectionPolicy::Accept);
	}


	void ConfigurationWidget::setRestrictiveDefaultConnectionPolicy() {
		setDefaultConnectionPolicy(ConnectionPolicy::Reject);
	}


	void ConfigurationWidget::setDefaultConnectionPolicy(ConnectionPolicy policy) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_ui->accessControl->setDefaultConnectionPolicy(policy);
	}


	void ConfigurationWidget::setDefaultMimeType(const QString & mimeType) {
		m_ui->fileAssociations->setDefaultMimeType(mimeType);
	}


	void ConfigurationWidget::setDefaultAction(WebServerAction action) {
		m_ui->mimeActions->setDefaultAction(action);
	}


	void ConfigurationWidget::clearIpConnectionPolicies() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_ui->accessControl->clearAllConnectionPolicies();
		m_server->configuration().clearAllIpAddressConnectionPolicies();
	}


}  // namespace Anansi
