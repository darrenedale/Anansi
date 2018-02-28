/// \file configurationwidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Definition of the ConfigurationWidget class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_CONFIGURATIONWIDGET_H
#define EQUITWEBSERVER_CONFIGURATIONWIDGET_H

#include <memory>

#include <QWidget>

#include "types.h"
#include "server.h"
#include "configuration.h"

class QString;

namespace EquitWebServer {

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

		void setDefaultMimeType(const QString &);
		void setDefaultAction(WebServerAction);

		void clearAllFileExtensionMIMETypes();
		void clearAllActions();

	private:
		// observed only
		Server * m_server;
		std::unique_ptr<Ui::ConfigurationWidget> m_ui;
	};

}  // namespace EquitWebServer

#endif  //	EQUITWEBSERVER_CONFIGURATIONWIDGET_H
