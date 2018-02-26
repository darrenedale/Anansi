#ifndef EQUITWEBSERVER_MIMEACTIONSWIDGET_H
#define EQUITWEBSERVER_MIMEACTIONSWIDGET_H

#include <memory>

#include <QWidget>

#include "configuration.h"

namespace EquitWebServer {

	class Server;
	class ServerMimeActionsModel;
	class MimeTypeCombo;

	namespace Ui {
		class MimeActionsWidget;
	}

	class MimeTypeActionsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit MimeTypeActionsWidget(QWidget * parent = nullptr);
		explicit MimeTypeActionsWidget(Server * server, QWidget * parent = nullptr);
		~MimeTypeActionsWidget();

		void setServer(Server * server);

		void clear();

	Q_SIGNALS:
		void defaultMimeTypeActionChanged(WebServerAction action);
		void mimeTypeActionRemoved(const QString &, WebServerAction action, const QString & cgi = {});

	private:
		std::unique_ptr<ServerMimeActionsModel> m_model;
		std::unique_ptr<Ui::MimeActionsWidget> m_ui;
		MimeTypeCombo * m_addMimeCombo;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_MIMEACTIONSWIDGET_H
