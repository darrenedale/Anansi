/// \file mainwindow.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Definition of the MainWindow class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MAINWINDOW_H
#define ANANSI_MAINWINDOW_H

#include <memory>
#include <vector>

#include <QString>
#include <QActionGroup>

#include "window.h"

class QMenu;
class QAction;
class QActionGroup;

namespace Anansi {

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

	private:
		QAction * addRecentConfiguration(const QString & path);
		void readRecentConfigurations();
		void saveRecentConfigurations();

		std::unique_ptr<Server> m_server;
		std::unique_ptr<Ui::MainWindow> m_ui;
		std::vector<std::unique_ptr<QAction>> m_recentConfigActions;
		std::unique_ptr<QActionGroup> m_recentConfigActionGroup;
	};

}  // namespace Anansi

#endif
