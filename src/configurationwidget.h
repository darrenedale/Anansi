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

#include "types.h"
#include "server.h"
#include "configuration.h"

class QString;

namespace EquitWebServer {

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

		void bindToLocalhost();
		void bindToHostAddress();
		void setListenAddress(const QString &);

		void clearIpConnectionPolicies();
		void setLiberalDefaultConnectionPolicy();
		void setRestrictiveDefaultConnectionPolicy();
		void setDefaultConnectionPolicy(ConnectionPolicy);

		void setDefaultMimeType(const QString & mimeType);
		void setDefaultAction(WebServerAction action);

		void clearAllFileExtensionMIMETypes();
		void clearAllActions();

	Q_SIGNALS:
		void serverStatusMessage(QString);
		void requestReceived(QString, quint16);
		void documentRootChanged(QString);

	private:
		// observed only
		Server * m_server;
		std::unique_ptr<Ui::ConfigurationWidget> m_ui;
	};

}  // namespace EquitWebServer

#endif  //	EQUITWEBSERVER_CONFIGURATIONWIDGET_H
