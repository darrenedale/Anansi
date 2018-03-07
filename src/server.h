/// \file server.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Definition of the Server class for EquitWebServer.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_SERVER_H
#define EQUITWEBSERVER_SERVER_H

#include "configuration.h"
#include <QString>
#include <QHostAddress>
#include <QTcpServer>


namespace EquitWebServer {

	class Server : public QTcpServer {
		Q_OBJECT

	public:
		explicit Server(const Configuration & opts);
		Server(const Server &) = delete;
		Server(Server &&) = delete;
		virtual ~Server() override;

		Server & operator=(const Server &) = delete;
		Server & operator=(Server &&) = delete;

		bool listen();
		void close();

		Configuration & configuration();
		bool setConfiguration(const Configuration &);

	Q_SIGNALS:
		void startedListening();
		void stoppedListening();
		void listeningStateChanged(bool listening);
		void connectionReceived(const QString &, quint16);
		void connectionAccepted(const QString &, quint16);
		void connectionRejected(const QString & ip, quint16 port, const QString & msg);
		void requestConnectionPolicyDetermined(const QString &, quint16, ConnectionPolicy);
		void requestActionTaken(const QString &, quint16, const QString &, WebServerAction);

	protected:
		virtual void incomingConnection(qintptr socket) override;

		Configuration m_config;
	};

}  // namespace EquitWebServer

#endif
