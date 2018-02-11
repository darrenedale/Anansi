/** \file ConfigurationWidget.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the ConfigurationWidget class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Changes
  * - (2012-06-21) document root widget now has an indicator that gives
  *   information on the validity of the path.
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_CONFIGURATIONWIDGET_H
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_H

#include <QWidget>

#include "server.h"


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


namespace EquitWebServer {

	class EditableTreeWidget;
	class IpListWidget;
	class ConnectionPolicyCombo;
	class ServerConfigWidget;
	class AccessControlWidget;
	class AccessLogWidget;

	/// \class ConfigurationWidget
	class ConfigurationWidget : public QWidget {
		Q_OBJECT

	public:
		explicit ConfigurationWidget(Server * server, QWidget * parent = nullptr);
		virtual ~ConfigurationWidget();

		void enableWidgets();
		void disableWidgets();
		void setServer(Server * server);
		void readConfiguration();

	public Q_SLOTS:
		void chooseDocumentRoot();
		void setDocumentRoot(const QString &);
		void setListenAddress(const QString &);
		void setListenPort(int);
		void bindToLocalhost();
		void bindToHostAddress();

		//		void setIPConnectionPolicy();
		void setIPConnectionPolicy(const QString & ip, Configuration::ConnectionPolicy p);
		void ipPolicyRemoved(const QString & ip);
		void clearIPConnectionPolicies();
		void setLiberalDefaultConnectionPolicy();
		void setRestrictedDefaultConnectionPolicy();
		void setDefaultConnectionPolicy(Configuration::ConnectionPolicy p);

		void addFileExtensionMIMEType();
		void setDefaultMIMEType();
		void setDefaultMIMEType(const QString & mime);
		void clearAllFileExtensionMIMETypes();
		void setMIMETypeAction();
		void setMIMETypeAction(const QString & mime, Configuration::WebServerAction action);
		void setDefaultAction();
		void setDefaultAction(Configuration::WebServerAction action);
		void removeAction(QTreeWidgetItem *);
		void actionDoubleClicked(QTreeWidgetItem *);
		void clearAllActions();
		void removeExtensionMIMEType(QTreeWidgetItem *);
		//		void logServerAction(const QString & addr, quint16 port, const QString & resource, int action);
		//		void logServerConnectionPolicy(const QString & addr, quint16 port, Configuration::ConnectionPolicy policy);
		void mimeActionSelectedItemChanged(QTreeWidgetItem *);
		void extensionTreeSelectedItemChanged(QTreeWidgetItem * it);
		void setAllowDirectoryListing(bool allow);

	Q_SIGNALS:
		void serverStatusMessage(QString);
		void requestReceived(QString, quint16);
		void documentRootChanged(QString);

	private:
		void connectEvents();
		void disconnectEvents();
		//		void repopulateAddressItems();
		//		void updateDocumentRootStatusIndicator();
		//		void updateListenAddressStatusIndicator();

		static QString s_mimeIconResourcePath;
		bool m_eventsConnected;
		Server * m_server;

		ServerConfigWidget * m_serverConfig;
		AccessControlWidget * m_accessConfig;

		QCheckBox * m_allowDirectoryListing;

		EditableTreeWidget * m_extensionMIMETypeTree;
		QComboBox * m_fileExtensionCombo;
		QComboBox * m_extensionMimeTypeCombo;
		QToolButton * m_extensionMimeTypeAddButton;

		EditableTreeWidget * m_actionTree;
		QComboBox * m_actionMimeTypeCombo;
		QComboBox * m_actionActionCombo;
		QToolButton * m_mimeTypeActionSetButton;

		QComboBox * m_defaultMIMECombo;
		QComboBox * m_defaultActionCombo;

		AccessLogWidget * m_accessLog;

		QTabWidget * m_serverControlsTab;
	}; /* ControllerWidget class */
}  // namespace EquitWebServer

#endif  //	EQUITWEBSERVER_CONFIGURATIONWIDGET_H
