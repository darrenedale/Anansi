/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file serverdetailswidget.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the ServerDetailsWidget class.
///
/// \dep
/// - <iostream>
/// - <QtEndian>
/// - <QFileDialog>
/// - <QRegularExpressionMatch>
/// - <QIcon>
/// - <QNetworkInterface>
/// - <QMessageBox>
/// - window.h
/// - configuration.h
///
/// \par Changes
/// - (2018-03) First release.

#include "serverdetailswidget.h"
#include "ui_serverdetailswidget.h"

#include <iostream>

#include <QtEndian>
#include <QFileDialog>
#include <QRegularExpressionMatch>
#include <QIcon>
#include <QNetworkInterface>
#include <QMessageBox>

#include "window.h"
#include "configuration.h"
#include "notifications.h"


namespace Anansi {


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
	  m_ui{std::make_unique<Ui::ServerDetailsWidget>()} {
		m_ui->setupUi(this);

		m_ui->docRootStatus->setVisible(false);

		connect(m_ui->docRoot, &QLineEdit::editingFinished, [this]() {
			Q_EMIT documentRootChanged(m_ui->docRoot->text());
		});

		connect(m_ui->serverAdmin, &QLineEdit::editingFinished, [this]() {
			Q_EMIT administratorEmailChanged(m_ui->serverAdmin->text());
		});

		connect(m_ui->cgiBin, &QLineEdit::editingFinished, [this]() {
			Q_EMIT cgiBinChanged(m_ui->cgiBin->text());
		});

		// this and textChanged lambda for cgi-bin are very similar. consider templating?
		connect(m_ui->docRoot, &QLineEdit::textChanged, [this](const QString & docRoot) {
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

		connect(m_ui->cgiBin, &QLineEdit::textChanged, [this](const QString & cgiBin) {
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

		connect(m_ui->chooseDocRoot, &QToolButton::clicked, this, &ServerDetailsWidget::chooseDocumentRoot);
		connect(m_ui->chooseCgiBin, &QToolButton::clicked, this, &ServerDetailsWidget::chooseCgiBin);

		connect(m_ui->address->lineEdit(), &QLineEdit::editingFinished, [this]() {
			static QRegularExpression ipAddressRx("^ *([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3}) *$");

			auto showError = [this](const QString & msg, const QIcon & icon = {}) {
				showNotification(this, msg, NotificationType::Warning);
				m_ui->addressStatus->setPixmap(icon.pixmap(MinimumStatusIconSize));
				m_ui->addressStatus->setToolTip(msg);
				m_ui->addressStatus->setVisible(true);
			};

			const auto addr = m_ui->address->currentText();
			Q_EMIT listenIpAddressChanged(addr);

			const auto match = ipAddressRx.match(addr);

			if(!match.hasMatch()) {
				showError(tr("<p>This is not a valid IPv4 address in dotted-decimal format.</p><p><small>Addresses must be entered in dotted-decimal format (e.g. 192.168.0.1). Use <strong>127.0.0.1</strong> for <em>localhost</em></small>"), QIcon(ErrorStatusIcon));
				return;
			}

			std::array<uint8_t, 4> addressBytes;
			uint32_t & addressInt = *reinterpret_cast<uint32_t *>(&addressBytes[0]);

			for(int i = 1; i <= 4; ++i) {
				// rx match conditions guarantee this can't fail
				uint32_t byte = static_cast<uint32_t>(match.captured(i).toInt());

				if(255 < byte) {
					showError(tr("<p>This is not a valid dotted-decimal IPv4 address. Each part of the address must be between 0 and 255 inclusive.</p><p><small>Enter the address in dotted-decimal format (e.g. 192.168.0.1). Use <strong>127.0.0.1</strong> for <em>localhost</em></small>"), QIcon(ErrorStatusIcon));
					return;
				}

				addressBytes[static_cast<unsigned>(i - 1)] = static_cast<uint8_t>(byte);
			}

			QHostAddress address(qToBigEndian(addressInt));

			if(!address.isLoopback()) {
				if(!QNetworkInterface::allAddresses().contains(address)) {
					showError(tr("<p>The IP address <strong>%1</strong> does not appear to belong to this device.</p><p><small>Attempting to start the server listening on this address is unlikely to succeed.</small></p>").arg(addr));
					return;
				}

				if(0 == addressBytes[0]) {
					showError(tr("<p>The IP address <strong>%1</strong> is only valid as a source address.</p> <p><small>Attempting to start the server listening on this address is unlikely to succeed.</small></p>").arg(addr));
					return;
				}

				if(!address.isInSubnet(QHostAddress(PrivateClassCNetworks), PrivateClassCNetmask) ||
					address.isInSubnet(QHostAddress(PrivateClassBNetworks), PrivateClassBNetmask) ||
					address.isInSubnet(QHostAddress(PrivateClassANetwork), PrivateClassANetmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is not in a private subnet.</p> <p>Starting the server listening on this address is <strong>likely to expose the server to the internet which is a security risk</strong>.</p>").arg(addr));
					return;
				}

				if(address.isInSubnet(QHostAddress(CarrierGradeNATNetwork), 10)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for <em>carrier-grade NAT</em>.</p><p><small>Attempting to start the server listening on this address is very unlikely to succeed.</small></p>").arg(addr));
					return;
				}

				if(address.isInSubnet(QHostAddress(IanaProtocolAssignmentsNetwork), IanaProtocolAssignmentsNetmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for <em>IANA protocol assignments</em>.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
					return;
				}

				if(address.isInSubnet(QHostAddress(IanaTest1Network), IanaTest1Netmask) ||
					address.isInSubnet(QHostAddress(IanaTest2Network), IanaTest2Netmask) ||
					address.isInSubnet(QHostAddress(IanaTest3Network), IanaTest3Netmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for <em>testing and documentation only</em> and are considered non-routable addresses.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
					return;
				}

				if(address.isInSubnet(QHostAddress(IanaEquipmentTestNetwork), IanaEquipmentTestNetmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for testing network devices.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
					return;
				}

				if(address.isInSubnet(QHostAddress(Ip6to4Network), Ip6to4Netmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for routing IPv6 traffic over IPv4 networks.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
					return;
				}

				if(address.isInSubnet(QHostAddress(Ip6to4Network), Ip6to4Netmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for routing IPv6 traffic over IPv4 networks.</p><p><small>You are unlikely to have an IP address in this range assigned to your computer so attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
					return;
				}

				if(address.isInSubnet(QHostAddress(MulticastNetwork), MulticastNetmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for IPv4 multicast assignments.</p><p><small>You are very unlikely to have an IP address in this range assigned to your computer and in any case running a standard web server on such an address is contrary to their purpose. Attempting to start the server listening on this address is likely to fail.</small></p>").arg(addr));
					return;
				}

				if(address == QHostAddress(BroadcastAddress)) {
					showError(tr("<p>The IP address <strong>255.255.255.255</strong> is the broadcast address and cannot be bound to.</p><p><small>It is not possible to have this IP address assigned to your computer and attempting to listen on it will fail.</small></p>"));
					return;
				}

				if(address.isInSubnet(QHostAddress(ReservedExClassENetwork), ReservedExClassENetmask)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in a reserved range.</p><p><small>Attempting to start the server listening on this address is very unlikely to succeed.</small></p>").arg(addr));
					return;
				}
			}

			for(auto * statusLabel : {m_ui->addressStatus, m_ui->docRootStatus, m_ui->cgiBinStatus}) {
				statusLabel->setPixmap({});
				statusLabel->setToolTip({});
				statusLabel->setVisible(false);
			}
		});

		connect(m_ui->port, &QSpinBox::editingFinished, [this]() {
			const auto value = m_ui->port->value();

			if(0 > value || 65535 < value) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid value " << value << " in port spin box\n";
				showNotification(this, tr("The listen port %1 is not valid. It must be in the range 0 to 65535 (inclusive).").arg(value), NotificationType::Warning);
				return;
			}

			Q_EMIT listenPortChanged(static_cast<uint16_t>(value));
		});

		repopulateLocalAddresses();
	}


	ServerDetailsWidget::~ServerDetailsWidget() = default;


	QString ServerDetailsWidget::documentRoot() const {
		return m_ui->docRoot->text();
	}


	QString ServerDetailsWidget::listenIpAddress() const {
		return m_ui->address->currentText();
	}


	quint16 ServerDetailsWidget::listenPort() const {
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
		return m_ui->cgiBin->text();
	}


	void ServerDetailsWidget::chooseDocumentRoot() {
		QString docRoot = QFileDialog::getExistingDirectory(this, tr("Choose the document root"), m_ui->docRoot->text());

		if(docRoot.isEmpty()) {
			return;
		}

		m_ui->docRoot->setText(docRoot);
	}


	void ServerDetailsWidget::setDocumentRoot(const QString & docRoot) {
		m_ui->docRoot->setText(docRoot);
	}


	void ServerDetailsWidget::setListenAddress(const QString & addr) {
		m_ui->address->setEditText(addr);
	}


	void ServerDetailsWidget::setListenPort(uint16_t port) {
		m_ui->port->setValue(port);
	}


	void ServerDetailsWidget::setAdministratorEmail(const QString & adminEmail) {
		m_ui->serverAdmin->setText(adminEmail);
	}


	void ServerDetailsWidget::chooseCgiBin() {
		QString cgiBin = QFileDialog::getExistingDirectory(this, tr("Choose the cgi-bin path"), m_ui->docRoot->text());

		if(cgiBin.isEmpty()) {
			return;
		}

		m_ui->cgiBin->setText(cgiBin);
	}


	void ServerDetailsWidget::setCgiBin(const QString & cgiBin) {
		m_ui->cgiBin->setText(cgiBin);
	}


	void ServerDetailsWidget::repopulateLocalAddresses() {
		m_ui->address->clear();

		for(const auto & hostAddress : QNetworkInterface::allAddresses()) {
			if(QAbstractSocket::IPv4Protocol == hostAddress.protocol()) {
				m_ui->address->addItem(hostAddress.toString());
			}
		}
	}

}  // namespace Anansi
