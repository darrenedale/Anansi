/// \file MainWindow.h
/// \author Darren Edale
/// \version 0.9.9
/// \date 19th June, 2012
///
/// \brief Definition of the MainWindow class for EquitWebServer
///
/// \par Changes
/// - (2012-06-19) file documentation created.

#ifndef EQUITWEBSERVER_MAINWINDOW_H
#define EQUITWEBSERVER_MAINWINDOW_H

#include <memory>
#include <vector>

#include <QMainWindow>
#include <QString>

#include "window.h"

class QMenu;

namespace EquitWebServer {

	class ConfigurationWidget;
	class Server;
	class MainWindowStatusBar;

	namespace Ui {
		class MainWindow;
	}

	class MainWindow : public Window {
		Q_OBJECT

	public:
		explicit MainWindow(QWidget * parent = nullptr);
		explicit MainWindow(std::unique_ptr<Server> server = nullptr, QWidget * parent = nullptr);
		virtual ~MainWindow();

		void setServer(std::unique_ptr<Server> server);

	public Q_SLOTS:
		void incrementRequestReceivedCount();
		void incrementRequestAcceptedCount();
		void incrementRequestRejectedCount();
		void resetRequestReceivedCount();
		void resetRequestAcceptedCount();
		void resetRequestRejectedCount();
		void resetAllRequestCounts();
		void setStatusMessage(const QString &);
		bool startServer();
		bool stopServer();
		void about();
		void saveConfiguration();
		void saveConfigurationAsDefault();
		void loadConfiguration();
		void loadConfiguration(const QString & fileName);

		MainWindowStatusBar * statusBar() const;

	private Q_SLOTS:
		void loadRecentConfiguration();

	private:
		void readRecentConfigs();
		void saveRecentConfigs();

		std::unique_ptr<Server> m_server;
		std::unique_ptr<Ui::MainWindow> m_ui;
		std::vector<QString> m_recentConfigs;
	};

}  // namespace EquitWebServer

#endif
