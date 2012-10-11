#include "bpMimeTypeManagerWidget.h"
#include "bpEditableTreeWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDebug>
#include <QItemDelegate>


bpMimeTypeManagerWidget::bpMimeTypeManagerWidget( const bpWebServerConfiguration * config, QWidget * parent )
:	QWidget(parent),
	m_mimeTree(0),
	m_defaultMIMECombo(0),
	m_defaultActionCombo(0)
{
	(void) config;
	QHBoxLayout * defaultsLayout = new QHBoxLayout();

	m_defaultMIMECombo = new QComboBox();
	m_defaultMIMECombo->setEditable(true);
	m_defaultMIMECombo->setToolTip(tr("The default MIME Type to use for all extensions without a registered MIME type."));
	QLabel * tempLabel = new QLabel(tr("Default MIME Type"));
	tempLabel->setToolTip(tr("The default MIME Type to use for all extensions without a registered MIME type."));
	tempLabel->setBuddy(m_defaultMIMECombo);

	defaultsLayout->addWidget(tempLabel);
	defaultsLayout->addWidget(m_defaultMIMECombo);
	defaultsLayout->setStretchFactor(m_defaultMIMECombo, 1);

	m_defaultActionCombo = new QComboBox();
	m_defaultActionCombo->addItem(tr("Ignore"), bpWebServerConfiguration::Ignore);
	m_defaultActionCombo->addItem(tr("Serve"), bpWebServerConfiguration::Serve);
	m_defaultActionCombo->addItem(tr("CGI"), bpWebServerConfiguration::CGI);
	m_defaultActionCombo->addItem(tr("Forbid"), bpWebServerConfiguration::Forbid);
	m_defaultActionCombo->setToolTip(tr("The default action to use for all MIME types without specific registered actions."));
	tempLabel = new QLabel(tr("Default Action"));
	tempLabel->setToolTip(m_defaultActionCombo->toolTip());
	tempLabel->setBuddy(m_defaultActionCombo);

	defaultsLayout->addWidget(tempLabel);
	defaultsLayout->addWidget(m_defaultActionCombo);
	defaultsLayout->setStretchFactor(m_defaultActionCombo, 1);

	m_mimeTree = new bpEditableTreeWidget();
	m_mimeTree->setColumnCount(3);
	QTreeWidgetItem * mimeTreeHeader = new QTreeWidgetItem();
	mimeTreeHeader->setText(0, tr("MIME Type"));
	mimeTreeHeader->setText(1, tr("Action"));
	mimeTreeHeader->setText(2, tr("CGI Executable"));
	m_mimeTree->setHeaderItem(mimeTreeHeader);
	m_mimeTree->setRootIsDecorated(false);
	m_mimeTree->setItemDelegateForColumn(2, new QItemDelegate(this));

	QVBoxLayout * mainLayout = new QVBoxLayout();
	mainLayout->addLayout(defaultsLayout);
	mainLayout->addWidget(m_mimeTree);
	setLayout(mainLayout);
}


virtual ~bpMimeTypeManagerWidget( void );

/* these are all slots to get connected to button clicked() signals, content (e.g. mime type) to come from
	* current content of line exit/combo/tree widgets */
void _addNewMimeType( void );
bool _removeMimeType( void );
bool _addExtensionToMimeType( void );
bool _removeExtensionFromMimeType( void );
bool _setMimeTypeCgiExecutable( void );
bool _setMimeTypeAction( void );
void _emitDefaultMimeTypeChanged( void );
void _emitDefaultActionChanged( void );


signals:
extensionAddedToMimeType( QString & mimeType, QString & extension );
extensionRemovedFromMimeType( QString & mimeType, QString & extension );
mimeTypeAdded( QString & mimeType );
mimeTypeRemoved( QString & mimeType );

mimeTypeActionChanged( QString & mimeType, bpWebServerConfiguration::WebServerAction action, QString cgiExe = QString() );
mimeTypeCgiExecutableChanged( QString & mimeType, QString & cgiExe );

defaultMimeTypeChanged( QString & mimeType );
defaultActionChanged( bpWebServerConfiguration::WebServerAction action );
