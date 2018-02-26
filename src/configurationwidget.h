/// \file ConfigurationWidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date 19th June, 2012
///
/// \brief Definition of the ConfigurationWidget class for EquitWebServer
///
/// \todo class documentation.
///
/// \par Changes
/// - (2012-06-21) document root widget now has an indicator that gives
///   information on the validity of the path.
/// - (2012-06-19) file documentation created.

#ifndef EQUITWEBSERVER_CONFIGURATIONWIDGET_H
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_H

#include <memory>

#include <QWidget>

#include "server.h"
#include "configuration.h"

class QString;

namespace EquitWebServer {

	class IpListWidget;
	class ConnectionPolicyCombo;
	class WebServerActionCombo;
	class MimeTypeCombo;
	class ServerDetailsWidget;
	class AccessControlWidget;
	class FileAssociationsWidget;
	class MimeTypeActionsWidget;
	class AccessLogWidget;

	namespace Ui {
		class ConfigurationWidget;
	}

	/// \class ConfigurationWidget
	class ConfigurationWidget : public QWidget {
		Q_OBJECT

	public:
		explicit ConfigurationWidget(QWidget * parent = nullptr);
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

		void setIpConnectionPolicy(const QString &, ConnectionPolicy);
		void ipPolicyRemoved(const QString &);
		void clearIpConnectionPolicies();
		void setLiberalDefaultConnectionPolicy();
		void setRestrictiveDefaultConnectionPolicy();
		void setDefaultConnectionPolicy(ConnectionPolicy);

		void setDefaultMimeType(const QString & mime);

		void clearAllFileExtensionMIMETypes();
		//		void setMimeTypeAction(const QString & mime, WebServerAction action);
		//		void setDefaultAction(WebServerAction action);
		void clearAllActions();
		void setAllowDirectoryListing(bool);

	Q_SIGNALS:
		void serverStatusMessage(QString);
		void requestReceived(QString, quint16);
		void documentRootChanged(QString);

	private:
		Server * m_server;
		//		ServerDetailsWidget * m_serverConfig;
		//		AccessControlWidget * m_accessConfig;
		//		QCheckBox * m_allowDirectoryListing;
		//		FileAssociationsWidget * m_fileAssociations;
		//		MimeTypeActionsWidget * m_mimeActions;
		//		AccessLogWidget * m_accessLog;

		//		QTabWidget * m_serverConfigTabs;


		std::unique_ptr<Ui::ConfigurationWidget> m_ui;
	};

}  // namespace EquitWebServer

#endif  //	EQUITWEBSERVER_CONFIGURATIONWIDGET_H
