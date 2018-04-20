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

/// \file serverdetailswidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the ServerDetailsWidget class.
///
/// \dep
/// - serverdetailswidget.h
/// - serverdetailswidget.ui
/// - <cstdint>
/// - <array>
/// - <iostream>
/// - <QtEndian>
/// - <QIcon>
/// - <QRegularExpression>
/// - <QRegularExpressionMatch>
/// - <QFileInfo>
/// - <QFileDialog>
/// - <QHostAddress>
/// - <QNetworkInterface>
/// - <QAbstractSocket>
/// - types.h
/// - server.h
/// - configuration.h
/// - strings.h
/// - notifications.h
///
/// \par Changes
/// - (2018-03) First release.

#include "serverdetailswidget.h"
#include "ui_serverdetailswidget.h"

#include <cstdint>
#include <array>
#include <iostream>

#include <QtEndian>
#include <QIcon>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFileInfo>
#include <QFileDialog>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>

#include "types.h"
#include "server.h"
#include "configuration.h"
#include "strings.h"
#include "notifications.h"


namespace Anansi {


	using Equit::starts_with;


	static const QString UnknownStatusIcon = QStringLiteral(":/icons/status/unknown");
	static const QString ErrorStatusIcon = QStringLiteral(":/icons/status/error");
	static const QString OkStatusIcon = QStringLiteral(":/icons/status/ok");
	static const QString WarningStatusIcon = QStringLiteral(":/icons/status/warning");
	static constexpr const int MinimumStatusIconSize = 16;

	// 192.168.0.0/16 (Private network, RFC 1918)
	static constexpr const uint32_t PrivateClassCNetworks = 0xc0a80000;
	static constexpr const int PrivateClassCNetmask = 16;

	// 172.16.0.0/12 (Private network, RFC 1918)
	static constexpr const uint32_t PrivateClassBNetworks = 0xac100000;
	static constexpr const int PrivateClassBNetmask = 12;

	// 10.0.0.0/8 (Private network, RFC 1918)
	static constexpr const uint32_t PrivateClassANetwork = 0x0a000000;
	static constexpr const int PrivateClassANetmask = 8;

	// 100.64.0.0/10 (carrier-grade NAT, RFC 6598)
	static constexpr const uint32_t CarrierGradeNATNetwork = 0x64400000;
	static constexpr const int CarrierGradeNATNetmask = 10;

	// 192.0.0.0/24 (IETF protocol assignments, RFC 6890)
	static constexpr const uint32_t IanaProtocolAssignmentsNetwork = 0xc0000000;
	static constexpr const int IanaProtocolAssignmentsNetmask = 24;

	// 192.0.2.0/24 (TEST-NET-1, RFC 5737)
	static constexpr const uint32_t IanaTest1Network = 0xc0000200;
	static constexpr const int IanaTest1Netmask = 24;

	// 198.51.100.0/24 (TEST-NET-2, RFC 5737)
	static constexpr const uint32_t IanaTest2Network = 0xc6336400;
	static constexpr const int IanaTest2Netmask = 24;

	// 203.0.113.0/24 (TEST-NET-3, RFC 5737)
	static constexpr const uint32_t IanaTest3Network = 0xcb007100;
	static constexpr const int IanaTest3Netmask = 24;

	// 198.18.0.0/15 (Network benchmark tests, RFC 2544)
	static constexpr const uint32_t IanaEquipmentTestNetwork = 0xc6120000;
	static constexpr const int IanaEquipmentTestNetmask = 15;

	// 240.0.0.0/4 (reserved (former class E network), RFC 1700)
	static constexpr const uint32_t ReservedExClassENetwork = 0xf0000000;
	static constexpr const int ReservedExClassENetmask = 4;

	// 192.88.99.0/24 (IPv6 to IPv4 relay, RFC 3068)
	static constexpr const uint32_t Ip6to4Network = 0xc0586300;
	static constexpr const int Ip6to4Netmask = 24;

	// 224.0.0.0/4 (IP multicast, RFC 5771)
	static constexpr const uint32_t MulticastNetwork = 0xe0000000;
	static constexpr const int MulticastNetmask = 4;

	// 255.255.255.255 (broadcast address)
	static constexpr const uint32_t BroadcastAddress = 0xffffffff;


	ServerDetailsWidget::ServerDetailsWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui{std::make_unique<Ui::ServerDetailsWidget>()},
	  m_server(nullptr) {
		m_ui->setupUi(this);

		m_ui->docRoot->setPathType(FilesystemPathWidget::PathType::ExistingDirectory);
		m_ui->docRoot->setDialogueCaption(tr("Choose document root"));
		m_ui->docRoot->setPlaceholderText(tr("Enter document root..."));

		m_ui->cgiBin->setPathType(FilesystemPathWidget::PathType::ExistingDirectory);
		m_ui->cgiBin->setDialogueCaption(tr("Choose the cgi-bin path"));
		m_ui->cgiBin->setPlaceholderText(tr("CGI executable path..."));

		m_ui->address->lineEdit()->setClearButtonEnabled(true);

		// can't do this in UI designer as I don't know how to set tab index for promoted
		// widgets (suspect it's not possible)
		setTabOrder(m_ui->docRoot, m_ui->address);
		setTabOrder(m_ui->address, m_ui->port);
		setTabOrder(m_ui->port, m_ui->cgiBin);
		setTabOrder(m_ui->cgiBin, m_ui->serverAdmin);

		connect(m_ui->docRoot, &FilesystemPathWidget::pathChanged, [this]() {
			eqAssert(m_server, "server cannot be null");
			const auto docRoot = m_ui->docRoot->path();

			if(!m_server->configuration().setDocumentRoot(docRoot)) {
				showNotification(this, tr("<p>The document root could not be set to <strong>%1</strong>.</p>").arg(docRoot), NotificationType::Error);
			}

			Q_EMIT documentRootChanged(docRoot);
		});

		// this and textChanged lambda for cgi-bin are very similar. consider templating?
		connect(m_ui->docRoot, &FilesystemPathWidget::textChanged, [this](const QString & docRoot) {
			QFileInfo docRootInfo(docRoot);

			auto setDocRootStatus = [this](const QString & msg, const QIcon & icon, bool visible = true) {
				m_ui->docRootStatus->setPixmap(icon.pixmap(MinimumStatusIconSize));
				m_ui->docRootStatus->setToolTip(msg);
				m_ui->docRootStatus->setVisible(visible);
			};

			if(!docRootInfo.exists()) {
				setDocRootStatus(tr("The path set for the document root does not exist."), QIcon(WarningStatusIcon));
				return;
			}

			if(!docRootInfo.isDir()) {
				setDocRootStatus(tr("The path set for the document root is not a directory.."), QIcon(WarningStatusIcon));
				return;
			}

			if(!docRootInfo.isReadable()) {
				setDocRootStatus(tr("The path set for the document root is not readable."), QIcon(WarningStatusIcon));
				return;
			}

			setDocRootStatus({}, {}, false);
		});

		connect(m_ui->serverAdmin, &QLineEdit::editingFinished, [this]() {
			eqAssert(m_server, "server cannot be null");
			const auto adminEmail = m_ui->serverAdmin->text();
			m_server->configuration().setAdministratorEmail(adminEmail);
			Q_EMIT administratorEmailChanged(adminEmail);
		});

		connect(m_ui->cgiBin, &FilesystemPathWidget::pathChanged, [this]() {
			eqAssert(m_server, "server cannot be null");
			const auto cgiBin = m_ui->cgiBin->path();

			if(!m_server->configuration().setCgiBin(cgiBin)) {
				showNotification(this, tr("<p>The cgi-bin directory could not be set to <strong>%1</strong>.</p>").arg(cgiBin), NotificationType::Error);
			}
			else {
				auto cgiBinInfo = QFileInfo(cgiBin);
				auto docRootInfo = QFileInfo(m_server->configuration().documentRoot());

				// if path does not exist, absoluteFilePath() returns empty which could result
				// in false positives
				if(cgiBinInfo.exists() && docRootInfo.exists() && starts_with(cgiBinInfo.absoluteFilePath(), docRootInfo.absoluteFilePath())) {
					showNotification(this, tr("<p>The cgi-bin directory is inside the document root.</p><p><small>This can be a security risk in some circumstances.</small></p>"), NotificationType::Warning);
				}
			}

			// NEXTRELEASE warn if system program location (e.g. /usr/bin, C:\Program Files)
			Q_EMIT cgiBinChanged(cgiBin);
		});

		connect(m_ui->cgiBin, &FilesystemPathWidget::textChanged, [this](const QString & cgiBin) {
			QFileInfo cgiBinInfo(cgiBin);

			auto setCgiBinStatus = [this](const QString & msg, const QIcon & icon, bool visible = true) {
				m_ui->cgiBinStatus->setPixmap(icon.pixmap(MinimumStatusIconSize));
				m_ui->cgiBinStatus->setToolTip(msg);
				m_ui->cgiBinStatus->setVisible(visible);
			};

			if(!cgiBinInfo.exists()) {
				setCgiBinStatus(tr("The path set for the CGI bin directory does not exist."), QIcon(WarningStatusIcon));
				return;
			}

			if(!cgiBinInfo.isDir()) {
				setCgiBinStatus(tr("The path set for the CGI bin directory is not a directory.."), QIcon(WarningStatusIcon));
				return;
			}

			if(!cgiBinInfo.isReadable()) {
				setCgiBinStatus(tr("The path set for the CGI bin directory is not readable."), QIcon(WarningStatusIcon));
				return;
			}

			setCgiBinStatus({}, {}, false);
		});

		connect(m_ui->address->lineEdit(), &QLineEdit::editingFinished, [this]() {
			eqAssert(m_server, "server cannot be null");
			static QRegularExpression ipAddressRx(R"(^ *([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3}) *$)");

			auto showIpValidationNotification = [this](const QString & msg, const QIcon & icon = {}, NotificationType type = NotificationType::Warning) {
				showNotification(this, msg, type);
				m_ui->addressStatus->setPixmap(icon.pixmap(MinimumStatusIconSize));
				m_ui->addressStatus->setToolTip(msg);
				m_ui->addressStatus->setVisible(true);
			};

			const auto addr = m_ui->address->currentText();
			const auto match = ipAddressRx.match(addr);

			if(!match.hasMatch()) {
				showIpValidationNotification(tr("<p>This is not a valid IPv4 address in dotted-decimal format.</p><p><small>Addresses must be entered in dotted-decimal format (e.g. 192.168.0.1). Use <strong>127.0.0.1</strong> for <em>localhost</em></small>"), QIcon(ErrorStatusIcon));
				return;
			}

			std::array<uint8_t, 4> addressBytes;
			uint32_t & addressInt = *(reinterpret_cast<uint32_t *>(&addressBytes[0]));

			for(int i = 1; i <= 4; ++i) {
				// rx match conditions guarantee this can't fail
				uint32_t byte = static_cast<uint32_t>(match.captured(i).toInt());

				if(255 < byte) {
					showIpValidationNotification(tr("<p>This is not a valid dotted-decimal IPv4 address. Each part of the address must be between 0 and 255 inclusive.</p><p><small>Enter the address in dotted-decimal format (e.g. 192.168.0.1). Use <strong>127.0.0.1</strong> for <em>localhost</em></small>"), QIcon(ErrorStatusIcon));
					return;
				}

				addressBytes[static_cast<unsigned>(i - 1)] = static_cast<uint8_t>(byte);
			}

			QHostAddress address(qToBigEndian(addressInt));

			if(!address.isLoopback()) {
				if(!QNetworkInterface::allAddresses().contains(address)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> does not appear to belong to this device.</p><p><small>Attempting to start the server listening on this address is unlikely to succeed.</small></p>").arg(addr));
				}
				else if(0 == addressBytes[0]) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is only valid as a source address.</p> <p><small>Attempting to start the server listening on this address is unlikely to succeed.</small></p>").arg(addr));
				}
				else if(!address.isInSubnet(QHostAddress(PrivateClassCNetworks), PrivateClassCNetmask) &&
						  !address.isInSubnet(QHostAddress(PrivateClassBNetworks), PrivateClassBNetmask) &&
						  !address.isInSubnet(QHostAddress(PrivateClassANetwork), PrivateClassANetmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is not in a private subnet.</p> <p>Starting the server listening on this address is <strong>likely to expose the server to the internet which is a security risk</strong>.</p>").arg(addr));
				}
				else if(address.isInSubnet(QHostAddress(CarrierGradeNATNetwork), CarrierGradeNATNetmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in the range reserved for <em>carrier-grade NAT</em>.</p><p><small>Attempting to start the server listening on this address is very unlikely to succeed.</small></p>").arg(addr));
				}
				else if(address.isInSubnet(QHostAddress(IanaProtocolAssignmentsNetwork), IanaProtocolAssignmentsNetmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in the range reserved for <em>IANA protocol assignments</em>.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
				}
				else if(address.isInSubnet(QHostAddress(IanaTest1Network), IanaTest1Netmask) ||
						  address.isInSubnet(QHostAddress(IanaTest2Network), IanaTest2Netmask) ||
						  address.isInSubnet(QHostAddress(IanaTest3Network), IanaTest3Netmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in the range reserved for <em>testing and documentation only</em> and are considered non-routable addresses.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
				}
				else if(address.isInSubnet(QHostAddress(IanaEquipmentTestNetwork), IanaEquipmentTestNetmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in the range reserved for testing network devices.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
				}
				else if(address.isInSubnet(QHostAddress(Ip6to4Network), Ip6to4Netmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in the range reserved for routing IPv6 traffic over IPv4 networks.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
				}
				else if(address.isInSubnet(QHostAddress(Ip6to4Network), Ip6to4Netmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in the range reserved for routing IPv6 traffic over IPv4 networks.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
				}
				else if(address.isInSubnet(QHostAddress(MulticastNetwork), MulticastNetmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in the range reserved for IPv4 multicast assignments.</p><p><small>You are very unlikely to have an IP address in this range assigned to your computer and in any case running a standard web server on such an address is contrary to their purpose. Attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
				}
				else if(address == QHostAddress(BroadcastAddress)) {
					showIpValidationNotification(tr("<p>The IP address <strong>255.255.255.255</strong> is the broadcast address and cannot be bound to.</p><p><small>It is not possible to have this IP address assigned to your computer and attempting to listen on it will fail.</small></p>"));
				}
				else if(address.isInSubnet(QHostAddress(ReservedExClassENetwork), ReservedExClassENetmask)) {
					showIpValidationNotification(tr("<p>The IP address <strong>%1</strong> is in a reserved range.</p><p><small>Attempting to start the server listening on this address is very unlikely to succeed.</small></p>").arg(addr));
				}
			}

			auto & config = m_server->configuration();

			if(!config.setListenAddress(addr)) {
				showNotification(this, tr("<p>The listen address could not be set to <strong>%1</strong>.</p><p><small>This is likely because it's not a valid dotted-decimal IPv4 address.</small></p>").arg(addr), NotificationType::Error);
				m_ui->address->setCurrentText(config.listenAddress());
				return;
			}

			m_ui->addressStatus->setPixmap({});
			m_ui->addressStatus->setToolTip({});
			m_ui->addressStatus->setVisible(false);

			if(m_server->isListening()) {
				showNotification(this, tr("<p>The listen address was changed while the server was running. This will not take effect until the server is restarted.</p><p><small>The server will continue to listen on the previous address until it is restarted.</small></p>"), NotificationType::Warning);
			}

			Q_EMIT listenIpAddressChanged(addr);
		});

		connect(m_ui->port, &QSpinBox::editingFinished, [this]() {
			eqAssert(m_server, "server must not be null");
			const auto port = m_ui->port->value();
			auto & config = m_server->configuration();

			if(!m_server->configuration().setPort(port)) {
				showNotification(this, tr("<p>The listen port could not be set to <strong>%1</strong>.</p><p><small>The port must be between 1 and 65535.</small></p>").arg(port), NotificationType::Error);
				auto oldPort = config.port();

				if(-1 == oldPort) {
					m_ui->port->setValue(Configuration::DefaultPort);
				}
				else {
					m_ui->port->setValue(static_cast<uint16_t>(oldPort));
				}

				return;
			}

			if(m_server->isListening()) {
				showNotification(this, tr("<p>The listen port was changed while the server was running. This will not take effect until the server is restarted.</p><p><small>The server will continue to listen on the previous port until it is restarted.</small></p>"), NotificationType::Warning);
			}

			Q_EMIT listenPortChanged(static_cast<uint16_t>(port));
		});

		clearStatuses();
		repopulateLocalAddresses();
	}


	ServerDetailsWidget::ServerDetailsWidget(Server * server, QWidget * parent)
	: ServerDetailsWidget(parent) {
		setServer(server);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	ServerDetailsWidget::~ServerDetailsWidget() = default;


	void ServerDetailsWidget::setServer(Server * server) {
		std::array<QSignalBlocker, 5> blocks = {{QSignalBlocker(m_ui->address), QSignalBlocker(m_ui->cgiBin), QSignalBlocker(m_ui->docRoot), QSignalBlocker(m_ui->port), QSignalBlocker(m_ui->serverAdmin)}};
		m_server = server;

		if(!server) {
			m_ui->docRoot->setPath(QStringLiteral(""));
			m_ui->address->setCurrentText(QStringLiteral(""));
			m_ui->port->setValue(Configuration::DefaultPort);
			m_ui->cgiBin->setPath(QStringLiteral(""));
			m_ui->serverAdmin->setText(QStringLiteral(""));
		}
		else {
			const auto & config = server->configuration();
			m_ui->docRoot->setPath(config.documentRoot());
			m_ui->address->setCurrentText(config.listenAddress());
			m_ui->port->setValue(config.port());
			m_ui->cgiBin->setPath(config.cgiBin());
			m_ui->serverAdmin->setText(config.administratorEmail());
		}

		clearStatuses();
	}


	QString ServerDetailsWidget::documentRoot() const {
		return m_ui->docRoot->path();
	}


	QString ServerDetailsWidget::listenIpAddress() const {
		return m_ui->address->currentText();
	}


	uint16_t ServerDetailsWidget::listenPort() const {
		int port = m_ui->port->value();

		if(-1 == port) {
			return Configuration::DefaultPort;
		}

		return static_cast<uint16_t>(port);
	}


	QString ServerDetailsWidget::administratorEmail() const {
		return m_ui->serverAdmin->text();
	}


	QString ServerDetailsWidget::cgiBin() const {
		return m_ui->cgiBin->path();
	}


	void ServerDetailsWidget::chooseDocumentRoot() {
		m_ui->docRoot->choosePath();
	}


	void ServerDetailsWidget::setDocumentRoot(const QString & docRoot) {
		m_ui->docRoot->setPath(docRoot);
		Q_EMIT documentRootChanged(docRoot);
	}


	void ServerDetailsWidget::setListenAddress(const QString & addr) {
		m_ui->address->setEditText(addr);
		Q_EMIT listenIpAddressChanged(addr);
	}


	void ServerDetailsWidget::setListenPort(uint16_t port) {
		m_ui->port->setValue(port);
		Q_EMIT listenPortChanged(port);
	}


	void ServerDetailsWidget::setAdministratorEmail(const QString & adminEmail) {
		m_ui->serverAdmin->setText(adminEmail);
		Q_EMIT administratorEmailChanged(adminEmail);
	}


	void ServerDetailsWidget::chooseCgiBin() {
		m_ui->cgiBin->choosePath();
	}


	void ServerDetailsWidget::setCgiBin(const QString & cgiBin) {
		m_ui->cgiBin->setPath(cgiBin);
		Q_EMIT cgiBinChanged(cgiBin);
	}


	void ServerDetailsWidget::repopulateLocalAddresses() {
		m_ui->address->clear();

		for(const auto & hostAddress : QNetworkInterface::allAddresses()) {
			if(QAbstractSocket::IPv4Protocol == hostAddress.protocol()) {
				m_ui->address->addItem(hostAddress.toString());
			}
		}
	}


	void ServerDetailsWidget::clearStatuses() {
		for(auto * statusLabel : {m_ui->addressStatus, m_ui->docRootStatus, m_ui->cgiBinStatus}) {
			statusLabel->setPixmap({});
			statusLabel->setToolTip({});
			statusLabel->setVisible(false);
		}
	}

}  // namespace Anansi
