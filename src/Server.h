/** \file Server.h
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the Server class for EquitWebServer.
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_SERVER_H
#define EQUITWEBSERVER_SERVER_H


#include <QTcpServer>
#include "Configuration.h"
#include <QHostAddress>


namespace EquitWebServer {
	class Server
	:	 public QTcpServer {

		Q_OBJECT

		friend class bpWebServerRequestHandler;

		public:
			Server( const Configuration & opts );
			bool listen( void );
			void close( void );

			Configuration & configuration( void );
			bool setConfiguration( const Configuration & );

		signals:
			void connectionReceived( QString, quint16 );
			void connectionAccepted( QString, quint16 );
			void connectionRejected( QString, quint16 );
			void requestConnectionPolicyDetermined( QString, quint16, int );
			void requestActionTaken( QString, quint16, QString, int );

		protected:
			void incomingConnection( int socket );

			Configuration m_config;

		protected slots:
			void slotHandlerRequestActionTaken( QString, quint16, QString, int );
	};	/* class Server */
}	/* EquitWebServer namespace */

#endif
