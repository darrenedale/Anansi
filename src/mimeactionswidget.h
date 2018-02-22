#ifndef EQUITWEBSERVER_MIMEACTIONSWIDGET_H
#define EQUITWEBSERVER_MIMEACTIONSWIDGET_H

#include <memory>

#include <QWidget>

namespace EquitWebServer {

	class Server;
	class ServerMimeActionsModel;

	namespace Ui {
		class MimeActionsWidget;
	}

	class MimeActionsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit MimeActionsWidget(QWidget * parent = nullptr);
		explicit MimeActionsWidget(Server * server, QWidget * parent = nullptr);
		~MimeActionsWidget();

		void setServer(Server * server);

		void clear();

	private:
		std::unique_ptr<ServerMimeActionsModel> m_model;
		std::unique_ptr<Ui::MimeActionsWidget> m_ui;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_MIMEACTIONSWIDGET_H
