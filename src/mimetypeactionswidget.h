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

		WebServerAction defaultAction() const;
		void setDefaultAction(WebServerAction action);

		void clear();

	Q_SIGNALS:
		void defaultActionChanged(WebServerAction action);
		void mimeTypeActionRemoved(const QString &, WebServerAction action, const QString & cgi = {});

	private Q_SLOTS:
		void onActionsSelectionChanged();

	private:
		std::unique_ptr<ServerMimeActionsModel> m_model;
		std::unique_ptr<Ui::MimeActionsWidget> m_ui;
		Server * m_server;  // observed only
		MimeTypeCombo * m_addMimeCombo;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_MIMEACTIONSWIDGET_H
