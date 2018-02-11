/** \file MainWindow.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the MainWindow class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_MAINWINDOW_H
#define EQUITWEBSERVER_MAINWINDOW_H

#include <memory>

#include <QMainWindow>

class QPushButton;
class QStatusBar;
class QMenuBar;
class QMenu;
class QLabel;
class QPoint;

namespace EquitWebServer {
	class ConfigurationWidget;
	class CounterLabel;
	class Server;

	/**
	  * \note The object takes ownership of the server
	  */
	class MainWindow
	: public QMainWindow {
		Q_OBJECT

	public:
		explicit MainWindow(std::unique_ptr<Server> server = nullptr, QWidget * parent = nullptr);
		virtual ~MainWindow();

	public slots:
		void incrementRequestReceivedCount();
		void incrementRequestAcceptedCount();
		void incrementRequestRejectedCount();
		void setRequestReceivedCount(int);
		void setRequestAcceptedCount(int);
		void setRequestRejectedCount(int);
		void resetRequestReceivedCount();
		void resetRequestAcceptedCount();
		void resetRequestRejectedCount();
		void resetAllRequestCounts();
		void setStatusMessage(QString);
		bool startServer();
		bool stopServer();
		void about();
		void saveConfiguration();
		void saveConfigurationAsDefault();
		void loadConfiguration();
		void loadConfiguration(const QString & fileName);

	private slots:
		void serverStarted();
		void serverStopped();
		void loadRecentConfiguration();
		void slotDocumentRootChanged();

	private:
		void ensureUserConfigDir();
		void readRecentConfigs();
		void saveRecentConfigs();

		std::unique_ptr<Server> m_server;
		QStatusBar * m_statusBar;
		QMenuBar * m_menuBar;
		QMenu * m_serverMenu;
		QMenu * m_accessMenu;
		QMenu * m_contentMenu;
		QMenu * m_recentConfigsMenu;
		ConfigurationWidget * m_controller;
		CounterLabel *m_requestReceivedCountLabel, *m_requestAcceptedCountLabel, *m_requestRejectedCountLabel;
		int m_requestReceivedCount, m_requestAcceptedCount, m_requestRejectedCount;
		QPushButton * m_startStopServer;
		QStringList m_recentConfigs;
	}; /* MainWindow class */
}  // namespace EquitWebServer

#endif
