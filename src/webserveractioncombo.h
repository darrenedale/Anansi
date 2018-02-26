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
