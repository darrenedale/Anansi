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
/// - <cstdint>
/// - <array>
/// - <Qt>
/// - <QString>
/// - <QFileInfo>
/// - <QSignalBlocker>
/// - <QNetworkInterface>
/// - <QAbstractSocket>
/// - assert.h
/// - server.h
/// - strings.h
/// - types.h
/// - notifications.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "configurationwidget.h"
#include "ui_configurationwidget.h"

#include <cstdint>
#include <array>

#include <Qt>
#include <QString>
#include <QFileInfo>
#include <QSignalBlocker>
#include <QNetworkInterface>
#include <QAbstractSocket>

#include "assert.h"
#include "server.h"
#include "strings.h"
#include "types.h"
#include "notifications.h"
#include "qtmetatypes.h"


namespace Anansi {


	using Equit::starts_with;

	static constexpr const int HeadingLogoPixmapSize = 48;


	ConfigurationWidget::ConfigurationWidget(QWidget * parent)
	: QWidget(parent),
	  m_server(nullptr),
	  m_ui(std::make_unique<Ui::ConfigurationWidget>()) {
		m_ui->setupUi(this);

		auto headerFont = m_ui->headingTitle->font();
		headerFont.setBold(true);
		headerFont.setPointSizeF(headerFont.pointSizeF() * 1.5);
		m_ui->headingTitle->setFont(headerFont);

		m_ui->headingLogo->setMinimumHeight(HeadingLogoPixmapSize);
		m_ui->picker->setCurrentRow(0);
		m_ui->stackedWidget->setCurrentIndex(0);
		setHeadingIcon(m_ui->picker->item(0)->icon());
		setHeading(tr("Server details"));

		m_ui->splitter->setStretchFactor(0, 0);
		m_ui->splitter->setStretchFactor(1, 1);

		connect(m_ui->picker, &QListWidget::currentRowChanged, [this](int rowIdx) {
			m_ui->stackedWidget->setCurrentIndex(rowIdx);
			const auto * const visibleWidget = m_ui->stackedWidget->currentWidget();
			setHeadingIcon(m_ui->picker->item(rowIdx)->icon());

			if(visibleWidget == m_ui->serverDetails) {
				setHeading(tr("Server details"));
			}
			else if(visibleWidget == m_ui->accessControl) {
				setHeading(tr("Access control"));
			}
			else if(visibleWidget == m_ui->contentControl) {
				setHeading(tr("Content control"));
			}
			else if(visibleWidget == m_ui->accessLog) {
				setHeading(tr("Access log"));
			}
			else {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: unrecognised item selected in main picker\n";
			}
		});

		connect(m_ui->allowServingCgiBin, &QCheckBox::toggled, [this](bool allow) {
			eqAssert(m_server, "server must not be null");
			m_server->configuration().setAllowServingFilesFromCgiBin(allow);

			if(allow) {
				showTransientNotification(this, tr("<p>Allowing direct access to files inside your CGI bin directory is considered a security risk. This option should be used sparingly and with caution.</p><p><small>This option only has any effect if your CGI bin directory is inside your document root. If it is outside your document root, files in your CGI bin directory are not directly accessible.</small></p>"), NotificationType::Warning);
			}
		});

		connect(m_ui->allowDirectoryListings, &QCheckBox::toggled, [this](bool allow) {
			eqAssert(m_server, "server must not be null");
			m_ui->sortOrder->setEnabled(allow);
			m_ui->sortOrderLabel->setEnabled(allow);
			m_ui->showHiddenFiles->setEnabled(allow);
			m_server->configuration().setDirectoryListingsAllowed(allow);
		});

		connect(m_ui->showHiddenFiles, &QCheckBox::toggled, [this](bool show) {
			eqAssert(m_server, "server must not be null");
			m_server->configuration().setShowHiddenFilesInDirectoryListings(show);
		});

		connect(m_ui->sortOrder, &DirectoryListingSortOrderCombo::sortOrderChanged, [this](DirectoryListingSortOrder order) {
			eqAssert(m_server, "server must not be null");
			m_server->configuration().setDirectoryListingSortOrder(order);
		});
	}


	ConfigurationWidget::ConfigurationWidget(Server * server, QWidget * parent)
	: ConfigurationWidget(parent) {
		setServer(server);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	ConfigurationWidget::~ConfigurationWidget() = default;


	void ConfigurationWidget::setServer(Server * server) {
		m_ui->serverDetails->setServer(server);
		m_ui->fileAssociations->setServer(server);
		m_ui->mediaTypeActions->setServer(server);
		m_ui->accessControl->setServer(server);
		m_server = server;

		if(m_server) {
			for(const auto & mediaType : m_server->configuration().registeredMediaTypes()) {
				m_ui->fileAssociations->addAvailableMediaType(mediaType);
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
		eqAssert(m_server, "server must not be null");

		std::array<QSignalBlocker, 9> blockers = {
		  {
			 QSignalBlocker(m_ui->serverDetails),
			 QSignalBlocker(m_ui->accessControl),
			 QSignalBlocker(m_ui->allowDirectoryListings),
			 QSignalBlocker(m_ui->allowServingCgiBin),
			 QSignalBlocker(m_ui->showHiddenFiles),
			 QSignalBlocker(m_ui->sortOrder),
			 QSignalBlocker(m_ui->fileAssociations),
			 QSignalBlocker(m_ui->mediaTypeActions),
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
		m_ui->mediaTypeActions->clear();
	}


	void ConfigurationWidget::setHeadingIcon(const QIcon & icon) {
		m_ui->headingLogo->setPixmap(icon.pixmap(HeadingLogoPixmapSize));
	}


	void ConfigurationWidget::setHeading(const QString & heading) {
		m_ui->headingTitle->setText(heading);
	}


	void ConfigurationWidget::clearAllFileExtensionMediaTypes() {
		QSignalBlocker block(m_ui->fileAssociations);
		m_ui->fileAssociations->clear();
	}


	void ConfigurationWidget::chooseDocumentRoot() {
		m_ui->serverDetails->chooseDocumentRoot();
	}


	void ConfigurationWidget::setListenAddress(const QString & addr) {
		eqAssert(m_server, "server must not be null");

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
		eqAssert(m_server, "server must not be null");
		m_ui->accessControl->setDefaultConnectionPolicy(policy);
	}


	void ConfigurationWidget::setDefaultMediaType(const QString & mediaType) {
		m_ui->fileAssociations->setDefaultMediaType(mediaType);
	}


	void ConfigurationWidget::setDefaultAction(WebServerAction action) {
		m_ui->mediaTypeActions->setDefaultAction(action);
	}


	void ConfigurationWidget::clearIpConnectionPolicies() {
		eqAssert(m_server, "server must not be null");
		m_ui->accessControl->clearAllConnectionPolicies();
		m_server->configuration().clearAllIpAddressConnectionPolicies();
	}


}  // namespace Anansi
