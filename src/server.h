/** \file Server.h
 * \author Darren Edale
 * \version 0.9.9
 * \date 19th June, 2012
 *
 * \brief Definition of the Server class for EquitWebServer.
 *
 * \todo
 * - class documentation.
 * - decide on application license.
 *
 * \par Changes
 * - (2012-06-19) file documentation created.
 *
 */

#ifndef EQUITWEBSERVER_SERVER_H
#define EQUITWEBSERVER_SERVER_H

#include "configuration.h"
#include <QString>
#include <QHostAddress>
#include <QTcpServer>

namespace EquitWebServer {

	class Server : public QTcpServer {
		Q_OBJECT

		friend class bpWebServerRequestHandler;

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

		static QByteArray mimeIconUri(const QString & mimeType);

	Q_SIGNALS:
		void connectionReceived(const QString &, uint16_t);
		void connectionAccepted(const QString &, uint16_t);
		void connectionRejected(const QString & ip, uint16_t port, const QString & msg);
		void requestConnectionPolicyDetermined(const QString &, uint16_t, Configuration::ConnectionPolicy);
		void requestActionTaken(const QString &, uint16_t, const QString &, Configuration::WebServerAction);

	protected:
		void incomingConnection(qintptr socket) override;

		Configuration m_config;
	}; /* class Server */

}  // namespace EquitWebServer

#endif