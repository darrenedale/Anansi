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
#include "accesslogwidget.h"
#include "connectionpolicycombo.h"
#include "webserveractioncombo.h"
#include "mimetypecombo.h"
#include "editabletreewidget.h"
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
	  m_eventsConnected(false),
	  m_server(server),
	  m_serverConfig(new ServerConfigWidget),
	  m_accessConfig(new AccessControlWidget),
	  m_allowDirectoryListing(new QCheckBox(tr("Allow directory listings"))),
	  m_fileAssociations(new FileAssociationsWidget(server)),
	  m_actionTree(new EditableTreeWidget),
	  m_actionMimeTypeCombo(new MimeTypeCombo(true)),
	  m_actionActionCombo(new WebServerActionCombo),
	  m_mimeTypeActionSetButton(new QToolButton),
	  m_defaultMimeCombo(new MimeTypeCombo(true)),
	  m_defaultActionCombo(new WebServerActionCombo),
	  m_accessLog(new AccessLogWidget),
	  m_serverControlsTab(new QTabWidget) {
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
			m_server->configuration().setDocumentRoot(docRoot);
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

		// file association slots
		connect(m_fileAssociations, &FileAssociationsWidget::extensionRemoved, [this](const QString & ext) {
			m_server->configuration().removeFileExtension(ext);
		});

		connect(m_fileAssociations, &FileAssociationsWidget::extensionChanged, [this](const QString & oldExt, const QString & newExt) {
			//			std::cout << "received extensionChanged(\"" << qPrintable(oldExt) << "\", \"" << qPrintable(newExt) << "\") signal\n"
			//						 << std::flush;
			auto & opts = m_server->configuration();
			auto mimeTypes = opts.mimeTypesForFileExtension(oldExt);
			opts.removeFileExtension(oldExt);

			for(const auto & mimeType : mimeTypes) {
				m_server->configuration().addFileExtensionMimeType(newExt, mimeType);
			}
		});

		connect(m_fileAssociations, &FileAssociationsWidget::extensionMimeTypeAdded, [this](const QString & ext, const QString & mimeType) {
			std::cout << "received extensionMimeTypeAdded(\"" << qPrintable(ext) << "\", \"" << qPrintable(mimeType) << "\") signal\n"
						 << std::flush;
			m_server->configuration().addFileExtensionMimeType(ext, mimeType);
		});

		connect(m_fileAssociations, &FileAssociationsWidget::extensionMimeTypeRemoved, [this](const QString & ext, const QString & mimeType) {
			//			std::cout << "received extensionMimeTypeRemoved(\"" << qPrintable(ext) << "\", \"" << qPrintable(mimeType) << "\") signal\n"
			//						 << std::flush;
			m_server->configuration().removeFileExtensionMimeType(ext, mimeType);
		});

		connect(m_fileAssociations, &FileAssociationsWidget::extensionMimeTypeChanged, [this](const QString & ext, const QString & oldMimeType, const QString & newMimeType) {
			std::cout << "received extensionMimeTypeChanged(\"" << qPrintable(ext) << "\", \"" << qPrintable(oldMimeType) << "\", \"" << qPrintable(newMimeType) << "\") signal\n"
						 << std::flush;

			if(oldMimeType == newMimeType) {
				return;
			}

			auto & opts = m_server->configuration();
			opts.addFileExtensionMimeType(ext, newMimeType);
			opts.removeFileExtensionMimeType(ext, oldMimeType);
		});

		//		for(const auto & mimeType : m_server->configuration().registeredMimeTypes()) {
		//			m_fileAssociations->addAvailableMimeType(mimeType);
		//		}

		/* widgets on the content control tab page */
		auto * contentControlTabPage = new QWidget;
		auto * contentControlLayout = new QVBoxLayout;
		contentControlTabPage->setLayout(contentControlLayout);
		auto * contentControlSplitter = new QSplitter;
		auto * actionSection = new QWidget;
		m_fileAssociations->setToolTip(tr("<p>This section allows you to associate file extensions with MIME types.</p><p>When a request is received for a resource, this section determines which MIME type is used when processing the request and sending response data.</p>"));
		actionSection->setToolTip(tr("<p>This section allows you to associate server actions with MIME types.</p><p>When a request is received for a resource, and its MIME type has been determined, this section defines what action the web server will take to generate the data for the response. The action can be:</p><ul><li><strong>Serve</strong> The resource (usually a file) will be sent verbatim</li><li><strong>Ignore</strong> The request will be ignored and no data will be sent</li><li><strong>Forbid</strong> The request will be rejected and a \"forbidden\" error response will be sent</li><li><strong>CGI</strong> The resource will be executed through the CGI environment and the output of the executed CGI command will be sent as the response. The CGI command to execute for a MIME type can be set by double-clicking the entry in the list; if no command is set, the resource is considered directly executable.</li></ul>"));
		contentControlSplitter->addWidget(m_fileAssociations);
		contentControlSplitter->addWidget(actionSection);
		contentControlLayout->addWidget(m_allowDirectoryListing);
		contentControlLayout->addWidget(contentControlSplitter);


		auto * actionControlLayout = new QHBoxLayout;
		//		m_actionMimeTypeCombo->setEditable(true);
		auto * actionMimeLabel = new QLabel(tr("MIME"));
		actionMimeLabel->setBuddy(m_actionMimeTypeCombo);

		m_mimeTypeActionSetButton->setIcon(QIcon(QStringLiteral(":/icons/buttons/setmimetypeaction")));

		actionControlLayout->addWidget(actionMimeLabel);
		actionControlLayout->addWidget(m_actionMimeTypeCombo);
		actionControlLayout->addWidget(m_actionActionCombo);
		actionControlLayout->addWidget(m_mimeTypeActionSetButton);
		actionControlLayout->setStretchFactor(m_actionMimeTypeCombo, 2);
		actionControlLayout->setStretchFactor(m_actionActionCombo, 1);

		auto * actionLayout = new QVBoxLayout(actionSection);
		m_actionTree->setColumnCount(3);
		m_actionTree->setItemDelegateForColumn(2, new QItemDelegate(this));
		auto * actionHeader = new QTreeWidgetItem;
		actionHeader->setText(0, tr("MIME Type"));
		actionHeader->setText(1, tr("Action"));
		actionHeader->setText(2, tr("CGI Executable"));
		m_actionTree->setHeaderItem(actionHeader);
		m_actionTree->setRootIsDecorated(false);

		auto * defaultActionLayout = new QHBoxLayout;
		m_defaultActionCombo->setToolTip(tr("The default action to use for all MIME types without specific registered actions."));
		auto * defaultActionLabel = new QLabel(tr("Default Action"));
		defaultActionLabel->setToolTip(tr("The default action to use for all MIME types without specific registered actions."));
		defaultActionLabel->setBuddy(m_defaultActionCombo);

		defaultActionLayout->addWidget(defaultActionLabel);
		defaultActionLayout->addWidget(m_defaultActionCombo);
		defaultActionLayout->setStretchFactor(m_defaultActionCombo, 1);

		actionLayout->addWidget(m_actionTree);
		actionLayout->addLayout(actionControlLayout);
		actionLayout->addLayout(defaultActionLayout);

		m_serverControlsTab->addTab(m_serverConfig, QIcon::fromTheme(QStringLiteral("network-server"), QIcon(QStringLiteral(":/icons/tabs/server"))), tr("Server"));
		m_serverControlsTab->setTabToolTip(0, tr("The main server setup."));
		m_serverControlsTab->addTab(m_accessConfig, QIcon::fromTheme(QStringLiteral("security-high"), QIcon(QStringLiteral(":/icons/tabs/accesscontrol"))), tr("Access Control"));
		m_serverControlsTab->setTabToolTip(1, tr("Tell the server what to do with HTTP connections from different IP addresses."));
		m_serverControlsTab->addTab(contentControlTabPage, QIcon::fromTheme(QStringLiteral("text-html"), QIcon(QStringLiteral(":/icons/tabs/contentcontrol"))), tr("Content Control"));
		m_serverControlsTab->setTabToolTip(2, tr("Tell the server how to handle requests for different types of resources."));
		m_serverControlsTab->addTab(m_accessLog, QIcon::fromTheme(QStringLiteral("text-x-log"), QIcon(QStringLiteral(":/icons/tabs/accesslog"))), tr("Access Log"));
		m_serverControlsTab->setTabToolTip(3, tr("View the server access log."));

		auto * mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m_serverControlsTab);

		readConfiguration();  // this ensures events are connected

		// now config read and lists populated, make columns a good width
		//		m_extensionMimeTypeTree->resizeColumnToContents(0);
		m_actionTree->resizeColumnToContents(0);
		m_actionTree->resizeColumnToContents(1);
		m_actionTree->resizeColumnToContents(2);

		setLayout(mainLayout);
	}


	ConfigurationWidget::~ConfigurationWidget() = default;


	void ConfigurationWidget::connectEvents() {
		if(!m_eventsConnected) {
			// content controls
			connect(m_allowDirectoryListing, &QCheckBox::toggled, this, &ConfigurationWidget::setAllowDirectoryListing);

			connect(m_mimeTypeActionSetButton, &QToolButton::clicked, this, qOverload<>(&ConfigurationWidget::setMimeTypeAction));

			connect(m_actionTree, &EditableTreeWidget::removingItem, this, &ConfigurationWidget::removeAction);
			connect(m_actionTree, &EditableTreeWidget::itemDoubleClicked, this, &ConfigurationWidget::onActionDoubleClicked);
			connect(m_actionTree, &EditableTreeWidget::currentItemChanged, this, &ConfigurationWidget::onMimeActionSelectedItemChanged);

			connect(m_defaultMimeCombo, &MimeTypeCombo::currentMimeTypeChanged, this, &ConfigurationWidget::setDefaultMimeType, Qt::UniqueConnection);
			connect(m_defaultActionCombo, &WebServerActionCombo::webServerActionChanged, this, &ConfigurationWidget::setDefaultAction, Qt::UniqueConnection);
			m_eventsConnected = true;
		}
	}


	void ConfigurationWidget::disconnectEvents() {
		if(m_eventsConnected) {
			// content controls
			disconnect(m_allowDirectoryListing, &QCheckBox::toggled, this, &ConfigurationWidget::setAllowDirectoryListing);

			disconnect(m_mimeTypeActionSetButton, &QToolButton::clicked, this, qOverload<>(&ConfigurationWidget::setMimeTypeAction));

			disconnect(m_actionTree, &EditableTreeWidget::removingItem, this, &ConfigurationWidget::removeAction);
			disconnect(m_actionTree, &EditableTreeWidget::itemDoubleClicked, this, &ConfigurationWidget::onActionDoubleClicked);
			disconnect(m_actionTree, &EditableTreeWidget::currentItemChanged, this, &ConfigurationWidget::onMimeActionSelectedItemChanged);

			disconnect(m_defaultMimeCombo, &MimeTypeCombo::currentMimeTypeChanged, this, &ConfigurationWidget::setDefaultMimeType);
			disconnect(m_defaultActionCombo, &WebServerActionCombo::webServerActionChanged, this, &ConfigurationWidget::setDefaultAction);
			m_eventsConnected = false;
		}
	}


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

		disconnectEvents();
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

		std::unordered_set<QString> allMimes;

		{
			m_fileAssociations->update();
			//			QSignalBlocker block(m_fileAssociations);
			//			m_fileAssociations->clear();

			//			//		m_extensionMimeTypeTree->clear();


			//			// read mime type extension mappings
			//			for(const auto & ext : opts.registeredFileExtensions()) {
			//				//			m_fileExtensionCombo->addItem(ext);
			//				//			QTreeWidgetItem * item = new QTreeWidgetItem(m_extensionMimeTypeTree);
			//				//			item->setText(0, ext);

			//				for(const auto & mimeType : opts.mimeTypesForFileExtension(ext)) {
			//					//				QTreeWidgetItem * child = new QTreeWidgetItem(item);
			//					//				child->setText(0, mimeType);
			//					//				QIcon icon = mimeIcon(mimeType);

			//					//				if(!icon.isNull()) {
			//					//					child->setIcon(0, icon);
			//					//				}

			//					m_fileAssociations->addExtensionMimeType(ext, mimeType);
			//					allMimes.insert(mimeType);
			//				}
			//			}
		}

		// read mime type actions
		m_actionTree->clear();

		for(const auto & mimeType : opts.registeredMimeTypes()) {
			QTreeWidgetItem * item = new QTreeWidgetItem(m_actionTree);
			item->setText(0, (mimeType));
			QIcon icon = mimeIcon(mimeType);

			if(!icon.isNull()) {
				item->setIcon(0, icon);
			}

			auto action = opts.mimeTypeAction(mimeType);
			item->setData(1, WebServerActionRole, QVariant::fromValue(action));

			switch(action) {
				case Configuration::WebServerAction::Ignore:
					item->setText(1, "Ignore");
					break;

				case Configuration::WebServerAction::Serve:
					item->setText(1, "Serve");
					break;

				case Configuration::WebServerAction::CGI:
					item->setText(1, "CGI");
					item->setText(2, opts.mimeTypeCgi(mimeType));
					break;

				case Configuration::WebServerAction::Forbid:
					item->setText(1, "Forbid");
					break;
			}

			allMimes.insert(mimeType);
		}

		QString defaultMime = opts.defaultMimeType();

		if(!defaultMime.isEmpty()) {
			allMimes.insert(defaultMime);
		}

		// populate all MIME type combos with known MIME types
		m_actionMimeTypeCombo->clear();

		for(const auto & mime : allMimes) {
			m_actionMimeTypeCombo->addMimeType(mime);
		}

		//		m_fileExtensionCombo->lineEdit()->setText(QStringLiteral(""));
		m_actionMimeTypeCombo->setCurrentMimeType(QStringLiteral(""));
		m_defaultActionCombo->setWebServerAction(opts.defaultAction());
		connectEvents();
		setEnabled(true);
	}


	void ConfigurationWidget::onActionDoubleClicked(QTreeWidgetItem * it) {
		if(!it || it->treeWidget() != m_actionTree) {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: received no item or item that does not belong to action list.\n";
			return;
		}

		Configuration & opts = m_server->configuration();
		QString mime = it->text(0);
		Configuration::WebServerAction action = opts.mimeTypeAction(mime);

		if(action != Configuration::WebServerAction::CGI) {
			if(QMessageBox::Yes != QMessageBox::question(this, "Set CGI Executable", tr("The action for the MIME type '%1' is not set to CGI. Should the web server alter the action for this MIME type to CGI?").arg(mime), QMessageBox::Yes | QMessageBox::No)) {
				return;
			}

			if(!opts.setMimeTypeAction(mime, Configuration::WebServerAction::CGI)) {
				QMessageBox::critical(this, "Set CGI Executable", tr("The action for the MIME type '%1' could not be set to CGI.").arg(mime));
				return;
			}
		}

		QString initialPath = it->text(2);

		if(initialPath.isEmpty()) {
			const auto paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

			if(!paths.isEmpty()) {
				initialPath = paths.front();
			}
		}

		QString newCGI = QFileDialog::getOpenFileName(this, "Set CGI Executable", initialPath);

		if(!newCGI.isNull()) {
			opts.setMimeTypeCgi(mime, newCGI);
			it->setText(2, newCGI);
			m_server->setConfiguration(opts);
		}
	}


	void ConfigurationWidget::clearAllActions() {
		m_actionTree->clear();
		m_server->configuration().clearAllMimeTypeActions();
	}


	void ConfigurationWidget::clearAllFileExtensionMIMETypes() {
		//		m_extensionMimeTypeTree->clear();
		QSignalBlocker block(m_fileAssociations);
		m_fileAssociations->clear();
		//		m_server->configuration().clearAllFileExtensions();
	}


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
		Configuration & opts = m_server->configuration();
		opts.setDefaultMimeType(mime);
	}


	void ConfigurationWidget::setMimeTypeAction() {
		setMimeTypeAction(m_actionMimeTypeCombo->currentMimeType().trimmed(), m_actionActionCombo->webServerAction());
	}


	void ConfigurationWidget::setMimeTypeAction(const QString & mime, Configuration::WebServerAction action) {
		Configuration & opts = m_server->configuration();

		if(opts.setMimeTypeAction(mime, action)) {
			int items = m_actionTree->topLevelItemCount();
			QTreeWidgetItem * it = nullptr;
			bool foundMIME = false;
			QString actionText;

			switch(action) {
				case Configuration::WebServerAction::Ignore:
					actionText = "Ignore";
					break;

				case Configuration::WebServerAction::Serve:
					actionText = "Serve";
					break;

				case Configuration::WebServerAction::CGI:
					actionText = "CGI";
					break;

				case Configuration::WebServerAction::Forbid:
					actionText = "Forbid";
					break;
			}

			for(int i = 0; i < items && !foundMIME; i++) {
				if((it = m_actionTree->topLevelItem(i)) && it->text(0) == mime) {
					foundMIME = true;
				}
			}

			if(!foundMIME) {
				it = new QTreeWidgetItem(m_actionTree);
			}

			QString iconName(mime);
			iconName.replace('/', '-');
			QIcon icon = QIcon::fromTheme(iconName, QIcon(MimeIconResourcePath + iconName + ".png"));

			if(icon.isNull()) {
				/* use generic icons from theme for certain MIME types */
				if("image/" == mime.left(6)) {
					icon = QIcon::fromTheme("image-x-generic", QIcon(MimeIconResourcePath + "image-x-generic"));
				}
				else if("audio/" == mime.left(6)) {
					icon = QIcon::fromTheme("audio-x-generic", QIcon(MimeIconResourcePath + "audio-x-generic"));
				}
				else if("video/" == mime.left(6)) {
					icon = QIcon::fromTheme("video-x-generic", QIcon(MimeIconResourcePath + "video-x-generic"));
				}
				else if("package/" == mime.left(8)) {
					icon = QIcon::fromTheme("package-x-generic", QIcon(MimeIconResourcePath + "package-x-generic"));
				}
				else if("text/" == mime.left(5)) {
					icon = QIcon::fromTheme("text-x-generic", QIcon(MimeIconResourcePath + "text-x-generic"));
				}
			}

			it->setText(0, mime);
			it->setText(1, actionText);

			if(!icon.isNull()) {
				it->setIcon(0, icon);
			}
		}
	}


	void ConfigurationWidget::removeAction(QTreeWidgetItem * it) {
		if(it) {
			QString mime = it->text(0);
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Clearing action for MIME type '" << qPrintable(mime) << "'\n";
			Configuration & opts = m_server->configuration();
			opts.unsetMimeTypeAction(mime);
		}
		else {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Could not identify the MIME type from which to remove the action.\n";
		}
	}


	void ConfigurationWidget::removeExtensionMimeType(QTreeWidgetItem * it) {
		if(it) {
			QTreeWidgetItem * p = it->parent();
			QString ext, mime;

			if(p) {
				/* MIME type */
				ext = p->text(0);
				mime = it->text(0);
			}
			else {
				/* extension */
				ext = it->text(0);
			}

			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Clearing MIME type '" << qPrintable(mime) << "' from extension '" << qPrintable(ext) << "'.\n";
			Configuration & opts = m_server->configuration();
			opts.removeFileExtensionMimeType(ext, mime);
		}
		else {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Could not identify the extention and MIME type pair to remove.\n";
		}
	}


	void ConfigurationWidget::setDefaultAction(Configuration::WebServerAction action) {
		Configuration & opts = m_server->configuration();
		opts.setDefaultAction(action);
	}


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


	void ConfigurationWidget::onMimeActionSelectedItemChanged(QTreeWidgetItem * it) {
		// update the mime action combos with selected entry
		if(it && it->treeWidget() == m_actionTree) {
			m_actionMimeTypeCombo->setCurrentMimeType(it->text(0));
			m_actionActionCombo->setWebServerAction(it->data(1, WebServerActionRole).value<Configuration::WebServerAction>());
		}
	}
}  // namespace EquitWebServer
