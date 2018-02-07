/** \file ConfigurationWidget.h
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the ConfigurationWidget class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Current Changes
  * - (2012-06-21) document root widget now has an indicator that gives
  *   information on the validity of the path.
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_CONFIGURATIONWIDGET_H
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_H

#include <QWidget>

#include "Server.h"

class bpIpListWidget;

class QLabel;
class QToolButton;
class QSpinBox;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QListWidget;
class QTabWidget;
class QGridLayout;
class QTreeWidget;
class QTreeWidgetItem;
class bpEditableTreeWidget;


namespace EquitWebServer {

	/**
	** @class bpWebServerController
	*/
	class ConfigurationWidget
	:	public QWidget
	{
		Q_OBJECT

		public:
			ConfigurationWidget( Server * server, QWidget *parent = 0 );
			virtual ~ConfigurationWidget( void );

			void enableWidgets( void );
			void disableWidgets( void );
			void setServer( Server * server );
			void readConfiguration( void );

		public slots:
			void selectDocumentRoot( void );
			void setDocumentRoot( const QString & );
			void setListenAddress( const QString & );
			void setListenPort( int );
			void bindToLocalhost( void );
			void bindToHostAddress( void );
			void setIPConnectionPolicy( void );
			void setIPConnectionPolicy( const QString & ip, Configuration::ConnectionPolicy p );
			void ipPolicyRemoved( const QString & ip );
			void clearIPConnectionPolicies( void );
			void setDefaultConnectionPolicy( void );
			void setLiberalDefaultConnectionPolicy( void );
			void setRestrictedDefaultConnectionPolicy( void );
			void setDefaultConnectionPolicy( Configuration::ConnectionPolicy p );
			void addFileExtensionMIMEType( void );
			void setDefaultMIMEType( void );
			void setDefaultMIMEType( const QString & mime );
			void clearAllFileExtensionMIMETypes( void );
			void setMIMETypeAction( void );
			void setMIMETypeAction( const QString & mime, Configuration::WebServerAction action );
			void setDefaultAction( void );
			void setDefaultAction( Configuration::WebServerAction action );
			void removeAction( QTreeWidgetItem * );
			void actionDoubleClicked( QTreeWidgetItem * );
			void clearAllActions( void );
			void removeExtensionMIMEType( QTreeWidgetItem * );
			void logServerAction( const QString & addr, quint16 port, const QString & resource, int action );
			void logServerConnectionPolicy( const QString & addr, quint16 port, int policy );
			void ipPolicySelectedItemChanged( QTreeWidgetItem * it );
			void mimeActionSelectedItemChanged( QTreeWidgetItem * );
			void extensionTreeSelectedItemChanged( QTreeWidgetItem * it );
			void setAllowDirectoryListing( bool allow );

		signals:
			void serverStatusMessage( QString );
			void requestReceived( QString, quint16 );
			void documentRootChanged( QString );

		private:
			void connectEvents( void );
			void disconnectEvents( void );
			void repopulateAddressItems( void );
			void updateDocumentRootStatusIndicator( void );
			void updateListenAddressStatusIndicator( void );

			static QString s_mimeIconResourcePath;
			bool m_eventsConnected;
			Server * m_server;

			QComboBox * m_serverAddressEdit;
			QLabel * m_serverAddressStatus;
			QSpinBox * m_serverPortWidget;
			QLineEdit * m_documentRootEdit;
			QLabel * m_documentRootStatus;
			QToolButton * m_documentRootSelect;

			QLineEdit * m_ipEdit;
			bpIpListWidget * m_ipPolicyListWidget;
			QComboBox * m_ipConnectionPolicyCombo;
			QToolButton * m_setIpConnectionPolicyButton;
			QComboBox * m_defaultConnectionPolicyCombo;

			QCheckBox * m_allowDirectoryListing;

			bpEditableTreeWidget * m_extensionMIMETypeTree;
			QComboBox * m_fileExtensionCombo;
			QComboBox * m_extensionMimeTypeCombo;
			QToolButton * m_extensionMimeTypeAddButton;

			bpEditableTreeWidget * m_actionTree;
			QComboBox * m_actionMimeTypeCombo;
			QComboBox * m_actionActionCombo;
			QToolButton * m_mimeTypeActionSetButton;

			QComboBox * m_defaultMIMECombo;
			QComboBox * m_defaultActionCombo;

			QTreeWidget * m_accessLogTabPage;

			QTabWidget * m_serverControlsTab;
	};	/* ControllerWidget class */
}	/* EquitWebServer namespace */

 #endif		//	EQUITWEBSERVER_CONFIGURATIONWIDGET_H
