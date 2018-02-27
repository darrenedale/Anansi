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


namespace EquitWebServer {


	static const QString UnknownStatusIcon = QStringLiteral(":/icons/status/unknown");
	static const QString ErrorStatusIcon = QStringLiteral(":/icons/status/error");
	static const QString OkStatusIcon = QStringLiteral(":/icons/status/ok");
	static const QString WarningStatusIcon = QStringLiteral(":/icons/status/warning");
	static constexpr const int MinimumStatusIconSize = 16;


	ServerDetailsWidget::ServerDetailsWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui{std::make_unique<Ui::ServerDetailsWidget>()} {
		m_ui->setupUi(this);

		m_ui->docRootStatus->setVisible(false);
		connect(m_ui->docRoot, &QLineEdit::textEdited, this, &ServerDetailsWidget::documentRootChanged);

		connect(m_ui->docRoot, &QLineEdit::textChanged, [this](const QString & docRoot) {
			QFileInfo docRootInfo(docRoot);

			if(!docRootInfo.exists()) {
				m_ui->docRootStatus->setPixmap(QIcon(WarningStatusIcon).pixmap(MinimumStatusIconSize));
				m_ui->docRootStatus->setToolTip(tr("The path set for the document root does not exist."));
				m_ui->docRootStatus->setVisible(true);
				return;
			}

			if(!docRootInfo.isDir()) {
				m_ui->docRootStatus->setPixmap(QIcon(WarningStatusIcon).pixmap(MinimumStatusIconSize));
				m_ui->docRootStatus->setToolTip(tr("The path set for the document root is not a directory."));
				m_ui->docRootStatus->setVisible(true);
				return;
			}

			if(!docRootInfo.isReadable()) {
				m_ui->docRootStatus->setPixmap(QIcon(WarningStatusIcon).pixmap(MinimumStatusIconSize));
				m_ui->docRootStatus->setToolTip(tr("The path set for the document root is not readable."));
				m_ui->docRootStatus->setVisible(true);
				return;
			}

			m_ui->docRootStatus->setPixmap({});
			m_ui->docRootStatus->setToolTip({});
			m_ui->docRootStatus->setVisible(false);
		});

		connect(m_ui->chooseDocRoot, &QToolButton::clicked, this, &ServerDetailsWidget::chooseDocumentRoot);

		connect(m_ui->address->lineEdit(), &QLineEdit::editingFinished, [this]() {
			static QRegularExpression ipAddressRx("^ *([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3}) *$");

			auto showError = [this](const QString & msg, const QIcon & icon = {}) {
				auto * win = qobject_cast<Window *>(window());

				if(!win) {
					QMessageBox::warning(this, tr("Set listen address"), msg);
				}
				else {
					win->showInlineNotification(msg, NotificationType::Warning);
				}

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
				// warn if IP is not one found on the machine
				if(!QNetworkInterface::allAddresses().contains(address)) {
					showError(tr("<p>The IP address <strong>%1</strong> does not appear to belong to this device.</p><p><small>Attempting to start the server listening on this address is unlikely to succeed.</small></p>").arg(addr));
					return;
				}

				if(0 == addressBytes[0]) {
					showError(tr("<p>The IP address <strong>%1</strong> is only valid as a source address.</p> <p><small>Attempting to start the server listening on this address is unlikely to succeed.</small></p>").arg(addr));
					return;
				}

				// warn if IP is not on a private subnet
				auto isPrivateSubnet = address.isInSubnet(QHostAddress(3232235520), 16) ||  // 192.168.0.0/16
											  address.isInSubnet(QHostAddress(2886729728), 12) ||  // 172.16.0.0/12
											  address.isInSubnet(QHostAddress(167772160), 8);		 // 10.0.0.0/8

				if(!isPrivateSubnet) {
					showError(tr("<p>The IP address <strong>%1</strong> is not in a private subnet.</p> <p>Starting the server listening on this address is <strong>likely to expose the server to the internet which is a security risk</strong>.</p>").arg(addr));
					return;
				}

				// carrier-grade NAT: 100.64.0.0/10
				if(address.isInSubnet(QHostAddress(1681915904), 10)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in the range reserved for <em>carrier-grade NAT</em>.</p><p><small>Attempting to start the server listening on this address is very unlikely to succeed.</small></p>").arg(addr));
					return;
				}

				// TODO determine the status of the following special ranges:
				// - 192.0.0.0/24 (IETF protocol assignments, RFC 6890)
				// - 192.0.2.0/24 (TEST-NET-1, RFC 5737)
				// - 192.88.99.0/24	(IPv6 to IPv4 relay, RFC 3068)
				// - 198.18.0.0/15 (Network benchmark tests, RFC 2544)
				// - 198.51.100.0/24 (TEST-NET-2, RFC 5737)
				// - 203.0.113.0/24 (TEST-NET-3, RFC 5737)
				// - 224.0.0.0/4 (IP multicast, RFC 5771)
				// - 255.255.255.255/32 (broadcast, RFC 919)

				// other reserved: 240.0.0.0/4
				if(address.isInSubnet(QHostAddress(4026531840), 4)) {
					showError(tr("<p>The IP address <strong>%1</strong> is in a reserved range.</p><p><small>Attempting to start the server listening on this address is very unlikely to succeed.</small></p>").arg(addr));
					return;
				}
			}

			m_ui->addressStatus->setPixmap({});
			m_ui->addressStatus->setToolTip({});
			m_ui->addressStatus->setVisible(false);
		});

		connect(m_ui->port, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
			if(0 > value || 65535 < value) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid value " << value << " in port spin box\n";
				auto * win = qobject_cast<Window *>(window());
				auto msg = tr("The listen port %1 is not valid. It must be in the range 0 to 65535 (inclusive).").arg(value);

				if(!win) {
					QMessageBox::warning(this, tr("Set listen port"), msg);
				}
				else {
					win->showInlineNotification(msg, NotificationType::Warning);
				}

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


	void ServerDetailsWidget::setListenIpAddress(const QString & addr) {
		m_ui->address->setEditText(addr);
	}


	void ServerDetailsWidget::setListenPort(uint16_t port) {
		m_ui->port->setValue(port);
	}


	void ServerDetailsWidget::repopulateLocalAddresses() {
		m_ui->address->clear();

		// for now, we only support ipv4 addresses
		for(const auto & hostAddress : QNetworkInterface::allAddresses()) {
			if(QAbstractSocket::IPv4Protocol == hostAddress.protocol()) {
				m_ui->address->addItem(hostAddress.toString());
			}
		}
	}

}  // namespace EquitWebServer
