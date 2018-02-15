/** \file ConfigurationWidget.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the ConfigurationWidget class for EquitWebServer
  *
  * \todo
  * - content control tab is ugly and complex. Can the two trees be integrated
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
  * - icon engine won't scale images to create new size for MIME type icons in
  *   lists - check out what's going on here...
  * - move the MIME type icon-finding code to a global support function. it is
  *   currently duplicated in readConfiguration() (twice), setMIMETypeAction()
  *   and addFileExtensionMIMEType()
  * - setting listen address to invalid value leaves config with old listen
  *   address. so, e.g., setting it to 127.0.0.1, then changing it to "invalid"
  *   leaves the config with 127.0.0.1 but the display with "invalid". should
  *   probably attempt DNS lookup or insist on IPv4 formatted addresses rather
  *   than host names.
  * - saving configuration after changing document root does not save new
  *   document root until server is restarted (because document root does not
  *   get set in server options until server is restarted).
  * - decide on application license
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

#include <QtGlobal>
#include <QString>
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
#include "accesslogwidget.h"
#include "connectionpolicycombo.h"
#include "editabletreewidget.h"
//#include "hostnetworkinfo.h"
#include "ipaddressconnectionpolicytreeitem.h"

#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_OK QIcon::fromTheme("task-complete", QIcon(":/icons/status/ok")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING QIcon::fromTheme("task-attention", QIcon(":/icons/status/warning")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_ERROR QIcon::fromTheme("task-attention", QIcon(":/icons/status/error")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_UNKNOWN QIcon("/icons/status/unknown").pixmap(16)

#define EQUITWEBSERVER_CONFIGURATIONWIDGET_MIMEICONRESOURCE_PATH ":/icons/mime/"


namespace EquitWebServer {


	QString ConfigurationWidget::s_mimeIconResourcePath(EQUITWEBSERVER_CONFIGURATIONWIDGET_MIMEICONRESOURCE_PATH);


	ConfigurationWidget::ConfigurationWidget(Server * server, QWidget * parent)
	: QWidget(parent),
	  m_eventsConnected(false),
	  m_server(server),
	  m_serverConfig(new ServerConfigWidget),
	  m_accessConfig(new AccessControlWidget),
	  m_allowDirectoryListing(nullptr),
	  m_extensionMIMETypeTree(nullptr),
	  m_fileExtensionCombo(nullptr),
	  m_extensionMimeTypeCombo(nullptr),
	  m_extensionMimeTypeAddButton(nullptr),
	  m_actionTree(nullptr),
	  m_actionMimeTypeCombo(nullptr),
	  m_actionActionCombo(nullptr),
	  m_mimeTypeActionSetButton(nullptr),
	  m_defaultMIMECombo(nullptr),
	  m_defaultActionCombo(nullptr),
	  m_accessLog(new AccessLogWidget),
	  m_serverControlsTab(nullptr) {
		Q_ASSERT(m_server);

		// TODO why do these work as lambdas but not as a directly-connected slots?
		// The slot is being connected successfully because the QMetaObject::Connection returned is valid.
		// But the slot is never called; contrarily, in the lambda, the lambda is invoked and the AccessLogWidget
		// slot is called successfully
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: connecting to server logging signals\n";
		connect(m_server, &Server::requestConnectionPolicyDetermined, m_accessLog, &AccessLogWidget::addPolicyEntry);
		connect(m_server, &Server::requestConnectionPolicyDetermined, [this](const QString & addr, quint16 port, Configuration::ConnectionPolicy policy) {
			std::cout << "calling AccessLogWidget::addPolicyEntry() as a result of Server::requestConnectionPolicyDetermined signal\n";
			m_accessLog->addPolicyEntry(addr, port, policy);
		});
		connect(m_server, &Server::requestActionTaken, m_accessLog, &AccessLogWidget::addActionEntry);
		connect(m_server, &Server::requestActionTaken, [this](const QString & addr, quint16 port, const QString & resource, Configuration::WebServerAction action) {
			std::cout << "calling AccessLogWidget::addActionEntry() as a result of Server::requestActionTaken signal\n";
			m_accessLog->addActionEntry(addr, port, resource, action);
		});

		// server config slots
		connect(m_serverConfig, &ServerConfigWidget::documentRootChanged, [this](const QString & docRoot) {
			m_server->configuration().setDocumentRoot(docRoot);
		});

		connect(m_serverConfig, &ServerConfigWidget::listenIpAddressChanged, [this](const QString & addr) {
			m_server->configuration().setListenAddress(addr);
		});

		connect(m_serverConfig, &ServerConfigWidget::listenPortChanged, [this](uint16_t port) {
			m_server->configuration().setPort(port);
		});

		// access config slots
		connect(m_accessConfig, &AccessControlWidget::defaultConnectionPolicyChanged, [this](Configuration::ConnectionPolicy policy) {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: setting default connection policy in response to signal from AccessControlWidget.\n"
						 << std::flush;
			m_server->configuration().setDefaultConnectionPolicy(policy);
		});

		connect(m_accessConfig, &AccessControlWidget::ipAddressConnectionPolicySet, [this](const QString & addr, Configuration::ConnectionPolicy policy) {
			m_server->configuration().setIpAddressPolicy(addr, policy);
		});

		connect(m_accessConfig, &AccessControlWidget::ipAddressRemoved, [this](const QString & addr) {
			m_server->configuration().clearIpAddressPolicy(addr);
		});

		/* widgets on the content control tab page */
		QWidget * contentControlTabPage = new QWidget;
		QVBoxLayout * contentControlLayout = new QVBoxLayout;
		contentControlTabPage->setLayout(contentControlLayout);
		QSplitter * contentControlSplitter = new QSplitter;
		QWidget * mimeSection = new QWidget;
		QWidget * actionSection = new QWidget;
		mimeSection->setToolTip(tr("<p>This section allows you to associate file extensions with MIME types.</p><p>When a request is received for a resource, this section determines which MIME type is used when processing the request and sending response data.</p>"));
		actionSection->setToolTip(tr("<p>This section allows you to associate server actions with MIME types.</p><p>When a request is received for a resource, and its MIME type has been determined, this section defines what action the web server will take to generate the data for the response. The action can be:</p><ul><li><strong>Serve</strong> The resource (usually a file) will be sent verbatim</li><li><strong>Ignore</strong> The request will be ignored and no data will be sent</li><li><strong>Forbid</strong> The request will be rejected and a \"forbidden\" error response will be sent</li><li><strong>CGI</strong> The resource will be executed through the CGI environment and the output of the executed CGI command will be sent as the response. The CGI command to execute for a MIME type can be set by double-clicking the entry in the list; if no command is set, the resource is considered directly executable.</li></ul>"));
		contentControlSplitter->addWidget(mimeSection);
		contentControlSplitter->addWidget(actionSection);
		m_allowDirectoryListing = new QCheckBox(tr("Allow directory listings"));
		contentControlLayout->addWidget(m_allowDirectoryListing);
		contentControlLayout->addWidget(contentControlSplitter);
		m_extensionMIMETypeTree = new EditableTreeWidget;
		m_extensionMIMETypeTree->setColumnCount(1);
		QTreeWidgetItem * mimeTreeHeader = new QTreeWidgetItem;
		mimeTreeHeader->setText(0, tr("MIME Type Associations"));
		m_extensionMIMETypeTree->setHeaderItem(mimeTreeHeader);
		m_fileExtensionCombo = new QComboBox;
		m_fileExtensionCombo->setEditable(true);
		QLabel * fileExtensionLabel = new QLabel(tr("&Extension"));
		fileExtensionLabel->setBuddy(m_fileExtensionCombo);

		m_extensionMimeTypeCombo = new QComboBox;
		m_extensionMimeTypeCombo->setEditable(true);
		QLabel * mimeLabel = new QLabel(tr("&MIME"));
		mimeLabel->setBuddy(m_extensionMimeTypeCombo);

		m_extensionMimeTypeAddButton = new QToolButton;
		m_extensionMimeTypeAddButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/buttons/addextensionmimetype")));

		QHBoxLayout * fileExtensionMIMETypeLayout = new QHBoxLayout;
		fileExtensionMIMETypeLayout->addWidget(fileExtensionLabel);
		fileExtensionMIMETypeLayout->addWidget(m_fileExtensionCombo);
		fileExtensionMIMETypeLayout->addWidget(mimeLabel);
		fileExtensionMIMETypeLayout->addWidget(m_extensionMimeTypeCombo);
		fileExtensionMIMETypeLayout->addWidget(m_extensionMimeTypeAddButton);
		fileExtensionMIMETypeLayout->setStretchFactor(m_fileExtensionCombo, 1);
		fileExtensionMIMETypeLayout->setStretchFactor(m_extensionMimeTypeCombo, 2);

		QHBoxLayout * defaultMIMETypeLayout = new QHBoxLayout;
		m_defaultMIMECombo = new QComboBox;
		m_defaultMIMECombo->setEditable(true);
		m_defaultMIMECombo->setToolTip(tr("The default MIME Type to use for all extensions without a registered MIME type."));
		QLabel * defaultMIMETypeLabel = new QLabel(tr("Default MIME Type"));
		defaultMIMETypeLabel->setToolTip(tr("The default MIME Type to use for all extensions without a registered MIME type."));
		defaultMIMETypeLabel->setBuddy(m_defaultMIMECombo);

		defaultMIMETypeLayout->addWidget(defaultMIMETypeLabel);
		defaultMIMETypeLayout->addWidget(m_defaultMIMECombo);
		defaultMIMETypeLayout->setStretchFactor(m_defaultMIMECombo, 1);

		QVBoxLayout * extensionMimeLayout = new QVBoxLayout(mimeSection);
		extensionMimeLayout->addWidget(m_extensionMIMETypeTree);
		extensionMimeLayout->addLayout(fileExtensionMIMETypeLayout);
		extensionMimeLayout->addLayout(defaultMIMETypeLayout);

		QHBoxLayout * actionControlLayout = new QHBoxLayout;
		m_actionMimeTypeCombo = new QComboBox;
		m_actionMimeTypeCombo->setEditable(true);
		QLabel * actionMimeLabel = new QLabel(tr("MIME"));
		actionMimeLabel->setBuddy(m_actionMimeTypeCombo);

		m_actionActionCombo = new QComboBox;
		m_actionActionCombo->addItem(tr("Ignore"), Configuration::WebServerAction::Ignore);
		m_actionActionCombo->addItem(tr("Serve"), Configuration::WebServerAction::Serve);
		m_actionActionCombo->addItem(tr("CGI"), Configuration::WebServerAction::CGI);
		m_actionActionCombo->addItem(tr("Forbid"), Configuration::WebServerAction::Forbid);
		m_mimeTypeActionSetButton = new QToolButton;
		m_mimeTypeActionSetButton->setIcon(QIcon(":/icons/buttons/setmimetypeaction"));

		actionControlLayout->addWidget(actionMimeLabel);
		actionControlLayout->addWidget(m_actionMimeTypeCombo);
		actionControlLayout->addWidget(m_actionActionCombo);
		actionControlLayout->addWidget(m_mimeTypeActionSetButton);
		actionControlLayout->setStretchFactor(m_actionMimeTypeCombo, 2);
		actionControlLayout->setStretchFactor(m_actionActionCombo, 1);

		QVBoxLayout * actionLayout = new QVBoxLayout(actionSection);
		m_actionTree = new EditableTreeWidget;
		m_actionTree->setColumnCount(3);
		m_actionTree->setItemDelegateForColumn(2, new QItemDelegate(this));
		QTreeWidgetItem * actionHeader = new QTreeWidgetItem;
		actionHeader->setText(0, tr("MIME Type"));
		actionHeader->setText(1, tr("Action"));
		actionHeader->setText(2, tr("CGI Executable"));
		m_actionTree->setHeaderItem(actionHeader);
		m_actionTree->setRootIsDecorated(false);

		QHBoxLayout * defaultActionLayout = new QHBoxLayout;
		m_defaultActionCombo = new QComboBox;
		m_defaultActionCombo->addItem(tr("Ignore"), Configuration::Ignore);
		m_defaultActionCombo->addItem(tr("Serve"), Configuration::Serve);
		m_defaultActionCombo->addItem(tr("CGI"), Configuration::CGI);
		m_defaultActionCombo->addItem(tr("Forbid"), Configuration::Forbid);
		m_defaultActionCombo->setToolTip(tr("The default action to use for all MIME types without specific registered actions."));
		QLabel * defaultActionLabel = new QLabel(tr("Default Action"));
		defaultActionLabel->setToolTip(tr("The default action to use for all MIME types without specific registered actions."));
		defaultActionLabel->setBuddy(m_defaultActionCombo);

		defaultActionLayout->addWidget(defaultActionLabel);
		defaultActionLayout->addWidget(m_defaultActionCombo);
		defaultActionLayout->setStretchFactor(m_defaultActionCombo, 1);

		actionLayout->addWidget(m_actionTree);
		actionLayout->addLayout(actionControlLayout);
		actionLayout->addLayout(defaultActionLayout);

		m_serverControlsTab = new QTabWidget;
		m_serverControlsTab->addTab(m_serverConfig, QIcon::fromTheme("network-server", QIcon(":/icons/tabs/server")), tr("Server"));
		m_serverControlsTab->setTabToolTip(0, tr("The main server setup."));
		m_serverControlsTab->addTab(m_accessConfig, QIcon::fromTheme("security-high", QIcon(":/icons/tabs/accesscontrol")), tr("Access Control"));
		m_serverControlsTab->setTabToolTip(1, tr("Tell the server what to do with HTTP connections from different IP addresses."));
		m_serverControlsTab->addTab(contentControlTabPage, QIcon::fromTheme("text-html", QIcon(":/icons/tabs/contentcontrol")), tr("Content Control"));
		m_serverControlsTab->setTabToolTip(2, tr("Tell the server how to handle requests for different types of resources."));
		m_serverControlsTab->addTab(m_accessLog, QIcon::fromTheme("text-x-log", QIcon(":/icons/tabs/accesslog")), tr("Access Log"));
		m_serverControlsTab->setTabToolTip(3, tr("View the server access log."));

		QVBoxLayout * mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m_serverControlsTab);

		readConfiguration();  // this ensures events are connected

		// now config read and lists populated, make columns a good width
		m_extensionMIMETypeTree->resizeColumnToContents(0);
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

			connect(m_extensionMIMETypeTree, &EditableTreeWidget::removingItem, this, &ConfigurationWidget::removeExtensionMIMEType);
			connect(m_extensionMIMETypeTree, &EditableTreeWidget::currentItemChanged, this, &ConfigurationWidget::extensionTreeSelectedItemChanged);
			connect(m_extensionMimeTypeAddButton, &QToolButton::clicked, this, &ConfigurationWidget::addFileExtensionMIMEType);
			connect(m_mimeTypeActionSetButton, &QToolButton::clicked, this, qOverload<>(&ConfigurationWidget::setMIMETypeAction));

			connect(m_actionTree, &EditableTreeWidget::removingItem, this, &ConfigurationWidget::removeAction);
			connect(m_actionTree, &EditableTreeWidget::itemDoubleClicked, this, &ConfigurationWidget::actionDoubleClicked);
			connect(m_actionTree, &EditableTreeWidget::currentItemChanged, this, &ConfigurationWidget::mimeActionSelectedItemChanged);

			connect(m_defaultMIMECombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDefaultMIMEType(QString)));
			connect(m_defaultActionCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDefaultMIMEType(QString)));
			m_eventsConnected = true;
		}
	}


	void ConfigurationWidget::disconnectEvents() {
		if(m_eventsConnected) {
			// content controls
			disconnect(m_allowDirectoryListing, &QCheckBox::toggled, this, &ConfigurationWidget::setAllowDirectoryListing);

			disconnect(m_extensionMIMETypeTree, &EditableTreeWidget::removingItem, this, &ConfigurationWidget::removeExtensionMIMEType);
			disconnect(m_extensionMIMETypeTree, &EditableTreeWidget::currentItemChanged, this, &ConfigurationWidget::extensionTreeSelectedItemChanged);
			disconnect(m_extensionMimeTypeAddButton, &QToolButton::clicked, this, &ConfigurationWidget::addFileExtensionMIMEType);
			disconnect(m_mimeTypeActionSetButton, &QToolButton::clicked, this, qOverload<>(&ConfigurationWidget::setMIMETypeAction));

			disconnect(m_actionTree, &EditableTreeWidget::removingItem, this, &ConfigurationWidget::removeAction);
			disconnect(m_actionTree, &EditableTreeWidget::itemDoubleClicked, this, &ConfigurationWidget::actionDoubleClicked);
			disconnect(m_actionTree, &EditableTreeWidget::currentItemChanged, this, &ConfigurationWidget::mimeActionSelectedItemChanged);

			disconnect(m_defaultMIMECombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultMIMEType()));
			disconnect(m_defaultActionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultAction()));
			m_eventsConnected = false;
		}
	}


	void ConfigurationWidget::setServer(Server * server) {
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
		Configuration & opts = m_server->configuration();
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

		for(const auto & ip : opts.registeredIPAddressList()) {
			m_accessConfig->setIpAddressConnectionPolicy(ip, opts.ipAddressPolicy(ip));
		}

		m_accessConfig->setDefaultConnectionPolicy(opts.defaultConnectionPolicy());

		m_allowDirectoryListing->setChecked(opts.isDirectoryListingAllowed());
		m_extensionMIMETypeTree->clear();

		QVector<QString> allMimes;

		// read mime type extension mappings
		{
			QStringList extensions = opts.registeredFileExtensions();
			QStringList::iterator i = extensions.begin();

			while(i != extensions.end()) {
				m_fileExtensionCombo->addItem(*i);
				QTreeWidgetItem * item = new QTreeWidgetItem(m_extensionMIMETypeTree);
				item->setText(0, *i);

				auto types = opts.mimeTypesForFileExtension(*i);
				auto iMime = types.begin();

				while(iMime != types.end()) {
					QTreeWidgetItem * child = new QTreeWidgetItem(item);
					child->setText(0, *iMime);
					QString iconName(*iMime);
					iconName.replace('/', '-');
					QIcon icon = QIcon::fromTheme(iconName, QIcon(s_mimeIconResourcePath + iconName + ".png"));

					if(icon.isNull()) {
						/* use generic icons from theme for certain MIME types */
						if("image/" == (*iMime).left(6)) {
							icon = QIcon::fromTheme("image-x-generic", QIcon(s_mimeIconResourcePath + "image-x-generic"));
						}
						else if("audio/" == (*iMime).left(6)) {
							icon = QIcon::fromTheme("audio-x-generic", QIcon(s_mimeIconResourcePath + "audio-x-generic"));
						}
						else if("video/" == (*iMime).left(6)) {
							icon = QIcon::fromTheme("video-x-generic", QIcon(s_mimeIconResourcePath + "video-x-generic"));
						}
						else if("package/" == (*iMime).left(8)) {
							icon = QIcon::fromTheme("package-x-generic", QIcon(s_mimeIconResourcePath + "package-x-generic"));
						}
						else if("text/" == (*iMime).left(5)) {
							icon = QIcon::fromTheme("text-x-generic", QIcon(s_mimeIconResourcePath + "text-x-generic"));
						}
					}

					if(!icon.isNull()) {
						child->setIcon(0, icon);
					}

					if(!allMimes.contains(*iMime)) {
						allMimes << (*iMime);
					}

					iMime++;
				}

				i++;
			}
		}

		// read mime type actions
		m_actionTree->clear();

		{
			QStringList mimes = opts.registeredMIMETypes();
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: " << mimes.count() << " MIME Types with registered actions.\n";
			QStringList::iterator i = mimes.begin();

			while(i != mimes.end()) {
				QTreeWidgetItem * item = new QTreeWidgetItem(m_actionTree);
				item->setText(0, (*i));
				QString iconName(*i);
				iconName.replace('/', '-');
				std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: icon name:" << qPrintable(iconName);
				std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: fallback icon resource path:" << qPrintable(s_mimeIconResourcePath + iconName + ".png");
				QIcon icon(QIcon::fromTheme(iconName, QIcon(s_mimeIconResourcePath + qPrintable(iconName) + ".png")));
				std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: found icon: " << (icon.isNull() ? "no" : "yes");

				if(icon.isNull()) {
					/* use generic icons from theme for certain MIME types */
					if("image/" == (*i).left(6)) {
						icon = QIcon::fromTheme("image-x-generic");
					}
					else if("audio/" == (*i).left(6)) {
						icon = QIcon::fromTheme("audio-x-generic");
					}
					else if("video/" == (*i).left(6)) {
						icon = QIcon::fromTheme("video-x-generic");
					}
					else if("package/" == (*i).left(8)) {
						icon = QIcon::fromTheme("package-x-generic");
					}
					else if("text/" == (*i).left(5)) {
						icon = QIcon::fromTheme("text-x-generic");
					}

					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: found generic icon without using MIME subtype: " << (icon.isNull() ? "no" : "yes");
				}

				if(!icon.isNull()) {
					item->setIcon(0, icon);
				}

				switch(opts.mimeTypeAction(*i)) {
					case Configuration::Ignore:
						item->setText(1, "Ignore");
						break;

					case Configuration::Serve:
						item->setText(1, "Serve");
						break;

					case Configuration::CGI:
						item->setText(1, "CGI");
						item->setText(2, opts.mimeTypeCgi(*i));
						break;

					case Configuration::Forbid:
						item->setText(1, "Forbid");
						break;
				}

				if(!allMimes.contains(*i)) {
					allMimes << (*i);
				}

				i++;
			}
		}

		QString defaultMime = opts.defaultMIMEType();

		if(!defaultMime.isEmpty() && !allMimes.contains(defaultMime)) {
			allMimes << defaultMime;
		}

		// populate all MIME type combos with known MIME types
		m_actionMimeTypeCombo->clear();
		m_extensionMimeTypeCombo->clear();
		m_defaultMIMECombo->clear();

		for(const auto & mime : allMimes) {
			m_actionMimeTypeCombo->addItem(mime);
			m_extensionMimeTypeCombo->addItem(mime);
			m_defaultMIMECombo->addItem(mime);
		}

		m_fileExtensionCombo->lineEdit()->setText("");
		m_extensionMimeTypeCombo->lineEdit()->setText("");
		m_actionMimeTypeCombo->lineEdit()->setText("");
		m_actionMimeTypeCombo->lineEdit()->setText("");

		m_defaultActionCombo->setCurrentIndex(m_actionActionCombo->findData(opts.defaultAction()));
		m_defaultMIMECombo->lineEdit()->setText(defaultMime);
		connectEvents();
		setEnabled(true);
	}


	void ConfigurationWidget::actionDoubleClicked(QTreeWidgetItem * it) {
		if(!it || it->treeWidget() != m_actionTree) {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: bpWebServer::bpWebServerController::actionDoubleClicked() - received no item or item that does not belong to action list.\n";
			return;
		}

		Configuration & opts = m_server->configuration();
		QString mime = it->text(0);
		Configuration::WebServerAction action = opts.mimeTypeAction(mime);

		if(action != Configuration::CGI) {
			if(QMessageBox::Yes != QMessageBox::question(this, "Set CGI Executable", tr("The action for the MIME type '%1' is not set to CGI. Should the web server alter the action for this MIME type to CGI?").arg(mime), QMessageBox::Yes | QMessageBox::No)) {
				return;
			}

			if(!opts.setMimeTypeAction(mime, Configuration::CGI)) {
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


	void ConfigurationWidget::addFileExtensionMIMEType() {
		QString ext = m_fileExtensionCombo->lineEdit()->text().trimmed();
		QString mime = m_extensionMimeTypeCombo->lineEdit()->text().trimmed();

		if(ext.startsWith(".")) {
			ext = ext.right(ext.size() - 1);
		}

		Configuration & opts = m_server->configuration();

		if(opts.addFileExtensionMIMEType(ext, mime)) {
			int items = m_extensionMIMETypeTree->topLevelItemCount();
			QTreeWidgetItem * it;
			bool addedMIME = false;

			for(int i = 0; i < items; i++) {
				if((it = m_extensionMIMETypeTree->topLevelItem(i)) && it->text(0) == ext) {
					QTreeWidgetItem * child = new QTreeWidgetItem(it);
					child->setText(0, mime);
					addedMIME = true;
					break; /* exit for loop */
				}
			}

			if(!addedMIME) {
				it = new QTreeWidgetItem(m_extensionMIMETypeTree);
				it->setText(0, ext);
				QTreeWidgetItem * child = new QTreeWidgetItem(it);
				child->setText(0, mime);

				/* add icon */
				QString iconName(mime);
				iconName.replace('/', '-');
				QIcon icon = QIcon::fromTheme(iconName, QIcon(s_mimeIconResourcePath + iconName + ".png"));

				if(icon.isNull()) {
					/* use generic icons from theme for certain MIME types */
					if("image/" == mime.left(6)) {
						icon = QIcon::fromTheme("image-x-generic", QIcon(s_mimeIconResourcePath + "image-x-generic"));
					}
					else if("audio/" == mime.left(6)) {
						icon = QIcon::fromTheme("audio-x-generic", QIcon(s_mimeIconResourcePath + "audio-x-generic"));
					}
					else if("video/" == mime.left(6)) {
						icon = QIcon::fromTheme("video-x-generic", QIcon(s_mimeIconResourcePath + "video-x-generic"));
					}
					else if("package/" == mime.left(8)) {
						icon = QIcon::fromTheme("package-x-generic", QIcon(s_mimeIconResourcePath + "package-x-generic"));
					}
					else if("text/" == mime.left(5)) {
						icon = QIcon::fromTheme("text-x-generic", QIcon(s_mimeIconResourcePath + "text-x-generic"));
					}
				}

				if(!icon.isNull()) {
					child->setIcon(0, icon);
				}
			}
		}
	}


	void ConfigurationWidget::clearAllFileExtensionMIMETypes() {
		m_extensionMIMETypeTree->clear();
		Configuration & opts = m_server->configuration();
		opts.clearAllFileExtensions();
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
		setListenAddress("127.0.0.1");
	}


	void ConfigurationWidget::bindToHostAddress() {
		QString addr;

		/* find first ipv4 address */
		for(const auto & hostAddress : QNetworkInterface::allAddresses()) {
			//		for(const auto & hostAddress : HostNetworkInfo::localHostAddresses(HostNetworkInfo::IPv4)) {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: has address" << qPrintable(hostAddress.toString());

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

		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: bpWebServer::bpWebServerController::bindToHostAddress() - binding to " << qPrintable(addr);
		setListenAddress(addr);
	}


	void ConfigurationWidget::setLiberalDefaultConnectionPolicy() {
		setDefaultConnectionPolicy(Configuration::AcceptConnection);
	}


	void ConfigurationWidget::setRestrictedDefaultConnectionPolicy() {
		setDefaultConnectionPolicy(Configuration::RejectConnection);
	}


	void ConfigurationWidget::setDefaultConnectionPolicy(Configuration::ConnectionPolicy policy) {
		m_accessConfig->setDefaultConnectionPolicy(policy);
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: setting default connection policy directly.\n"
					 << std::flush;
		m_server->configuration().setDefaultConnectionPolicy(policy);
	}


	void ConfigurationWidget::setDefaultMIMEType() {
		setDefaultMIMEType(m_defaultMIMECombo->lineEdit()->text());
	}


	void ConfigurationWidget::setDefaultMIMEType(const QString & mime) {
		Configuration & opts = m_server->configuration();
		opts.setDefaultMIMEType(mime);
	}


	void ConfigurationWidget::setDefaultAction() {
		setDefaultAction(Configuration::WebServerAction(m_defaultActionCombo->itemData(m_defaultActionCombo->currentIndex()).toInt()));
	}


	void ConfigurationWidget::setMIMETypeAction() {
		setMIMETypeAction(m_actionMimeTypeCombo->lineEdit()->text().trimmed(), Configuration::WebServerAction(m_actionActionCombo->itemData(m_actionActionCombo->currentIndex()).toInt()));
	}


	void ConfigurationWidget::setMIMETypeAction(const QString & mime, Configuration::WebServerAction action) {
		Configuration & opts = m_server->configuration();

		if(opts.setMimeTypeAction(mime, action)) {
			int items = m_actionTree->topLevelItemCount();
			QTreeWidgetItem * it = nullptr;
			bool foundMIME = false;
			QString actionText;

			switch(action) {
				case Configuration::Ignore:
					actionText = "Ignore";
					break;

				case Configuration::Serve:
					actionText = "Serve";
					break;

				case Configuration::CGI:
					actionText = "CGI";
					break;

				case Configuration::Forbid:
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
			QIcon icon = QIcon::fromTheme(iconName, QIcon(s_mimeIconResourcePath + iconName + ".png"));

			if(icon.isNull()) {
				/* use generic icons from theme for certain MIME types */
				if("image/" == mime.left(6)) {
					icon = QIcon::fromTheme("image-x-generic", QIcon(s_mimeIconResourcePath + "image-x-generic"));
				}
				else if("audio/" == mime.left(6)) {
					icon = QIcon::fromTheme("audio-x-generic", QIcon(s_mimeIconResourcePath + "audio-x-generic"));
				}
				else if("video/" == mime.left(6)) {
					icon = QIcon::fromTheme("video-x-generic", QIcon(s_mimeIconResourcePath + "video-x-generic"));
				}
				else if("package/" == mime.left(8)) {
					icon = QIcon::fromTheme("package-x-generic", QIcon(s_mimeIconResourcePath + "package-x-generic"));
				}
				else if("text/" == mime.left(5)) {
					icon = QIcon::fromTheme("text-x-generic", QIcon(s_mimeIconResourcePath + "text-x-generic"));
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


	void ConfigurationWidget::removeExtensionMIMEType(QTreeWidgetItem * it) {
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
			opts.removeFileExtensionMIMEType(ext, mime);
		}
		else {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Could not identify the extention and MIME type pair to remove.\n";
		}
	}


	void ConfigurationWidget::setDefaultAction(Configuration::WebServerAction action) {
		Configuration & opts = m_server->configuration();
		opts.setDefaultAction(action);
	}


	//	void ConfigurationWidget::setIPConnectionPolicy() {
	//		setIPConnectionPolicy(m_accessConfig->currentIpAddress().trimmed(), m_accessConfig->currentIpAddressConnectionPolicy());
	//	}


	void ConfigurationWidget::setIPConnectionPolicy(const QString & ip, Configuration::ConnectionPolicy policy) {
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


	void ConfigurationWidget::clearIPConnectionPolicies() {
		m_accessConfig->clearAllConnectionPolicies();
		m_server->configuration().clearAllIpAddressPolicies();
	}


	//	void ConfigurationWidget::logServerAction(const QString & addr, quint16 port, const QString & resource, int action) {
	//		QTreeWidgetItem * logEntry = new QTreeWidgetItem(m_accessLogTabPage);
	//		logEntry->setText(0, addr);
	//		logEntry->setText(1, QString::number(port));
	//		logEntry->setText(2, resource);

	//		switch(action) {
	//			case Configuration::Ignore:
	//				logEntry->setText(3, tr("Ignored"));
	//				break;

	//			case Configuration::Serve:
	//				logEntry->setText(3, tr("Served"));
	//				break;

	//			case Configuration::Forbid:
	//				logEntry->setText(3, tr("Forbidden, not found, or CGI failed"));
	//				break;

	//			case Configuration::CGI:
	//				logEntry->setText(3, tr("Executed through CGI"));
	//				break;

	//			default:
	//				logEntry->setText(3, tr("Unknown Action"));
	//				break;
	//		}
	//	}


	//	void ConfigurationWidget::logServerConnectionPolicy(const QString & addr, quint16 port, Configuration::ConnectionPolicy policy) {
	//		QTreeWidgetItem * logEntry = new QTreeWidgetItem(m_accessLogTabPage);
	//		logEntry->setText(0, addr);
	//		logEntry->setText(1, QString::number(port));

	//		switch(policy) {
	//			case Configuration::AcceptConnection:
	//				logEntry->setText(3, tr("Accepted"));
	//				logEntry->setIcon(3, QIcon(":/icons/connectionpolicies/accept"));
	//				break;

	//			case Configuration::RejectConnection:
	//				logEntry->setText(3, tr("Rejected"));
	//				logEntry->setIcon(3, QIcon(":/icons/connectionpolicies/reject"));
	//				break;

	//			case Configuration::NoConnectionPolicy:
	//				logEntry->setText(3, tr("No Connection Policy"));
	//				break;

	//			default:
	//				logEntry->setText(3, tr("Unknown Connection Policy"));
	//				break;
	//		}
	//	}


	void ConfigurationWidget::extensionTreeSelectedItemChanged(QTreeWidgetItem * it) {
		// update the mime action combos with selected entry
		if(it && it->treeWidget() == m_extensionMIMETypeTree) {
			QTreeWidgetItem * p = it->parent();

			if(p) {
				// it's a MIME type item
				m_fileExtensionCombo->lineEdit()->setText(p->text(0));
				m_extensionMimeTypeCombo->lineEdit()->setText(it->text(0));
			}
			else {
				// it's a file extension item
				m_fileExtensionCombo->lineEdit()->setText(it->text(0));
				m_extensionMimeTypeCombo->lineEdit()->setText("");
			}
		}
	}


	void ConfigurationWidget::mimeActionSelectedItemChanged(QTreeWidgetItem * it) {
		// update the mime action combos with selected entry
		if(it && it->treeWidget() == m_actionTree) {
			m_actionMimeTypeCombo->lineEdit()->setText(it->text(0));
			m_actionActionCombo->setCurrentIndex(m_actionActionCombo->findText(it->text(1)));
		}
	}
}  // namespace EquitWebServer
