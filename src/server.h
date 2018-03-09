/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Anansi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Anansi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file server.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Definition of the Server class for Anansi..
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SERVER_H
#define ANANSI_SERVER_H

#include "configuration.h"
#include <QString>
#include <QHostAddress>
#include <QTcpServer>


namespace Anansi {

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

}  // namespace Anansi

#endif
