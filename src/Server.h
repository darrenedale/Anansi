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

#include "Configuration.h"
#include <QHostAddress>
#include <QTcpServer>

namespace EquitWebServer {
	class Server : public QTcpServer {
		Q_OBJECT

		friend class bpWebServerRequestHandler;

	public:
		explicit Server(const Configuration & opts);
		bool listen();
		void close();

		Configuration & configuration();
		bool setConfiguration(const Configuration &);

	Q_SIGNALS:
		void connectionReceived(QString, quint16);
		void connectionAccepted(QString, quint16);
		void connectionRejected(const QString & ip, quint16 port, const QString & msg);
		void requestConnectionPolicyDetermined(QString, quint16, int);
		void requestActionTaken(QString, quint16, QString, int);

	protected:
		void incomingConnection(qintptr socket) override;

		Configuration m_config;

	protected slots:
		void slotHandlerRequestActionTaken(QString, quint16, QString, int);
	}; /* class Server */
}  // namespace EquitWebServer

#endif
