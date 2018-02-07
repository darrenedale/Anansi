/** \file ConfigurationWidget.cpp
  * \author darren Hatherley
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
  * \par Current Changes
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

#include "ConfigurationWidget.h"

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

#include "bpIpListWidget.h"
#include "bpEditableTreeWidget.h"
#include "HostNetworkInfo.h"

#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_OK QIcon::fromTheme("task-complete", QIcon(":/icons/status/ok")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING QIcon::fromTheme("task-attention", QIcon(":/icons/status/warning")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_ERROR QIcon::fromTheme("task-attention", QIcon(":/icons/status/error")).pixmap(16)
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_UNKNOWN QIcon("/icons/status/unknown").pixmap(16)

#define EQUITWEBSERVER_CONFIGURATIONWIDGET_MIMEICONRESOURCE_PATH ":/icons/mime/"


QString EquitWebServer::ConfigurationWidget::s_mimeIconResourcePath(EQUITWEBSERVER_CONFIGURATIONWIDGET_MIMEICONRESOURCE_PATH);


EquitWebServer::ConfigurationWidget::ConfigurationWidget( EquitWebServer::Server * server, QWidget * parent )
:	QWidget(parent),
	m_eventsConnected(false),
	m_server(server),
	//m_statusLabel(0),
	m_serverAddressEdit(0),
	m_serverAddressStatus(0),
	m_serverPortWidget(0),
	m_documentRootEdit(0),
	m_documentRootStatus(0),
	m_documentRootSelect(0),
	m_ipEdit(0),
	m_ipPolicyListWidget(0),
	m_ipConnectionPolicyCombo(0),
	m_setIpConnectionPolicyButton(0),
	m_defaultConnectionPolicyCombo(0),
	m_allowDirectoryListing(0),
	m_extensionMIMETypeTree(0),
	m_fileExtensionCombo(0),
	m_extensionMimeTypeCombo(0),
	m_extensionMimeTypeAddButton(0),
	m_actionTree(0),
	m_actionMimeTypeCombo(0),
	m_actionActionCombo(0),
	m_mimeTypeActionSetButton(0),
	m_defaultMIMECombo(0),
	m_defaultActionCombo(0),
	m_accessLogTabPage(0),
	m_serverControlsTab(0) {
	Q_ASSERT(m_server);

	connect(m_server, SIGNAL(requestConnectionPolicyDetermined( QString, quint16, int )), this, SLOT(logServerConnectionPolicy( const QString &, quint16, int )));
	connect(m_server, SIGNAL(requestActionTaken( QString, quint16, QString, int )), this, SLOT(logServerAction( const QString &, quint16, const QString &, int )));

	QLabel * documentRootLabel = new QLabel("&Document Root");
	m_documentRootEdit = new QLineEdit();
	m_documentRootStatus = new QLabel();
	m_documentRootSelect = new QToolButton();
	m_documentRootEdit->setToolTip(tr("The local directory from which files will be served."));
	m_documentRootStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_UNKNOWN);
	m_documentRootStatus->setToolTip("");
	m_documentRootSelect ->setIcon(QIcon::fromTheme("document-open-folder", QIcon(":/icons/buttons/choosedocumentroot")));
	documentRootLabel->setBuddy(m_documentRootEdit);

	m_serverAddressEdit = new QComboBox();
	m_serverAddressStatus = new QLabel();
	m_serverAddressEdit->setEditable(true);
	m_serverAddressEdit->setToolTip(tr("Only IPv4 addresses are currently supported."));
#if QT_VERSION >= 0x040700
	m_serverAddressEdit->lineEdit()->setPlaceholderText(tr("IPv4 address ..."));
#endif
	repopulateAddressItems();
	m_serverAddressEdit->setEditText(m_server->configuration().getListenAddress());
	m_serverAddressStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_UNKNOWN);
	m_serverAddressStatus->setToolTip("");

	m_serverPortWidget = new QSpinBox();
	m_serverPortWidget->setRange(-1, 65535);
	m_serverPortWidget->setSpecialValueText(tr("Default"));
	m_serverPortWidget->setToolTip(tr("The port must be between 0 and 65535. Note that port 80, while it is the default HTTP port, is usually protected on most systems."));

	QHBoxLayout * documentRootLayout = new QHBoxLayout();
	documentRootLayout->addWidget(m_documentRootEdit);
	documentRootLayout->addWidget(m_documentRootSelect);
	documentRootLayout->addWidget(m_documentRootStatus);

	QHBoxLayout * serverAddressLayout = new QHBoxLayout();
	serverAddressLayout->addWidget(m_serverAddressEdit);
	serverAddressLayout->addWidget(m_serverAddressStatus);
	serverAddressLayout->setStretch(0, 100);
	serverAddressLayout->setStretch(1, 0);

	QWidget * mainTabPage = new QWidget();
	QGridLayout * serverConfigLayout = new QGridLayout(mainTabPage);
	serverConfigLayout->addWidget(documentRootLabel, 0, 0);
	serverConfigLayout->addLayout(documentRootLayout, 0, 1);
	serverConfigLayout->addWidget(new QLabel(tr("Listen Address")), 1, 0);
	serverConfigLayout->addLayout(serverAddressLayout, 1, 1);
	serverConfigLayout->addWidget(new QLabel(tr("Listen Port")), 2, 0);
	serverConfigLayout->addWidget(m_serverPortWidget, 2, 1);
	serverConfigLayout->setRowStretch(3,1);

	QWidget * accessControlTabPage = new QWidget();

	m_ipPolicyListWidget = new bpIpListWidget();
	m_ipPolicyListWidget->setToolTip(tr("The policies for HTTP requests from specific IP addresses. These are applied before the default policy is used."));

	QGridLayout * ipAndPolicyLayout = new QGridLayout();
	QHBoxLayout * ipAddressLayout = new QHBoxLayout();
	m_ipEdit = new QLineEdit();
#if QT_VERSION >= 0x040700
	m_ipEdit->setPlaceholderText(tr("Enter an IP address ..."));
#endif
	m_ipEdit->setToolTip(tr("<p>Enter an IP address and choose <strong>Accept</strong> to allow HTTP connections from that IP address, or <strong>Reject</strong> to reject connections from that IP address.</p><p>Choosing <strong>No Policy</strong> will use the default policy.</p>"));
	m_ipConnectionPolicyCombo = new QComboBox();
	m_ipConnectionPolicyCombo->addItem(QIcon(":/icons/connectionpolicies/nopolicy"), tr("No Policy"), int(EquitWebServer::Configuration::NoConnectionPolicy));
	m_ipConnectionPolicyCombo->addItem(QIcon(":/icons/connectionpolicies/accept"), tr("Accept Connection"), int(EquitWebServer::Configuration::AcceptConnection));
	m_ipConnectionPolicyCombo->addItem(QIcon(":/icons/connectionpolicies/reject"), tr("Reject Connection"), int(EquitWebServer::Configuration::RejectConnection));
	m_ipConnectionPolicyCombo->setToolTip(tr("<p>Choose a policy to use for HTTP connections from the IP address in the box to the left. Choosing <strong>No Policy</strong> will use the default policy indicated below.</p><p>Any addresses for which there is no specified policy also follow the default policy.</p>"));
	m_setIpConnectionPolicyButton = new QToolButton();
	m_setIpConnectionPolicyButton->setIcon(QIcon(":/icons/buttons/setippolicy"));
	m_setIpConnectionPolicyButton->setToolTip(tr("Add or update the HTTP connection policy for this IP address."));

	ipAddressLayout->addWidget(m_ipEdit);
	ipAddressLayout->addWidget(m_ipConnectionPolicyCombo);
	ipAddressLayout->addWidget(m_setIpConnectionPolicyButton);

	m_defaultConnectionPolicyCombo = new QComboBox();
	m_defaultConnectionPolicyCombo->addItem(QIcon(":/icons/connectionpolicies/nopolicy"), tr("No Policy"), int(EquitWebServer::Configuration::NoConnectionPolicy));
	m_defaultConnectionPolicyCombo->addItem(QIcon(":/icons/connectionpolicies/accept"), tr("Accept Connection"), int(EquitWebServer::Configuration::AcceptConnection));
	m_defaultConnectionPolicyCombo->addItem(QIcon(":/icons/connectionpolicies/reject"), tr("Reject Connection"), int(EquitWebServer::Configuration::RejectConnection));
	m_defaultConnectionPolicyCombo->setToolTip(tr("<p>Choose the policy to use for HTTP connections from IP addresses that do not have a specific policy, including those for which <strong>No Policy</strong> has been chosen.</p>"));

	QLabel * defaultPolicyLabel = new QLabel(tr("Default &Policy"));
	defaultPolicyLabel->setBuddy(m_defaultConnectionPolicyCombo);
	defaultPolicyLabel->setToolTip(tr("The policy to use for HTTP connections from IP addresses that do not have a specific policy."));

	QLabel * ipAddressLabel = new QLabel(tr("&IP Address Policy"));
	ipAddressLabel->setBuddy(m_ipEdit);
	QFrame * separator;
	ipAndPolicyLayout->addWidget(ipAddressLabel, 0, 0);
	ipAndPolicyLayout->addLayout(ipAddressLayout, 0, 1);
	ipAndPolicyLayout->addWidget(separator = new QFrame(), 1, 0, 1, 2);
	separator->setFrameStyle(QFrame::HLine);
	ipAndPolicyLayout->addWidget(defaultPolicyLabel, 2, 0);
	ipAndPolicyLayout->addWidget(m_defaultConnectionPolicyCombo, 2, 1);

	QVBoxLayout * ipListControlLayout = new QVBoxLayout(accessControlTabPage);
	ipListControlLayout->addWidget(m_ipPolicyListWidget);
	ipListControlLayout->addLayout(ipAndPolicyLayout);

	/* widgets on the content control tab page */
	QWidget * contentControlTabPage = new QWidget();
	QVBoxLayout * contentControlLayout = new QVBoxLayout();
	contentControlTabPage->setLayout(contentControlLayout);
	QSplitter * contentControlSplitter = new QSplitter();
	QWidget * mimeSection = new QWidget();
	QWidget * actionSection = new QWidget();
	mimeSection->setToolTip(tr("<p>This section allows you to associate file extensions with MIME types.</p><p>When a request is received for a resource, this section determines which MIME type is used when processing the request and sending response data.</p>"));
	actionSection->setToolTip(tr("<p>This section allows you to associate server actions with MIME types.</p><p>When a request is received for a resource, and its MIME type has been determined, this section defines what action the web server will take to generate the data for the response. The action can be:</p><ul><li><strong>Serve</strong> The resource (usually a file) will be sent verbatim</li><li><strong>Ignore</strong> The request will be ignored and no data will be sent</li><li><strong>Forbid</strong> The request will be rejected and a \"forbidden\" error response will be sent</li><li><strong>CGI</strong> The resource will be executed through the CGI environment and the output of the executed CGI command will be sent as the response. The CGI command to execute for a MIME type can be set by double-clicking the entry in the list; if no command is set, the resource is considered directly executable.</li></ul>"));
	contentControlSplitter->addWidget(mimeSection);
	contentControlSplitter->addWidget(actionSection);
	m_allowDirectoryListing = new QCheckBox(tr("Allow directory listings"));
	contentControlLayout->addWidget(m_allowDirectoryListing);
	contentControlLayout->addWidget(contentControlSplitter);
	m_extensionMIMETypeTree = new bpEditableTreeWidget();
	m_extensionMIMETypeTree->setColumnCount(1);
	QTreeWidgetItem * mimeTreeHeader = new QTreeWidgetItem();
	mimeTreeHeader->setText(0, tr("MIME Type Associations"));
	m_extensionMIMETypeTree->setHeaderItem(mimeTreeHeader);
	m_fileExtensionCombo = new QComboBox();
	m_fileExtensionCombo->setEditable(true);
	QLabel * fileExtensionLabel = new QLabel(tr("&Extension"));
	fileExtensionLabel->setBuddy(m_fileExtensionCombo);

	m_extensionMimeTypeCombo = new QComboBox();
	m_extensionMimeTypeCombo->setEditable(true);
	QLabel * mimeLabel = new QLabel(tr("&MIME"));
	mimeLabel->setBuddy(m_extensionMimeTypeCombo);

	m_extensionMimeTypeAddButton = new QToolButton();
	m_extensionMimeTypeAddButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/buttons/addextensionmimetype")));

	QHBoxLayout * fileExtensionMIMETypeLayout = new QHBoxLayout();
	fileExtensionMIMETypeLayout->addWidget(fileExtensionLabel);
	fileExtensionMIMETypeLayout->addWidget(m_fileExtensionCombo);
	fileExtensionMIMETypeLayout->addWidget(mimeLabel);
	fileExtensionMIMETypeLayout->addWidget(m_extensionMimeTypeCombo);
	fileExtensionMIMETypeLayout->addWidget(m_extensionMimeTypeAddButton);
	fileExtensionMIMETypeLayout->setStretchFactor(m_fileExtensionCombo, 1);
	fileExtensionMIMETypeLayout->setStretchFactor(m_extensionMimeTypeCombo, 2);

	QHBoxLayout * defaultMIMETypeLayout = new QHBoxLayout();
	m_defaultMIMECombo = new QComboBox();
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

	QHBoxLayout * actionControlLayout = new QHBoxLayout();
	m_actionMimeTypeCombo = new QComboBox();
	m_actionMimeTypeCombo->setEditable(true);
	QLabel * actionMimeLabel = new QLabel(tr("MIME"));
	actionMimeLabel->setBuddy(m_actionMimeTypeCombo);

	m_actionActionCombo = new QComboBox();
	m_actionActionCombo->addItem(tr("Ignore"), EquitWebServer::Configuration::Ignore);
	m_actionActionCombo->addItem(tr("Serve"), EquitWebServer::Configuration::Serve);
	m_actionActionCombo->addItem(tr("CGI"), EquitWebServer::Configuration::CGI);
	m_actionActionCombo->addItem(tr("Forbid"), EquitWebServer::Configuration::Forbid);
	m_mimeTypeActionSetButton = new QToolButton();
	m_mimeTypeActionSetButton->setIcon(QIcon(":/icons/buttons/setmimetypeaction"));

	actionControlLayout->addWidget(actionMimeLabel);
	actionControlLayout->addWidget(m_actionMimeTypeCombo);
	actionControlLayout->addWidget(m_actionActionCombo);
	actionControlLayout->addWidget(m_mimeTypeActionSetButton);
	actionControlLayout->setStretchFactor(m_actionMimeTypeCombo, 2);
	actionControlLayout->setStretchFactor(m_actionActionCombo, 1);

	QVBoxLayout * actionLayout = new QVBoxLayout(actionSection);
	m_actionTree = new bpEditableTreeWidget();
	m_actionTree->setColumnCount(3);
	m_actionTree->setItemDelegateForColumn(2, new QItemDelegate(this));
	QTreeWidgetItem * actionHeader = new QTreeWidgetItem();
	actionHeader->setText(0, tr("MIME Type"));
	actionHeader->setText(1, tr("Action"));
	actionHeader->setText(2, tr("CGI Executable"));
	m_actionTree->setHeaderItem(actionHeader);
	m_actionTree->setRootIsDecorated(false);

	QHBoxLayout * defaultActionLayout = new QHBoxLayout();
	m_defaultActionCombo = new QComboBox();
	m_defaultActionCombo->addItem(tr("Ignore"), EquitWebServer::Configuration::Ignore);
	m_defaultActionCombo->addItem(tr("Serve"), EquitWebServer::Configuration::Serve);
	m_defaultActionCombo->addItem(tr("CGI"), EquitWebServer::Configuration::CGI);
	m_defaultActionCombo->addItem(tr("Forbid"), EquitWebServer::Configuration::Forbid);
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

	m_accessLogTabPage = new QTreeWidget();
	QTreeWidgetItem * accessLogHeader = new QTreeWidgetItem();
	m_accessLogTabPage->setHeaderItem(accessLogHeader);
	accessLogHeader->setText(0, tr("Remote IP"));
	accessLogHeader->setText(1, tr("Remote Port"));
	accessLogHeader->setText(2, tr("Resource Requested"));
	accessLogHeader->setText(3, tr("Response/Action"));
	m_accessLogTabPage->setRootIsDecorated(false);

	m_serverControlsTab = new QTabWidget();
	m_serverControlsTab->addTab(mainTabPage, QIcon::fromTheme("network-server", QIcon(":/icons/tabs/server")), tr("Server"));
	m_serverControlsTab->setTabToolTip(0, tr("The main server setup."));
	m_serverControlsTab->addTab(accessControlTabPage, QIcon(":/icons/tabs/accesscontrol"), tr("Access Control"));
	m_serverControlsTab->setTabToolTip(1, tr("Tell the server what to do with HTTP connections from different IP addresses."));
	m_serverControlsTab->addTab(contentControlTabPage, QIcon(":/icons/tabs/contentcontrol"), tr("Content Control"));
	m_serverControlsTab->setTabToolTip(2, tr("Tell the server how to handle requests for different types of resources."));
	m_serverControlsTab->addTab(m_accessLogTabPage, QIcon(":/icons/tabs/accesslog"), tr("Access Log"));
	m_serverControlsTab->setTabToolTip(3, tr("View the server access log."));

	QVBoxLayout * mainLayout = new QVBoxLayout();
	mainLayout->addWidget(m_serverControlsTab);

	readConfiguration();	// this ensures events are connected
	updateDocumentRootStatusIndicator();
	updateListenAddressStatusIndicator();

	/* now config has been read and lists have been populated, make the columns
	  a good width */
	m_extensionMIMETypeTree->resizeColumnToContents(0);
	m_actionTree->resizeColumnToContents(0);
	m_actionTree->resizeColumnToContents(1);
	m_actionTree->resizeColumnToContents(2);

	/* finally, add the layout to the window so it is shown when the window
	  is opened */
	setLayout(mainLayout);
}


EquitWebServer::ConfigurationWidget::~ConfigurationWidget( void ) {
}


void EquitWebServer::ConfigurationWidget::connectEvents( void ) {
	if(!m_eventsConnected) {
		// basic configuration
		connect(m_documentRootSelect, SIGNAL(clicked()), this, SLOT(selectDocumentRoot()));
		connect(m_documentRootEdit, SIGNAL(textEdited(QString)), this, SLOT(setDocumentRoot(QString)));
		connect(m_serverAddressEdit, SIGNAL(editTextChanged(QString)), this, SLOT(setListenAddress(QString)));
		connect(m_serverPortWidget, SIGNAL(valueChanged(int)), this, SLOT(setListenPort(int)));

		// access Controls
		connect(m_ipPolicyListWidget, SIGNAL(ipAddressRemoved( QString )), this, SLOT(ipPolicyRemoved(QString)));
		connect(m_ipPolicyListWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(ipPolicySelectedItemChanged( QTreeWidgetItem * )));
		connect(m_setIpConnectionPolicyButton, SIGNAL(clicked()), this, SLOT(setIPConnectionPolicy()));

		connect(m_defaultConnectionPolicyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultConnectionPolicy()));

		// content controls
		connect(m_allowDirectoryListing, SIGNAL(toggled(bool)), this, SLOT(setAllowDirectoryListing(bool)));

		connect(m_extensionMIMETypeTree, SIGNAL(removingItem( QTreeWidgetItem * )), this, SLOT(removeExtensionMIMEType( QTreeWidgetItem * )));
		connect(m_extensionMIMETypeTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(extensionTreeSelectedItemChanged( QTreeWidgetItem * )));
		connect(m_extensionMimeTypeAddButton, SIGNAL(clicked()), this, SLOT(addFileExtensionMIMEType()));
		connect(m_mimeTypeActionSetButton, SIGNAL(clicked()), this, SLOT(setMIMETypeAction()));

		connect(m_actionTree, SIGNAL(removingItem( QTreeWidgetItem * )), this, SLOT(removeAction( QTreeWidgetItem * )));
		connect(m_actionTree, SIGNAL(itemDoubleClicked( QTreeWidgetItem *, int  )), this, SLOT(actionDoubleClicked( QTreeWidgetItem * )));
		connect(m_actionTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(mimeActionSelectedItemChanged(QTreeWidgetItem *)));

		connect(m_defaultMIMECombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultMIMEType()));
		connect(m_defaultActionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultAction()));
		m_eventsConnected = true;
	}
}


void EquitWebServer::ConfigurationWidget::disconnectEvents( void ) {
	if(m_eventsConnected) {
		// basic configuration
		disconnect(m_documentRootSelect, SIGNAL(clicked()), this, SLOT(selectDocumentRoot()));
		disconnect(m_documentRootEdit, SIGNAL(textEdited(QString)), this, SLOT(setDocumentRoot(QString)));
		disconnect(m_serverAddressEdit, SIGNAL(editTextChanged(QString)), this, SLOT(setListenAddress(QString)));
		disconnect(m_serverPortWidget, SIGNAL(valueChanged(int)), this, SLOT(setListenPort(int)));

		// access Controls
		disconnect(m_ipPolicyListWidget, SIGNAL(ipAddressRemoved( QString )), this, SLOT(ipPolicyRemoved(QString)));
		disconnect(m_ipPolicyListWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(ipPolicySelectedItemChanged( QTreeWidgetItem * )));
		disconnect(m_setIpConnectionPolicyButton, SIGNAL(clicked()), this, SLOT(setIPConnectionPolicy()));

		disconnect(m_defaultConnectionPolicyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultConnectionPolicy()));

		// content controls
		disconnect(m_allowDirectoryListing, SIGNAL(toggled(bool)), this, SLOT(setAllowDirectoryListing(bool)));

		disconnect(m_extensionMIMETypeTree, SIGNAL(removingItem( QTreeWidgetItem * )), this, SLOT(removeExtensionMIMEType( QTreeWidgetItem * )));
		disconnect(m_extensionMIMETypeTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(extensionTreeSelectedItemChanged( QTreeWidgetItem * )));
		disconnect(m_extensionMimeTypeAddButton, SIGNAL(clicked()), this, SLOT(addFileExtensionMIMEType()));
		disconnect(m_mimeTypeActionSetButton, SIGNAL(clicked()), this, SLOT(setMIMETypeAction()));

		disconnect(m_actionTree, SIGNAL(removingItem( QTreeWidgetItem * )), this, SLOT(removeAction( QTreeWidgetItem * )));
		disconnect(m_actionTree, SIGNAL(itemDoubleClicked( QTreeWidgetItem *, int  )), this, SLOT(actionDoubleClicked( QTreeWidgetItem * )));
		disconnect(m_actionTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(mimeActionSelectedItemChanged(QTreeWidgetItem *)));

		disconnect(m_defaultMIMECombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultMIMEType()));
		disconnect(m_defaultActionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultAction()));
		m_eventsConnected = false;
	}
}


void EquitWebServer::ConfigurationWidget::repopulateAddressItems( void ) {
	Q_ASSERT(m_serverAddressEdit);

	m_serverAddressEdit->clear();

	/* for now, we only support ipv4 addresses */
	foreach(QHostAddress ha, HostNetworkInfo::localHostAddresses(HostNetworkInfo::IPv4))
		m_serverAddressEdit->addItem(ha.toString());
}


void EquitWebServer::ConfigurationWidget::updateDocumentRootStatusIndicator( void ) {
	QFileInfo fi(m_documentRootEdit->text());

	if(!fi.exists()) {
		m_documentRootStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING);
		m_documentRootStatus->setToolTip(tr("The path set for the document root does not exist."));
		m_documentRootStatus->setVisible(true);
	}
	else if(!fi.isDir()) {
		m_documentRootStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING);
		m_documentRootStatus->setToolTip(tr("The path set for the document root is not a directory."));
		m_documentRootStatus->setVisible(true);
	}
	else if(!fi.isReadable()) {
		m_documentRootStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING);
		m_documentRootStatus->setToolTip(tr("The path set for the document root is not readable."));
		m_documentRootStatus->setVisible(true);
	}
	else {
		m_documentRootStatus->setVisible(false);
//		m_documentRootStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_OK);
//		m_documentRootStatus->setToolTip("");
	}

}


void EquitWebServer::ConfigurationWidget::updateListenAddressStatusIndicator( void ) {
	static QRegExp s_ip4Validator(" *([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3}) *");

	QString ip(m_serverAddressEdit->currentText());

	if(!s_ip4Validator.exactMatch(ip) ) {
		m_serverAddressStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING);
		m_serverAddressStatus->setToolTip(tr("The listen address you have entered is not a valid IPv4 address."));
		m_serverAddressStatus->setVisible(true);
	}
	else if(4 != s_ip4Validator.captureCount() ||
		1 > s_ip4Validator.cap(1).toInt() || 255 < s_ip4Validator.cap(1).toInt() ||
		0 > s_ip4Validator.cap(2).toInt() || 255 < s_ip4Validator.cap(2).toInt() ||
		0 > s_ip4Validator.cap(3).toInt() || 255 < s_ip4Validator.cap(3).toInt() ||
		0 > s_ip4Validator.cap(4).toInt() || 255 < s_ip4Validator.cap(4).toInt()) {
		m_serverAddressStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING);
		m_serverAddressStatus->setToolTip(tr("The listen address you have entered is not a valid IPv4 address."));
		m_serverAddressStatus->setVisible(true);
	}
	else if(!HostNetworkInfo::localHostAddresses(HostNetworkInfo::IPv4).contains(QHostAddress(ip.trimmed()))) {
		m_serverAddressStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_WARNING);
		m_serverAddressStatus->setToolTip(tr("The listen address you have entered does not appear to be an address that belongs to this device."));
		m_serverAddressStatus->setVisible(true);
	}
	else {
		m_serverAddressStatus->setVisible(false);
//		m_serverAddressStatus->setPixmap(EQUITWEBSERVER_CONFIGURATIONWIDGET_STATUSICON_OK);
//		m_serverAddressStatus->setToolTip("");
	}

}


void EquitWebServer::ConfigurationWidget::setServer( Server * server ) {
	m_server = server;
	if(m_server) readConfiguration();
	else setEnabled(false);
}


void EquitWebServer::ConfigurationWidget::readConfiguration( void ) {
	Q_ASSERT(m_server);
	Q_ASSERT(m_extensionMIMETypeTree);
	Q_ASSERT(m_extensionMimeTypeCombo);
	Q_ASSERT(m_fileExtensionCombo);
	Q_ASSERT(m_actionTree);
	Q_ASSERT(m_actionMimeTypeCombo);
	Q_ASSERT(m_defaultMIMECombo);
	Q_ASSERT(m_actionActionCombo);
	Q_ASSERT(m_defaultConnectionPolicyCombo);
	Q_ASSERT(m_serverPortWidget);
	Q_ASSERT(m_allowDirectoryListing);

	disconnectEvents();
	EquitWebServer::Configuration & opts = m_server->configuration();
	m_serverPortWidget->setValue(opts.port());
	m_documentRootEdit->setText(opts.getDocumentRoot());
	m_serverAddressEdit->setEditText(opts.getListenAddress());

	// read ip policy configuration
	m_ipPolicyListWidget->clear();

	{
		foreach(QString ip, opts.getRegisteredIPAddressList()) {
			QTreeWidgetItem * item = new QTreeWidgetItem(m_ipPolicyListWidget);
			item->setText(0, ip);

			switch(opts.getIPAddressPolicy(ip)) {
				default:
				case EquitWebServer::Configuration::NoConnectionPolicy:
					item->setText(1, "No Policy");
					break;

				case EquitWebServer::Configuration::RejectConnection:
					item->setText(1, "Reject Connection");
					item->setIcon(1, QIcon(":/icons/connectionpolicies/reject"));
					break;

				case EquitWebServer::Configuration::AcceptConnection:
					item->setText(1, "Accept Connection");
					item->setIcon(1, QIcon(":/icons/connectionpolicies/accept"));
					break;
			}
		}
	}

	m_allowDirectoryListing->setChecked(opts.isDirectoryListingAllowed());
	m_extensionMIMETypeTree->clear();

	QVector<QString> allMimes;

	// read mime type extension mappings
	{
		QStringList extensions = opts.getRegisteredFileExtensions();
		QStringList::iterator i = extensions.begin();

		while(i != extensions.end()) {
			m_fileExtensionCombo->addItem(*i);
			QTreeWidgetItem * item = new QTreeWidgetItem(m_extensionMIMETypeTree);
			item->setText(0, *i);

			QVector<QString> types = opts.getMIMETypesForFileExtension(*i);

			QVector<QString>::iterator iMime = types.begin();

			while(iMime != types.end()) {
				QTreeWidgetItem * child = new QTreeWidgetItem(item);
				child->setText(0, *iMime);
				QString iconName(*iMime);
				iconName.replace('/', '-');
				QIcon icon = QIcon::fromTheme(iconName, QIcon(s_mimeIconResourcePath + iconName + ".png"));

				if(icon.isNull()) {
					/* use generic icons from theme for certain MIME types */
					if("image/" == (*iMime).left(6)) icon = QIcon::fromTheme("image-x-generic", QIcon(s_mimeIconResourcePath + "image-x-generic"));
					else if("audio/" == (*iMime).left(6)) icon = QIcon::fromTheme("audio-x-generic", QIcon(s_mimeIconResourcePath + "audio-x-generic"));
					else if("video/" == (*iMime).left(6)) icon = QIcon::fromTheme("video-x-generic", QIcon(s_mimeIconResourcePath + "video-x-generic"));
					else if("package/" == (*iMime).left(8)) icon = QIcon::fromTheme("package-x-generic", QIcon(s_mimeIconResourcePath + "package-x-generic"));
					else if("text/" == (*iMime).left(5)) icon = QIcon::fromTheme("text-x-generic", QIcon(s_mimeIconResourcePath + "text-x-generic"));
				}

				if(!icon.isNull()) child->setIcon(0, icon);

				if(!allMimes.contains(*iMime))
					allMimes << (*iMime);

				iMime++;
			}

			i++;
		}
	}

	// read mime type actions
	m_actionTree->clear();

	{
		QStringList mimes = opts.getRegisteredMIMETypes();
		qDebug() << mimes.count() << " MIME Types with registered actions.";
		QStringList::iterator i = mimes.begin();

		while(i != mimes.end()) {
			QTreeWidgetItem * item = new QTreeWidgetItem(m_actionTree);
			item->setText(0, (*i));
			QString iconName(*i);
			iconName.replace('/', '-');
qDebug() << "icon name:" << iconName;
qDebug() << "   fallback icon resource path:" << (s_mimeIconResourcePath + iconName + ".png");
			QIcon icon(QIcon::fromTheme(iconName, QIcon(s_mimeIconResourcePath + iconName + ".png")));
qDebug() << "   found icon: " << (icon.isNull() ? "no" : "yes");

			if(icon.isNull()) {
				/* use generic icons from theme for certain MIME types */
				if("image/" == (*i).left(6)) icon = QIcon::fromTheme("image-x-generic");
				else if("audio/" == (*i).left(6)) icon = QIcon::fromTheme("audio-x-generic");
				else if("video/" == (*i).left(6)) icon = QIcon::fromTheme("video-x-generic");
				else if("package/" == (*i).left(8)) icon = QIcon::fromTheme("package-x-generic");
				else if("text/" == (*i).left(5)) icon = QIcon::fromTheme("text-x-generic");

qDebug() << "   found generic icon without using MIME subtype: " << (icon.isNull() ? "no" : "yes");
			}

			if(!icon.isNull()) item->setIcon(0, icon);

			switch(opts.getMIMETypeAction(*i)) {
				case EquitWebServer::Configuration::Ignore:
					item->setText(1, "Ignore");
					break;

				case EquitWebServer::Configuration::Serve:
					item->setText(1, "Serve");
					break;

				case EquitWebServer::Configuration::CGI:
					item->setText(1, "CGI");
					item->setText(2, opts.getMIMETypeCGI(*i));
					break;

				case EquitWebServer::Configuration::Forbid:
					item->setText(1, "Forbid");
					break;

				default:
					item->setText(1, "Unknown");
					break;
			}

			if(!allMimes.contains(*i))
				allMimes << (*i);

			i++;
		}
	}

	QString defaultMime = opts.getDefaultMIMEType();

	if(!defaultMime.isEmpty() && !allMimes.contains(defaultMime))
		allMimes << defaultMime;

	// populate all MIME type combos with known MIME types
	m_actionMimeTypeCombo->clear();
	m_extensionMimeTypeCombo->clear();
	m_defaultMIMECombo->clear();

	foreach(QString mime, allMimes) {
		m_actionMimeTypeCombo->addItem(mime);
		m_extensionMimeTypeCombo->addItem(mime);
		m_defaultMIMECombo->addItem(mime);
	}

	m_fileExtensionCombo->lineEdit()->setText("");
	m_extensionMimeTypeCombo->lineEdit()->setText("");
	m_actionMimeTypeCombo->lineEdit()->setText("");
	m_actionMimeTypeCombo->lineEdit()->setText("");

	m_defaultActionCombo->setCurrentIndex(m_actionActionCombo->findData(opts.getDefaultAction()));
	m_defaultConnectionPolicyCombo->setCurrentIndex(m_defaultConnectionPolicyCombo->findData(opts.getDefaultConnectionPolicy()));
	m_defaultMIMECombo->lineEdit()->setText(defaultMime);
	connectEvents();
	setEnabled(true);
}


void EquitWebServer::ConfigurationWidget::actionDoubleClicked( QTreeWidgetItem * it ) {
	if(!it || it->treeWidget() != m_actionTree) {
		qDebug() << "bpWebServer::bpWebServerController::actionDoubleClicked() - received no item or item that does not belong to action list.";
		return;
	}

	EquitWebServer::Configuration & opts = m_server->configuration();
	QString mime = it->text(0);
	EquitWebServer::Configuration::WebServerAction action = opts.getMIMETypeAction(mime);

	if(action != EquitWebServer::Configuration::CGI) {
		if(QMessageBox::question(this, "Set CGI Executable", tr("The action for the MIME type '%1' is not set to CGI. Should the web server alter the action for this MIME type to CGI?").arg(mime), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		if(!opts.setMIMETypeAction(mime, EquitWebServer::Configuration::CGI)) {
			QMessageBox::critical(this, "Set CGI Executable", tr("The action for the MIME type '%1' could not be set to CGI.").arg(mime));
			return;
		}
	}

	/* work out the initial path to navigate to in the file select dialogue */
	QString initialPath = it->text(2);

	/* if the item clicked does not already have a CGI exe listed, use the
	   platform-specific default programs directory */
	if(initialPath.isEmpty()) {
#if defined(Q_OS_MACX)
		initialPath = "/Applications/";
#elif defined(Q_OS_WIN32)
		/* use QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation) ?*/
		initialPath = getenv("PROGRAMFILES");
#elif defined(Q_OS_LINUX) || defined(Q_OS_SOLARIS) || defined(Q_OS_UNIX) || defined(Q_OS_BSD4)
		/* should this be /usr/local/bin ? */
		initialPath = "/usr/bin/";
#else
		initialPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation);
#endif
	}

	QString newCGI = QFileDialog::getOpenFileName(this, "Set CGI Executable", initialPath);

	if(!newCGI.isNull()) {
		opts.setMIMETypeCGI(mime, newCGI);
		it->setText(2, newCGI);
		m_server->setConfiguration(opts);
	}
}


void EquitWebServer::ConfigurationWidget::clearAllActions( void ) {
	m_actionTree->clear();
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.clearAllMIMETypeActions();
}


void EquitWebServer::ConfigurationWidget::addFileExtensionMIMEType( void ) {
	QString ext = m_fileExtensionCombo->lineEdit()->text().trimmed();
	QString mime = m_extensionMimeTypeCombo->lineEdit()->text().trimmed();
	if(ext.startsWith(".")) ext = ext.right(ext.size() - 1);
	EquitWebServer::Configuration & opts = m_server->configuration();

	if(opts.addFileExtensionMIMEType(ext, mime)) {
		int items = m_extensionMIMETypeTree->topLevelItemCount();
		QTreeWidgetItem * it;
		bool addedMIME = false;

		for(int i = 0; i < items; i++) {
			if((it = m_extensionMIMETypeTree->topLevelItem(i)) && it->text(0) == ext) {
				QTreeWidgetItem * child = new QTreeWidgetItem(it);
				child->setText(0, mime);
				addedMIME = true;
				break;	/* exit for loop */
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
				if("image/" == mime.left(6)) icon = QIcon::fromTheme("image-x-generic", QIcon(s_mimeIconResourcePath + "image-x-generic"));
				else if("audio/" == mime.left(6)) icon = QIcon::fromTheme("audio-x-generic", QIcon(s_mimeIconResourcePath + "audio-x-generic"));
				else if("video/" == mime.left(6)) icon = QIcon::fromTheme("video-x-generic", QIcon(s_mimeIconResourcePath + "video-x-generic"));
				else if("package/" == mime.left(8)) icon = QIcon::fromTheme("package-x-generic", QIcon(s_mimeIconResourcePath + "package-x-generic"));
				else if("text/" == mime.left(5)) icon = QIcon::fromTheme("text-x-generic", QIcon(s_mimeIconResourcePath + "text-x-generic"));
			}

			if(!icon.isNull()) child->setIcon(0, icon);
		}
	}
}


void EquitWebServer::ConfigurationWidget::clearAllFileExtensionMIMETypes( void ) {
	m_extensionMIMETypeTree->clear();
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.clearAllFileExtensions();
}


void EquitWebServer::ConfigurationWidget::disableWidgets( void ) {
	m_serverAddressEdit->setEnabled(false);
	m_serverPortWidget->setEnabled(false);
	m_documentRootEdit->setEnabled(false);
	m_documentRootSelect->setEnabled(false);
}


void EquitWebServer::ConfigurationWidget::enableWidgets( void ) {
	m_serverAddressEdit->setEnabled(true);
	m_serverPortWidget->setEnabled(true);
	m_documentRootEdit->setEnabled(true);
	m_documentRootSelect->setEnabled(true);
}


void EquitWebServer::ConfigurationWidget::selectDocumentRoot( void ) {
	QString initialPath = m_documentRootEdit->text();

	QDir d(initialPath);

	/* d.isRoot() is our condition to tell if the whole path is non-existent,
	 * so we have to skip the test if the path is already the root so that
	 *we don't open the file select dialogue at the user's home directory
	 *if they have actually entered the root directory in the line edit */
	if(!d.isRoot()) {
		/* continually trim off parts of the path until it exists */
		while(!d.exists() && !d.isRoot()) d.cdUp();
		if(d.isRoot()) initialPath = QDir::homePath();
	}

	QString newRoot = QFileDialog::getExistingDirectory(this, "Choose the document root", initialPath);
	if(!newRoot.isNull()) setDocumentRoot(newRoot);
}


///TODO connecting this to QLineEdit::textEdited() is a bit heavy-handed because it makes the server restart
///on every keypress and/or executes several copy operations on the bpWebServer::Configuration object on every
///keypress. migrate to getConfiguration() returning a pointer to the actual options, and/or change to use
///the editingFinished() signal
void EquitWebServer::ConfigurationWidget::setDocumentRoot( const QString & r ) {
	if(r.isEmpty())
		return;

	/* if we don't do this check, the widget will always be updated, which will mean
	 * that the cursor will be moved to the end of the widget on every keypress */
	if(m_documentRootEdit->text() != r) {
		m_documentRootEdit->setText(r);
		emit(documentRootChanged(m_documentRootEdit->text()));
	}

	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.setDocumentRoot(r);
	updateDocumentRootStatusIndicator();
}


void EquitWebServer::ConfigurationWidget::setAllowDirectoryListing( bool allow ) {
	m_allowDirectoryListing->setChecked(allow);
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.setAllowDirectoryListing(allow);
}


void EquitWebServer::ConfigurationWidget::setListenAddress( const QString & addr ) {
	if(addr.isEmpty())
		return;

	if(addr != m_serverAddressEdit->currentText()) m_serverAddressEdit->setEditText(addr);
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.setListenAddress(addr);

	updateListenAddressStatusIndicator();
}


void EquitWebServer::ConfigurationWidget::setListenPort( int port ) {
	m_serverPortWidget->setValue(port);
	EquitWebServer::Configuration & opts = m_server->configuration();
	if(-1 == port) port = Configuration::DefaultPort;
	opts.setPort(port);
}


void EquitWebServer::ConfigurationWidget::bindToLocalhost( void ) {
	setListenAddress("127.0.0.1");
}


void EquitWebServer::ConfigurationWidget::bindToHostAddress( void ) {
	QString addr;

	/* find first ipv4 address */
	foreach(QHostAddress ha, HostNetworkInfo::localHostAddresses(HostNetworkInfo::IPv4)) {
		qDebug() << "has address" << qPrintable(ha.toString());
		if(ha.toString().startsWith("127.")) continue;

		if(QAbstractSocket::IPv4Protocol == ha.protocol()) {
			addr = ha.toString();
			break;
		}
	}

	if(addr.isNull()) {
		QMessageBox::critical(this, tr("Listen on host address"), tr("This computer does not appear to have any IPv4 addresses."));
		return;
	}

	qDebug() << "bpWebServer::bpWebServerController::bindToHostAddress() - binding to " << qPrintable(addr);
	setListenAddress(addr);
}


void EquitWebServer::ConfigurationWidget::setDefaultConnectionPolicy( void ) {
	setDefaultConnectionPolicy(EquitWebServer::Configuration::ConnectionPolicy(m_defaultConnectionPolicyCombo->itemData(m_defaultConnectionPolicyCombo->currentIndex()).toInt()));
}


void EquitWebServer::ConfigurationWidget::setLiberalDefaultConnectionPolicy( void ) {
	setDefaultConnectionPolicy(EquitWebServer::Configuration::AcceptConnection);
}


void EquitWebServer::ConfigurationWidget::setRestrictedDefaultConnectionPolicy( void ) {
	setDefaultConnectionPolicy(EquitWebServer::Configuration::RejectConnection);
}



void EquitWebServer::ConfigurationWidget::setDefaultConnectionPolicy( EquitWebServer::Configuration::ConnectionPolicy p ) {
	m_defaultConnectionPolicyCombo->setCurrentIndex(m_defaultConnectionPolicyCombo->findData(p));
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.setDefaultConnectionPolicy(p);
}


void EquitWebServer::ConfigurationWidget::setDefaultMIMEType( void ) {
	setDefaultMIMEType(m_defaultMIMECombo->lineEdit()->text());
}


void EquitWebServer::ConfigurationWidget::setDefaultMIMEType( const QString & mime ) {
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.setDefaultMIMEType(mime);
}


void EquitWebServer::ConfigurationWidget::setDefaultAction( void ) {
	setDefaultAction(EquitWebServer::Configuration::WebServerAction(m_defaultActionCombo->itemData(m_defaultActionCombo->currentIndex()).toInt()));
}


void EquitWebServer::ConfigurationWidget::setMIMETypeAction( void ) {
	setMIMETypeAction(m_actionMimeTypeCombo->lineEdit()->text().trimmed(), EquitWebServer::Configuration::WebServerAction(m_actionActionCombo->itemData(m_actionActionCombo->currentIndex()).toInt()));
}


void EquitWebServer::ConfigurationWidget::setMIMETypeAction( const QString & mime, EquitWebServer::Configuration::WebServerAction action ) {
	EquitWebServer::Configuration & opts = m_server->configuration();

	if(opts.setMIMETypeAction(mime, action)) {
		int items = m_actionTree->topLevelItemCount();
		QTreeWidgetItem * it = 0;
		bool foundMIME = false;
		QString actionText;

		switch(action) {
			case EquitWebServer::Configuration::Ignore:
				actionText = "Ignore";
				break;

			case EquitWebServer::Configuration::Serve:
				actionText = "Serve";
				break;

			case EquitWebServer::Configuration::CGI:
				actionText = "CGI";
				break;

			case EquitWebServer::Configuration::Forbid:
				actionText = "Forbid";
				break;

			default:
				actionText = "Unknown";
				break;
		}

		for(int i = 0; i < items && !foundMIME; i++) {
			if((it = m_actionTree->topLevelItem(i)) && it->text(0) == mime)
				foundMIME = true;
		}

		if(!foundMIME)
			it = new QTreeWidgetItem(m_actionTree);

		QString iconName(mime);
		iconName.replace('/', '-');
		QIcon icon = QIcon::fromTheme(iconName, QIcon(s_mimeIconResourcePath + iconName + ".png"));

		if(icon.isNull()) {
			/* use generic icons from theme for certain MIME types */
			if("image/" == mime.left(6)) icon = QIcon::fromTheme("image-x-generic", QIcon(s_mimeIconResourcePath + "image-x-generic"));
			else if("audio/" == mime.left(6)) icon = QIcon::fromTheme("audio-x-generic", QIcon(s_mimeIconResourcePath + "audio-x-generic"));
			else if("video/" == mime.left(6)) icon = QIcon::fromTheme("video-x-generic", QIcon(s_mimeIconResourcePath + "video-x-generic"));
			else if("package/" == mime.left(8)) icon = QIcon::fromTheme("package-x-generic", QIcon(s_mimeIconResourcePath + "package-x-generic"));
			else if("text/" == mime.left(5)) icon = QIcon::fromTheme("text-x-generic", QIcon(s_mimeIconResourcePath + "text-x-generic"));
		}

		it->setText(0, mime);
		it->setText(1, actionText);
		if(!icon.isNull()) it->setIcon(0, icon);
	}
}


void EquitWebServer::ConfigurationWidget::removeAction( QTreeWidgetItem * it ) {
	if(it) {
		QString mime = it->text(0);
		qDebug() << QString("Clearing action for MIME type '%1'.").arg(mime).toAscii().constData();
		EquitWebServer::Configuration & opts = m_server->configuration();
		opts.unsetMIMETypeAction(mime);
	}
	else
		qDebug() << "Could not identify the MIME type from which to remove the action.";
}


void EquitWebServer::ConfigurationWidget::removeExtensionMIMEType(  QTreeWidgetItem * it ) {
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

		qDebug() << QString("Clearing MIME type '%1' from extension '%2'.").arg(mime).arg(ext).toAscii().constData();
		EquitWebServer::Configuration & opts = m_server->configuration();
		opts.removeFileExtensionMIMEType(ext,mime);
	}
	else
		qDebug() << "Could not identify the extention and MIME type pair to remove.";
}


void EquitWebServer::ConfigurationWidget::setDefaultAction( EquitWebServer::Configuration::WebServerAction action ) {
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.setDefaultAction(action);
}


void EquitWebServer::ConfigurationWidget::setIPConnectionPolicy( void ) {
	setIPConnectionPolicy(m_ipEdit->text().trimmed(), EquitWebServer::Configuration::ConnectionPolicy(m_ipConnectionPolicyCombo->itemData(m_ipConnectionPolicyCombo->currentIndex()).toInt()));
}


void EquitWebServer::ConfigurationWidget::setIPConnectionPolicy( const QString & ip, EquitWebServer::Configuration::ConnectionPolicy p ) {
	EquitWebServer::Configuration & opts = m_server->configuration();
	QString policy;
	QIcon icon;

	switch(p) {
		case EquitWebServer::Configuration::AcceptConnection:
			policy = "Accept Connection";
			icon = QIcon(":/icons/connectionpolicies/accept");
			break;

		case EquitWebServer::Configuration::RejectConnection:
			policy = "Reject Connection";
			icon = QIcon(":/icons/connectionpolicies/reject");
			break;

		case EquitWebServer::Configuration::NoConnectionPolicy:
		default:
			policy = "No Policy";
			break;
	}

	if(opts.setIPAddressPolicy(ip, p)) {
		QList<QTreeWidgetItem *> items = m_ipPolicyListWidget->findItems(ip, Qt::MatchCaseSensitive);

		if(items.count() == 0) {
			QTreeWidgetItem * it = new QTreeWidgetItem(m_ipPolicyListWidget);
			it->setText(0, ip);
			it->setText(1, policy);
			if(!icon.isNull()) it->setIcon(1, icon);
		}
		else {
			QList<QTreeWidgetItem *>::iterator it = items.begin();

			while(it != items.end()) {
				(*it)->setText(1, policy);
				(*it)->setIcon(1, icon);
				it++;
			}
		}

		opts.setIPAddressPolicy(ip, p);
		return;
	}

	QMessageBox::warning(this, tr("Set IP Address Connection Policy"), tr("The connection policy for '%1' could not be set.").arg(ip));
}


void EquitWebServer::ConfigurationWidget::ipPolicyRemoved( const QString & ip ) {
	qDebug() << "bpWebServer::bpWebServerController::ipPolicyRemoved(" << ip.toAscii().constData() << ")";
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.clearIPAddressPolicy(ip);
}


void EquitWebServer::ConfigurationWidget::clearIPConnectionPolicies( void ) {
	m_ipPolicyListWidget->clear();
	EquitWebServer::Configuration & opts = m_server->configuration();
	opts.clearAllIPAddressPolicies();
}


void EquitWebServer::ConfigurationWidget::logServerAction( const QString & addr, quint16 port, const QString & resource, int action ) {
	QTreeWidgetItem * logEntry = new QTreeWidgetItem(m_accessLogTabPage);
	logEntry->setText(0, addr);
	logEntry->setText(1, QString::number(port));
	logEntry->setText(2, resource);

	switch(action) {
		case EquitWebServer::Configuration::Ignore:
			logEntry->setText(3, tr("Ignored"));
			break;

		case EquitWebServer::Configuration::Serve:
			logEntry->setText(3, tr("Served"));
			break;

		case EquitWebServer::Configuration::Forbid:
			logEntry->setText(3, tr("Forbidden, not found, or CGI failed"));
			break;

		case EquitWebServer::Configuration::CGI:
			logEntry->setText(3, tr("Executed through CGI"));
			break;

		default:
			logEntry->setText(3, tr("Unknown Action"));
			break;
	}
}


void EquitWebServer::ConfigurationWidget::logServerConnectionPolicy( const QString & addr, quint16 port, int policy ) {
	QTreeWidgetItem * logEntry = new QTreeWidgetItem(m_accessLogTabPage);
	logEntry->setText(0, addr);
	logEntry->setText(1, QString::number(port));

	switch(policy) {
		case EquitWebServer::Configuration::AcceptConnection:
			logEntry->setText(3, tr("Accepted"));
			logEntry->setIcon(3, QIcon(":/icons/connectionpolicies/accept"));
			break;

		case EquitWebServer::Configuration::RejectConnection:
			logEntry->setText(3, tr("Rejected"));
			logEntry->setIcon(3, QIcon(":/icons/connectionpolicies/reject"));
			break;

		case EquitWebServer::Configuration::NoConnectionPolicy:
			logEntry->setText(3, tr("No Connection Policy"));
			break;

		default:
			logEntry->setText(3, tr("Unknown Connection Policy"));
			break;
	}
}


void EquitWebServer::ConfigurationWidget::ipPolicySelectedItemChanged( QTreeWidgetItem * it ) {
	// update the mime action combos with selected entry
	if(it && it->treeWidget() == m_ipPolicyListWidget) {
		m_ipEdit->setText(it->text(0));
		m_ipConnectionPolicyCombo->setCurrentIndex(m_ipConnectionPolicyCombo->findText(it->text(1)));
	}
}


void EquitWebServer::ConfigurationWidget::extensionTreeSelectedItemChanged( QTreeWidgetItem * it ) {
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


void EquitWebServer::ConfigurationWidget::mimeActionSelectedItemChanged( QTreeWidgetItem * it ) {
	// update the mime action combos with selected entry
	if(it && it->treeWidget() == m_actionTree) {
		m_actionMimeTypeCombo->lineEdit()->setText(it->text(0));
		m_actionActionCombo->setCurrentIndex(m_actionActionCombo->findText(it->text(1)));
	}
}
