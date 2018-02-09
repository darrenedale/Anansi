#include "serverconfigwidget.h"
#include "ui_serverconfigwidget.h"

#include <iostream>

#include <QFileDialog>
#include <QRegularExpressionMatch>
#include <QIcon>

#include "hostnetworkinfo.h"
#include "configuration.h"


namespace EquitWebServer {


	static const QString UnknownStatusIcon = QStringLiteral(":/icons/status/unknown");
	static const QString ErrorStatusIcon = QStringLiteral(":/icons/status/error");
	static const QString OkStatusIcon = QStringLiteral(":/icons/status/ok");
	static const QString WarningStatusIcon = QStringLiteral(":/icons/status/warning");
	static constexpr const int MinimumStatusIconSize = 16;


	ServerConfigWidget::ServerConfigWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui{std::make_unique<Ui::ServerConfigWidget>()} {
		m_ui->setupUi(this);

		m_ui->docRootStatus->setVisible(false);
		connect(m_ui->docRoot, &QLineEdit::textEdited, this, &ServerConfigWidget::documentRootChanged);

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

		connect(m_ui->chooseDocRoot, &QToolButton::clicked, this, &ServerConfigWidget::chooseDocumentRoot);
		connect(m_ui->address, &QComboBox::editTextChanged, this, &ServerConfigWidget::listenIpAddressChanged);

		connect(m_ui->address, &QComboBox::editTextChanged, [this](const QString & addr) {
			static QRegularExpression ipAddressRx("^ *([0-9]{1,3})(?:\\.([0-9]{1,3})){3} *$");
			auto match = ipAddressRx.match(addr);

			if(!match.hasMatch()) {
				m_ui->addressStatus->setPixmap(QIcon(ErrorStatusIcon).pixmap(MinimumStatusIconSize));
				m_ui->addressStatus->setToolTip(tr("<p>This is not a valid IPv4 address in dotted-decimal format.</p><p><small>Addresses must be entered in dotted-decimal format (e.g. 192.168.0.1). Use <strong>127.0.0.1</strong> for <em>localhost</em></small>"));
				m_ui->addressStatus->setVisible(true);
				return;
			}

			uint32_t address = 0;

			for(int i = 1; i <= 4; ++i) {
				// rx match conditions guarantee this can't fail
				int byte = match.captured(i).toInt();

				if(255 < byte) {
					m_ui->addressStatus->setPixmap(QIcon(ErrorStatusIcon).pixmap(MinimumStatusIconSize));
					m_ui->addressStatus->setToolTip(tr("<p>This is not a valid dotted-decimal IPv4 address. Each part of the address must be between 0 and 255 inclusive.</p><p><small>Enter the address in dotted-decimal format (e.g. 192.168.0.1). Use <strong>127.0.0.1</strong> for <em>localhost</em></small>"));
					m_ui->addressStatus->setVisible(true);
					return;
				}

				address += static_cast<uint32_t>(byte << (8 * (4 - i)));
			}

			// TODO warn if IP is not one found on the machine

			// TODO check invalid ranges

			m_ui->addressStatus->setPixmap({});
			m_ui->addressStatus->setToolTip({});
			m_ui->addressStatus->setVisible(false);
		});

		connect(m_ui->port, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
			if(0 > value || 65535 < value) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid value " << value << " in port spin box\n";
				return;
			}

			Q_EMIT listenPortChanged(static_cast<uint16_t>(value));
		});

		repopulateLocalAddresses();
	}


	ServerConfigWidget::~ServerConfigWidget() = default;


	QString ServerConfigWidget::documentRoot() const {
		return m_ui->docRoot->text();
	}


	QString ServerConfigWidget::listenIpAddress() const {
		return m_ui->address->currentText();
	}


	quint16 ServerConfigWidget::listenPort() const {
		int port = m_ui->port->value();

		if(-1 == port) {
			return Configuration::DefaultPort;
		}

		return static_cast<uint16_t>(port);
	}


	void ServerConfigWidget::chooseDocumentRoot() {
		QString docRoot = QFileDialog::getExistingDirectory(this, tr("Choose the document root"), m_ui->docRoot->text());

		if(docRoot.isEmpty()) {
			return;
		}

		// TODO does this emit the signal?
		m_ui->docRoot->setText(docRoot);
	}


	void ServerConfigWidget::setDocumentRoot(const QString & docRoot) {
		m_ui->docRoot->setText(docRoot);
	}


	void ServerConfigWidget::setListenIpAddress(const QString & addr) {
		m_ui->address->setEditText(addr);
	}


	void ServerConfigWidget::setListenPort(uint16_t port) {
		m_ui->port->setValue(port);
	}


	void ServerConfigWidget::repopulateLocalAddresses() {
		m_ui->address->clear();

		/* for now, we only support ipv4 addresses */
		for(const QHostAddress & hostAddress : HostNetworkInfo::localHostAddresses(HostNetworkInfo::IPv4)) {
			m_ui->address->addItem(hostAddress.toString());
		}
	}

}  // namespace EquitWebServer
