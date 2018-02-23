/** \file ConfigurationWidget.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the ConfigurationWidget class for EquitWebServer
  *
  * \todo content control tab is ugly and complex. Can the two trees be integrated
  *   by making everything hang off the MIME type? This would remove our ability
  *   to have a file extension associated with more than one MIME type, but is
  *   this ability really worth the complexity in the UI? So, it could be:
  *      MIME Type/Extensions  | Action  | CGI Program
  *     =======================|=========|==============
  *     - application/x-php    | CGI     | /usr/bin/php
  *       +- php
  *       +- php3
  *       +- php4
  *       +- php5
  *     - image/png            | Serve   |
  *       +- png
  *     - image/jpeg           | Serve   |
  *       +- jpg
  *       +- jpeg
  *   albeit this would involve no longer being able to have, for example, .php
  *   files served as text/plain if application/x-php is set to Forbid.
  *   Workaround? Have the ability to move extensions to other MIME types by
  *   drag-and-drop.
  * \todo icon engine won't scale images to create new size for MIME type icons in
  *   lists - check out what's going on here...
  * \todo setting listen address to invalid value leaves config with old listen
  *   address. so, e.g., setting it to 127.0.0.1, then changing it to "invalid"
  *   leaves the config with 127.0.0.1 but the display with "invalid". should
  *   probably attempt DNS lookup or insist on IPv4 formatted addresses rather
  *   than host names.
  * \todo saving configuration after changing document root does not save new
  *   document root until server is restarted (because document root does not
  *   get set in server options until server is restarted).
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
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QItemDelegate>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QNetworkInterface>

#include "iplistwidget.h"
#include "serverconfigwidget.h"
#include "accesscontrolwidget.h"
#include "fileassociationswidget.h"
#include "mimeactionswidget.h"
#include "accesslogwidget.h"
#include "connectionpolicycombo.h"
#include "webserveractioncombo.h"
#include "mimetypecombo.h"
#include "ipaddressconnectionpolicytreeitem.h"
#include "mimeicons.h"


// TODO these are not yet used ...
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_OK QIcon::fromTheme("task-complete", QIcon(":/icons/status/ok")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING QIcon::fromTheme("task-attention", QIcon(":/icons/status/warning")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_ERROR QIcon::fromTheme("task-attention", QIcon(":/icons/status/error")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_UNKNOWN QIcon("/icons/status/unknown").pixmap(16)


Q_DECLARE_METATYPE(EquitWebServer::Configuration::WebServerAction);
Q_DECLARE_METATYPE(EquitWebServer::Configuration::ConnectionPolicy);


namespace EquitWebServer {


	static constexpr const int WebServerActionRole = Qt::UserRole + 6392;


	ConfigurationWidget::ConfigurationWidget(Server * server, QWidget * parent)
	: QWidget(parent),
	  m_server(server),
	  m_serverConfig(new ServerConfigWidget),
	  m_accessConfig(new AccessControlWidget),
	  m_allowDirectoryListing(new QCheckBox(tr("Allow directory listings"))),
	  m_fileAssociations(new FileAssociationsWidget(server)),
	  m_mimeActions(new MimeActionsWidget(server)),
	  m_accessLog(new AccessLogWidget),
	  m_serverConfigTabs(new QTabWidget) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");

		// TODO these work as lambdas but not as directly-connected slots because strongly-typed enums
		// can't be queued as args for queued connections between threads. need to use Q_DECLARE_METATYPE
		// and qRegisterMetaType()
		//		connect(m_server, &Server::requestConnectionPolicyDetermined, m_accessLog, &AccessLogWidget::addPolicyEntry);
		connect(m_server, &Server::requestConnectionPolicyDetermined, [this](const QString & addr, quint16 port, Configuration::ConnectionPolicy policy) {
			m_accessLog->addPolicyEntry(addr, port, policy);
		});
		//		connect(m_server, &Server::requestActionTaken, m_accessLog, &AccessLogWidget::addActionEntry);
		connect(m_server, &Server::requestActionTaken, [this](const QString & addr, quint16 port, const QString & resource, Configuration::WebServerAction action) {
			m_accessLog->addActionEntry(addr, port, resource, action);
		});

		// server config slots
		connect(m_serverConfig, &ServerConfigWidget::documentRootChanged, [this](const QString & docRoot) {
			if(m_server->configuration().setDocumentRoot(docRoot)) {
				Q_EMIT documentRootChanged(docRoot);
			}
		});

		connect(m_serverConfig, &ServerConfigWidget::listenIpAddressChanged, [this](const QString & addr) {
			m_server->configuration().setListenAddress(addr);
		});

		connect(m_serverConfig, &ServerConfigWidget::listenPortChanged, [this](quint16 port) {
			m_server->configuration().setPort(port);
		});

		// access config slots
		connect(m_accessConfig, &AccessControlWidget::defaultConnectionPolicyChanged, [this](Configuration::ConnectionPolicy policy) {
			m_server->configuration().setDefaultConnectionPolicy(policy);
		});

		connect(m_accessConfig, &AccessControlWidget::ipAddressConnectionPolicySet, [this](const QString & addr, Configuration::ConnectionPolicy policy) {
			m_server->configuration().setIpAddressPolicy(addr, policy);
		});

		connect(m_accessConfig, &AccessControlWidget::ipAddressRemoved, [this](const QString & addr) {
			m_server->configuration().clearIpAddressPolicy(addr);
		});

		for(const auto & mimeType : m_server->configuration().registeredMimeTypes()) {
			m_fileAssociations->addAvailableMimeType(mimeType);
		}

		// file association slots
		connect(m_fileAssociations, &FileAssociationsWidget::defaultMimeTypeChanged, [this](const QString & mimeType) {
			// TODO this should be done in FileAssociationsWidget
			m_server->configuration().setDefaultMimeType(mimeType);
		});

		// mime actions slots
		connect(m_mimeActions, &MimeActionsWidget::defaultMimeTypeActionChanged, [this](Configuration::WebServerAction action) {
			// TODO this should be done in MimeActionsWidget
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: received defaultMimeTypeActionChanged() with action = " << static_cast<int>(action) << "\n"
						 << std::flush;
			m_server->configuration().setDefaultAction(action);
		});

		/* widgets on the content control tab page */
		auto * contentControlTabPage = new QWidget;
		auto * contentControlLayout = new QVBoxLayout;
		contentControlTabPage->setLayout(contentControlLayout);
		auto * contentControlSplitter = new QSplitter;
		m_fileAssociations->setToolTip(tr("<p>This section allows you to associate file extensions with MIME types.</p><p>When a request is received for a resource, this section determines which MIME type is used when processing the request and sending response data.</p>"));
		m_mimeActions->setToolTip(tr("<p>This section allows you to associate server actions with MIME types.</p><p>When a request is received for a resource, and its MIME type has been determined, this section defines what action the web server will take to generate the data for the response. The action can be:</p><ul><li><strong>Serve</strong> The resource (usually a file) will be sent verbatim</li><li><strong>Ignore</strong> The request will be ignored and no data will be sent</li><li><strong>Forbid</strong> The request will be rejected and a \"forbidden\" error response will be sent</li><li><strong>CGI</strong> The resource will be executed through the CGI environment and the output of the executed CGI command will be sent as the response. The CGI command to execute for a MIME type can be set by double-clicking the entry in the list; if no command is set, the resource is considered directly executable.</li></ul>"));
		contentControlSplitter->addWidget(m_fileAssociations);
		contentControlSplitter->addWidget(m_mimeActions);
		contentControlLayout->addWidget(m_allowDirectoryListing);
		contentControlLayout->addWidget(contentControlSplitter);

		m_serverConfigTabs->addTab(m_serverConfig, QIcon::fromTheme(QStringLiteral("network-server"), QIcon(QStringLiteral(":/icons/tabs/server"))), tr("Server"));
		m_serverConfigTabs->setTabToolTip(0, tr("The main server setup."));
		m_serverConfigTabs->addTab(m_accessConfig, QIcon::fromTheme(QStringLiteral("security-high"), QIcon(QStringLiteral(":/icons/tabs/accesscontrol"))), tr("Access Control"));
		m_serverConfigTabs->setTabToolTip(1, tr("Tell the server what to do with HTTP connections from different IP addresses."));
		m_serverConfigTabs->addTab(contentControlTabPage, QIcon::fromTheme(QStringLiteral("text-html"), QIcon(QStringLiteral(":/icons/tabs/contentcontrol"))), tr("Content Control"));
		m_serverConfigTabs->setTabToolTip(2, tr("Tell the server how to handle requests for different types of resources."));
		m_serverConfigTabs->addTab(m_accessLog, QIcon::fromTheme(QStringLiteral("text-x-log"), QIcon(QStringLiteral(":/icons/tabs/accesslog"))), tr("Access Log"));
		m_serverConfigTabs->setTabToolTip(3, tr("View the server access log."));

		auto * mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m_serverConfigTabs);

		connect(m_allowDirectoryListing, &QCheckBox::toggled, this, &ConfigurationWidget::setAllowDirectoryListing);

		readConfiguration();  // this ensures events are connected
		setLayout(mainLayout);
	}


	ConfigurationWidget::~ConfigurationWidget() = default;


	void ConfigurationWidget::setServer(Server * server) {
		m_fileAssociations->setServer(server);
		m_server = server;

		if(m_server) {
			readConfiguration();
		}
		else {
			setEnabled(false);
		}
	}


	void ConfigurationWidget::readConfiguration() {
		Q_ASSERT(m_server);

		std::array<QSignalBlocker, 6> blockers = {
		  {
			 QSignalBlocker(m_serverConfig),
			 QSignalBlocker(m_accessConfig),
			 QSignalBlocker(m_allowDirectoryListing),
			 QSignalBlocker(m_fileAssociations),
			 QSignalBlocker(m_mimeActions),
			 QSignalBlocker(m_accessLog),
		  }};

		const Configuration & opts = m_server->configuration();
		m_serverConfig->setDocumentRoot(opts.documentRoot());
		m_serverConfig->setListenIpAddress(opts.listenAddress());

		int port = opts.port();

		if(port >= 0 && port <= 65535) {
			m_serverConfig->setListenPort(static_cast<uint16_t>(port));
		}
		else {
			m_serverConfig->setListenPort(Configuration::DefaultPort);
		}

		// read ip policy configuration
		m_accessConfig->clearAllConnectionPolicies();

		for(const auto & ip : opts.registeredIpAddressList()) {
			m_accessConfig->setIpAddressConnectionPolicy(ip, opts.ipAddressPolicy(ip));
		}

		m_accessConfig->setDefaultConnectionPolicy(opts.defaultConnectionPolicy());
		m_allowDirectoryListing->setChecked(opts.isDirectoryListingAllowed());

		m_fileAssociations->update();
		m_mimeActions->update();

		setEnabled(true);
	}


	void ConfigurationWidget::clearAllActions() {
		m_mimeActions->clear();
	}


	void ConfigurationWidget::clearAllFileExtensionMIMETypes() {
		QSignalBlocker block(m_fileAssociations);
		m_fileAssociations->clear();
	}


	// TODO better names for these two - they intentionally only disable those widgets
	// that should not be available while the server is listening
	void ConfigurationWidget::disableWidgets() {
		m_serverConfig->setEnabled(false);
	}


	void ConfigurationWidget::enableWidgets() {
		m_serverConfig->setEnabled(true);
	}


	void ConfigurationWidget::chooseDocumentRoot() {
		m_serverConfig->chooseDocumentRoot();
	}


	///TODO connecting this to QLineEdit::textEdited() is a bit heavy-handed because it makes the server restart
	///on every keypress and/or executes several copy operations on the bpWebServer::Configuration object on every
	///keypress. migrate to getConfiguration() returning a pointer to the actual options, and/or change to use
	///the editingFinished() signal
	void ConfigurationWidget::setDocumentRoot(const QString & docRoot) {
		if(docRoot.isEmpty()) {
			return;
		}

		/* if we don't do this check, the widget will always be updated, which will mean
		 * that the cursor will be moved to the end of the widget on every keypress */
		if(m_serverConfig->documentRoot() != docRoot) {
			m_serverConfig->setDocumentRoot(docRoot);
		}

		m_server->configuration().setDocumentRoot(docRoot);
	}


	void ConfigurationWidget::setAllowDirectoryListing(bool allow) {
		m_allowDirectoryListing->setChecked(allow);
		Configuration & opts = m_server->configuration();
		opts.setAllowDirectoryListing(allow);
	}


	void ConfigurationWidget::setListenAddress(const QString & addr) {
		if(addr.isEmpty()) {
			return;
		}

		if(addr != m_serverConfig->listenIpAddress()) {
			m_serverConfig->setListenIpAddress(addr);
		}

		m_server->configuration().setListenAddress(addr);
	}


	void ConfigurationWidget::setListenPort(int port) {
		if(-1 == port) {
			port = Configuration::DefaultPort;
		}
		else if(0 > port || 65535 < port) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid port " << port << "\n";
			return;
		}

		m_serverConfig->setListenPort(static_cast<uint16_t>(port));
		m_server->configuration().setPort(port);
	}


	void ConfigurationWidget::bindToLocalhost() {
		setListenAddress(QStringLiteral("127.0.0.1"));
	}


	void ConfigurationWidget::bindToHostAddress() {
		QString addr;

		/* find first ipv4 address */
		for(const auto & hostAddress : QNetworkInterface::allAddresses()) {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: has address" << qPrintable(hostAddress.toString()) << "\n";

			if(hostAddress.isLoopback()) {
				continue;
			}

			if(QAbstractSocket::IPv4Protocol == hostAddress.protocol()) {
				addr = hostAddress.toString();
				break;
			}
		}

		if(addr.isNull()) {
			QMessageBox::critical(this, tr("Listen on host address"), tr("This computer does not appear to have any IPv4 addresses."));
			return;
		}

		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: binding to " << qPrintable(addr) << "\n";
		setListenAddress(addr);
	}


	void ConfigurationWidget::setLiberalDefaultConnectionPolicy() {
		setDefaultConnectionPolicy(Configuration::ConnectionPolicy::Accept);
	}


	void ConfigurationWidget::setRestrictiveDefaultConnectionPolicy() {
		setDefaultConnectionPolicy(Configuration::ConnectionPolicy::Reject);
	}


	void ConfigurationWidget::setDefaultConnectionPolicy(Configuration::ConnectionPolicy policy) {
		m_accessConfig->setDefaultConnectionPolicy(policy);
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: setting default connection policy directly.\n"
					 << std::flush;
		m_server->configuration().setDefaultConnectionPolicy(policy);
	}


	void ConfigurationWidget::setDefaultMimeType(const QString & mime) {
		m_server->configuration().setDefaultMimeType(mime);
	}


	//		void ConfigurationWidget::setMimeTypeAction(const QString & mime, Configuration::WebServerAction action) {
	//			m_mimeActions->setMimeTypeAction();
	//		}


	//	void ConfigurationWidget::removeAction(QTreeWidgetItem * it) {
	//		if(it) {
	//			QString mime = it->text(0);
	//			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Clearing action for MIME type '" << qPrintable(mime) << "'\n";
	//			Configuration & opts = m_server->configuration();
	//			opts.unsetMimeTypeAction(mime);
	//		}
	//		else {
	//			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Could not identify the MIME type from which to remove the action.\n";
	//		}
	//	}


	//	void ConfigurationWidget::removeExtensionMimeType(QTreeWidgetItem * it) {
	//		if(it) {
	//			QTreeWidgetItem * p = it->parent();
	//			QString ext, mime;

	//			if(p) {
	//				/* MIME type */
	//				ext = p->text(0);
	//				mime = it->text(0);
	//			}
	//			else {
	//				/* extension */
	//				ext = it->text(0);
	//			}

	//			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Clearing MIME type '" << qPrintable(mime) << "' from extension '" << qPrintable(ext) << "'.\n";
	//			Configuration & opts = m_server->configuration();
	//			opts.removeFileExtensionMimeType(ext, mime);
	//		}
	//		else {
	//			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Could not identify the extention and MIME type pair to remove.\n";
	//		}
	//	}


	//	void ConfigurationWidget::setDefaultAction(Configuration::WebServerAction action) {
	//		Configuration & opts = m_server->configuration();
	//		opts.setDefaultAction(action);
	//	}


	void ConfigurationWidget::setIpConnectionPolicy(const QString & ip, Configuration::ConnectionPolicy policy) {
		if(m_server->configuration().setIpAddressPolicy(ip, policy)) {
			m_accessConfig->setIpAddressConnectionPolicy(ip, policy);
			return;
		}

		QMessageBox::warning(this, tr("Set IP Address Connection Policy"), tr("The connection policy for '%1' could not be set.").arg(ip));
	}


	void ConfigurationWidget::ipPolicyRemoved(const QString & ip) {
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: " << qPrintable(ip) << "\n";
		m_server->configuration().clearIpAddressPolicy(ip);
	}


	void ConfigurationWidget::clearIpConnectionPolicies() {
		m_accessConfig->clearAllConnectionPolicies();
		m_server->configuration().clearAllIpAddressPolicies();
	}


}  // namespace EquitWebServer
