/// \file webserveractioncombo.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the WebServerActionCombo class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_WEBSERVERACTIONCOMBO_H
#define EQUITWEBSERVER_WEBSERVERACTIONCOMBO_H

#include <QComboBox>

#include "configuration.h"

namespace EquitWebServer {

	class WebServerActionCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit WebServerActionCombo(QWidget * parent = nullptr);
		virtual ~WebServerActionCombo() = default;

		void addItem() = delete;

		WebServerAction webServerAction();

	public Q_SLOTS:
		void setWebServerAction(WebServerAction action);

	Q_SIGNALS:
		void webServerActionChanged(WebServerAction);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_WEBSERVERACTIONCOMBO_H
