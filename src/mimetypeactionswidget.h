/// \file mimetypeactionswidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the MimeTypeActionsWidget class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_MIMETYPEACTIONSWIDGET_H
#define EQUITWEBSERVER_MIMETYPEACTIONSWIDGET_H

#include <memory>

#include <QWidget>

#include "configuration.h"

namespace EquitWebServer {

	class Server;
	class ServerMimeActionsModel;
	class MimeCombo;

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
		MimeCombo * m_addMimeCombo;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMETYPEACTIONSWIDGET_H
