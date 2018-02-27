/** \file ConfigurationWidget.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the ConfigurationWidget class for EquitWebServer
  *
  * \todo setting listen address to invalid value leaves config with old listen
  *   address. so, e.g., setting it to 127.0.0.1, then changing it to "invalid"
  *   leaves the config with 127.0.0.1 but the display with "invalid". should
  *   probably attempt DNS lookup or insist on IPv4 formatted addresses rather
  *   than host names.
  *
  * \par Changes
  * - (2012-06-22) added widget to control "allow directory listings" config.
  * - (2012-06-22) action and mime association list columns stretch to an
  *   appropriate width on creation
  * - (2012-06-22) adding a MIME type action now tries to add an appropriate
  *   icon also.
  * - (2012-06-21) removed catch-all headers QtGui and QtNetwork in favour of
  *   individual headers as required to (hopefully) speed up compilation a
  *   little.
  * - (2012-06-21) added status icon to indicate whether document root path
  *   exists/is a directory/is readable.
  * - (2012-06-21) selectDocumentRoot() will now use whatever portion of the
  *   current document root path is valid as the initial directory in th
  *   dialogue. if it is entirely invalid, the user's home directory is used.
  * - (2012-06-19) changed populateAddressItems() to use new HostNetworkInfo
  *   class instead of QHostInfo as QHostInfo is not a reliable way of
  *   enumerating all network interface ip addresses.
  * - (2012-06-19) file documentation created.
  */

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

#include "serverdetailswidget.h"
#include "accesscontrolwidget.h"
#include "fileassociationswidget.h"
#include "mimetypeactionswidget.h"
#include "accesslogwidget.h"
#include "connectionpolicycombo.h"
#include "webserveractioncombo.h"
#include "mimetypecombo.h"
#include "mimeicons.h"


// TODO these are not yet used ...
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_OK QIcon::fromTheme("task-complete", QIcon(":/icons/status/ok")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING QIcon::fromTheme("task-attention", QIcon(":/icons/status/warning")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_ERROR QIcon::fromTheme("task-attention", QIcon(":/icons/status/error")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_UNKNOWN QIcon("/icons/status/unknown").pixmap(16)


Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);
Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);


namespace EquitWebServer {


	static constexpr const int WebServerActionRole = Qt::UserRole + 6392;


	ConfigurationWidget::ConfigurationWidget(QWidget * parent)
	: QWidget(parent),
	  m_server(nullptr),
	  m_ui(std::make_unique<Ui::ConfigurationWidget>()) {
		m_ui->setupUi(this);

		m_ui->splitter->setStretchFactor(0, 0);
		m_ui->splitter->setStretchFactor(1, 1);

		// server config slots
		connect(m_ui->serverDetails, &ServerDetailsWidget::documentRootChanged, [this](const QString & docRoot) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

			if(m_server->configuration().setDocumentRoot(docRoot)) {
				Q_EMIT documentRootChanged(docRoot);
			}
		});

		connect(m_ui->serverDetails, &ServerDetailsWidget::listenIpAddressChanged, [this](const QString & addr) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setListenAddress(addr);
		});

		connect(m_ui->serverDetails, &ServerDetailsWidget::listenPortChanged, [this](quint16 port) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setPort(port);
		});

		connect(m_ui->allowDirectoryListings, &QCheckBox::toggled, [this](bool allow) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_ui->allowDirectoryListings->setChecked(allow);
			m_server->configuration().setAllowDirectoryListing(allow);
		});
	}


	ConfigurationWidget::ConfigurationWidget(Server * server, QWidget * parent)
	: ConfigurationWidget(parent) {
		setServer(server);
	}


	ConfigurationWidget::~ConfigurationWidget() = default;


	void ConfigurationWidget::setServer(Server * server) {
		m_ui->fileAssociations->setServer(server);
		m_ui->mimeTypeActions->setServer(server);
		m_ui->accessControl->setServer(server);
		m_server = server;

		if(m_server) {
			for(const auto & mimeType : m_server->configuration().registeredMimeTypes()) {
				m_ui->fileAssociations->addAvailableMimeType(mimeType);
			}

			readConfiguration();

			// TODO these work as lambdas but not as directly-connected slots because strongly-typed enums
			// can't be queued as args for queued connections between threads. need to use Q_DECLARE_METATYPE
			// and qRegisterMetaType()
			//		connect(m_server, &Server::requestConnectionPolicyDetermined, m_ui->accessLog, &AccessLogWidget::addPolicyEntry);
			connect(m_server, &Server::requestConnectionPolicyDetermined, [this](const QString & addr, quint16 port, ConnectionPolicy policy) {
				m_ui->accessLog->addPolicyEntry(addr, port, policy);
			});
			//		connect(m_server, &Server::requestActionTaken, m_ui->accessLog, &AccessLogWidget::addActionEntry);
			connect(m_server, &Server::requestActionTaken, [this](const QString & addr, quint16 port, const QString & resource, WebServerAction action) {
				m_ui->accessLog->addActionEntry(addr, port, resource, action);
			});
		}
		else {
			setEnabled(false);
		}
	}


	void ConfigurationWidget::readConfiguration() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		std::array<QSignalBlocker, 6> blockers = {
		  {
			 QSignalBlocker(m_ui->serverDetails),
			 QSignalBlocker(m_ui->accessControl),
			 QSignalBlocker(m_ui->allowDirectoryListings),
			 QSignalBlocker(m_ui->fileAssociations),
			 QSignalBlocker(m_ui->mimeTypeActions),
			 QSignalBlocker(m_ui->accessLog),
		  }};

		const Configuration & opts = m_server->configuration();
		m_ui->serverDetails->setDocumentRoot(opts.documentRoot());
		m_ui->serverDetails->setListenIpAddress(opts.listenAddress());

		int port = opts.port();

		if(port >= 0 && port <= 65535) {
			m_ui->serverDetails->setListenPort(static_cast<uint16_t>(port));
		}
		else {
			m_ui->serverDetails->setListenPort(Configuration::DefaultPort);
		}

		m_ui->allowDirectoryListings->setChecked(opts.isDirectoryListingAllowed());

		//		m_ui->fileAssociations->update();
		//		m_ui->mimeTypeActions->update();
		//		m_ui->accessControl->update();

		setEnabled(true);
	}


	void ConfigurationWidget::clearAllActions() {
		m_ui->mimeTypeActions->clear();
	}


	void ConfigurationWidget::clearAllFileExtensionMIMETypes() {
		QSignalBlocker block(m_ui->fileAssociations);
		m_ui->fileAssociations->clear();
	}


	// TODO better names for these two - they intentionally only disable those widgets
	// that should not be available while the server is listening
	void ConfigurationWidget::disableWidgets() {
		m_ui->serverDetails->setEnabled(false);
	}


	void ConfigurationWidget::enableWidgets() {
		m_ui->serverDetails->setEnabled(true);
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
			m_ui->serverDetails->setListenIpAddress(addr);
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
			QMessageBox::critical(this, tr("Listen on host address"), tr("This computer does not appear to have any IPv4 addresses."));
			return;
		}

		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: binding to " << qPrintable(addr) << "\n";
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
		//		m_server->configuration().setDefaultConnectionPolicy(policy);
	}


	void ConfigurationWidget::setDefaultMimeType(const QString & mimeType) {
		m_ui->fileAssociations->setDefaultMimeType(mimeType);
	}


	void ConfigurationWidget::setDefaultAction(WebServerAction action) {
		m_ui->mimeTypeActions->setDefaultAction(action);
	}


	void ConfigurationWidget::clearIpConnectionPolicies() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_ui->accessControl->clearAllConnectionPolicies();
		m_server->configuration().clearAllIpAddressPolicies();
	}


}  // namespace EquitWebServer
