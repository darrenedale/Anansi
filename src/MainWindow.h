/** \file MainWindow.h
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the MainWindow class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_MAINWINDOW_H
#define EQUITWEBSERVER_MAINWINDOW_H

#include <QMainWindow>

class QPushButton;
class QStatusBar;
class QMenuBar;
class QMenu;
class QLabel;
class QPoint;


namespace EquitWebServer {
	class ConfigurationWidget;
	class ConnectionCountLabel;
	class Server;

	/**
	  * \note The object takes ownership of the server
	  */
	class MainWindow
	:	public QMainWindow {
		Q_OBJECT

		public:
			explicit MainWindow( Server * server = nullptr, QWidget * parent = nullptr );
			virtual ~MainWindow( void );

		public slots:
			void incrementRequestReceivedCount( void );
			void incrementRequestAcceptedCount( void );
			void incrementRequestRejectedCount( void );
			void setRequestReceivedCount( int );
			void setRequestAcceptedCount( int );
			void setRequestRejectedCount( int );
			void resetRequestReceivedCount( void );
			void resetRequestAcceptedCount( void );
			void resetRequestRejectedCount( void );
			void resetAllRequestCounts( void );
			void setStatusMessage( QString );
			bool startServer( void );
			bool stopServer( void );
			void about( void );
			void saveConfiguration( void );
			void saveConfigurationAsDefault( void );
			void loadConfiguration( void );
			void loadConfiguration( const QString & fileName );

		private slots:
			void serverStarted( void );
			void serverStopped( void );
			void loadRecentConfiguration( void );
			void slotDocumentRootChanged( void );

		private:
			void ensureUserConfigDir( void );
			void readRecentConfigs( void );
			void saveRecentConfigs( void );

			Server * m_server;
			QStatusBar * m_statusBar;
			QMenuBar * m_menuBar;
			QMenu * m_serverMenu;
			QMenu * m_accessMenu;
			QMenu * m_contentMenu;
			QMenu * m_recentConfigsMenu;
			ConfigurationWidget * m_controller;
			ConnectionCountLabel * m_requestReceivedCountLabel, * m_requestAcceptedCountLabel, * m_requestRejectedCountLabel;
			int m_requestReceivedCount, m_requestAcceptedCount, m_requestRejectedCount;
			QPushButton * m_startStopServer;
			QStringList m_recentConfigs;
	};	/* MainWindow class */
}  /* EquitWebServer namespace */

#endif
