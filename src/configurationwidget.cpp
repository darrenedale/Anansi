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

#include "iplistwidget.h"
#include "serverdetailswidget.h"
#include "accesscontrolwidget.h"
#include "fileassociationswidget.h"
#include "mimetypeactionswidget.h"
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


Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);
Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);


namespace EquitWebServer {


	static constexpr const int WebServerActionRole = Qt::UserRole + 6392;


	ConfigurationWidget::ConfigurationWidget(QWidget * parent)
	: QWidget(parent),
	  m_server(nullptr),
	  m_ui(std::make_unique<Ui::ConfigurationWidget>()) {
		m_ui->setupUi(this);
	}


	ConfigurationWidget::ConfigurationWidget(Server * server, QWidget * parent)
	: ConfigurationWidget(parent) {
		setServer(server);

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

		// access config slots
		connect(m_ui->accessControl, &AccessControlWidget::defaultConnectionPolicyChanged, [this](ConnectionPolicy policy) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setDefaultConnectionPolicy(policy);
		});

		connect(m_ui->accessControl, &AccessControlWidget::ipAddressConnectionPolicySet, [this](const QString & addr, ConnectionPolicy policy) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setIpAddressPolicy(addr, policy);
		});

		connect(m_ui->accessControl, &AccessControlWidget::ipAddressRemoved, [this](const QString & addr) {
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().clearIpAddressPolicy(addr);
		});

		for(const auto & mimeType : m_server->configuration().registeredMimeTypes()) {
			m_ui->fileAssociations->addAvailableMimeType(mimeType);
		}

		// file association slots
		connect(m_ui->fileAssociations, &FileAssociationsWidget::defaultMimeTypeChanged, [this](const QString & mimeType) {
			// TODO this should be done in FileAssociationsWidget
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setDefaultMimeType(mimeType);
		});

		// mime actions slots
		connect(m_ui->mimeTypeActions, &MimeTypeActionsWidget::defaultMimeTypeActionChanged, [this](WebServerAction action) {
			// TODO this should be done in MimeActionsWidget
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: received defaultMimeTypeActionChanged() with action = " << static_cast<int>(action) << "\n"
						 << std::flush;
			Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
			m_server->configuration().setDefaultAction(action);
		});

		connect(m_ui->allowDirectoryListings, &QCheckBox::toggled, this, &ConfigurationWidget::setAllowDirectoryListing);
		readConfiguration();
	}


	ConfigurationWidget::~ConfigurationWidget() = default;


	void ConfigurationWidget::setServer(Server * server) {
		m_ui->fileAssociations->setServer(server);
		m_ui->mimeTypeActions->setServer(server);
		m_server = server;

		if(m_server) {
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

		// read ip policy configuration
		m_ui->accessControl->clearAllConnectionPolicies();

		for(const auto & ip : opts.registeredIpAddressList()) {
			m_ui->accessControl->setIpAddressConnectionPolicy(ip, opts.ipAddressPolicy(ip));
		}

		m_ui->accessControl->setDefaultConnectionPolicy(opts.defaultConnectionPolicy());
		m_ui->allowDirectoryListings->setChecked(opts.isDirectoryListingAllowed());

		m_ui->fileAssociations->update();
		m_ui->mimeTypeActions->update();

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


	///TODO connecting this to QLineEdit::textEdited() is a bit heavy-handed because it makes the server restart
	///on every keypress and/or executes several copy operations on the bpWebServer::Configuration object on every
	///keypress. migrate to getConfiguration() returning a pointer to the actual options, and/or change to use
	///the editingFinished() signal
	void ConfigurationWidget::setDocumentRoot(const QString & docRoot) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		if(docRoot.isEmpty()) {
			return;
		}

		/* if we don't do this check, the widget will always be updated, which will mean
		 * that the cursor will be moved to the end of the widget on every keypress */
		if(m_ui->serverDetails->documentRoot() != docRoot) {
			m_ui->serverDetails->setDocumentRoot(docRoot);
		}

		m_server->configuration().setDocumentRoot(docRoot);
	}


	void ConfigurationWidget::setAllowDirectoryListing(bool allow) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_ui->allowDirectoryListings->setChecked(allow);
		Configuration & opts = m_server->configuration();
		opts.setAllowDirectoryListing(allow);
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


	void ConfigurationWidget::setListenPort(int port) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		if(-1 == port) {
			port = Configuration::DefaultPort;
		}
		else if(0 > port || 65535 < port) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid port " << port << "\n";
			return;
		}

		m_ui->serverDetails->setListenPort(static_cast<uint16_t>(port));
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
		setDefaultConnectionPolicy(ConnectionPolicy::Accept);
	}


	void ConfigurationWidget::setRestrictiveDefaultConnectionPolicy() {
		setDefaultConnectionPolicy(ConnectionPolicy::Reject);
	}


	void ConfigurationWidget::setDefaultConnectionPolicy(ConnectionPolicy policy) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_ui->accessControl->setDefaultConnectionPolicy(policy);
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: setting default connection policy directly.\n"
					 << std::flush;
		m_server->configuration().setDefaultConnectionPolicy(policy);
	}


	void ConfigurationWidget::setDefaultMimeType(const QString & mime) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_server->configuration().setDefaultMimeType(mime);
	}


	//		void ConfigurationWidget::setMimeTypeAction(const QString & mime, WebServerAction action) {
	//			m_ui->mimeTypeActions->setMimeTypeAction();
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


	//	void ConfigurationWidget::setDefaultAction(WebServerAction action) {
	//		Configuration & opts = m_server->configuration();
	//		opts.setDefaultAction(action);
	//	}


	void ConfigurationWidget::setIpConnectionPolicy(const QString & ip, ConnectionPolicy policy) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		if(m_server->configuration().setIpAddressPolicy(ip, policy)) {
			m_ui->accessControl->setIpAddressConnectionPolicy(ip, policy);
			return;
		}

		QMessageBox::warning(this, tr("Set IP Address Connection Policy"), tr("The connection policy for '%1' could not be set.").arg(ip));
	}


	void ConfigurationWidget::ipPolicyRemoved(const QString & ip) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_server->configuration().clearIpAddressPolicy(ip);
	}


	void ConfigurationWidget::clearIpConnectionPolicies() {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server must not be null");
		m_ui->accessControl->clearAllConnectionPolicies();
		m_server->configuration().clearAllIpAddressPolicies();
	}


}  // namespace EquitWebServer
