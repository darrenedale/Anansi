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
/// \version 1.0.0
/// \date March 2018
///
/// \brief Definition of the Server class for Anansi.
///
/// \dep
/// - <cstdint>
/// - <QTcpServer>
/// - types.h
/// - configuration.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SERVER_H
#define ANANSI_SERVER_H

#include <cstdint>

#include <QTcpServer>

#include "types.h"
#include "configuration.h"

class QString;

namespace Anansi {

	class Server : public QTcpServer {
		Q_OBJECT

	public:
		explicit Server(const Configuration & config);
		explicit Server(Configuration && config);
		Server(const Server &) = delete;
		Server(Server &&) = delete;

		Server & operator=(const Server &) = delete;
		Server & operator=(Server &&) = delete;

		bool listen();
		void close();

		inline Configuration & configuration() noexcept {
			return m_config;
		}

		inline const Configuration & configuration() const noexcept {
			return m_config;
		}

		bool setConfiguration(const Configuration & config);
		bool setConfiguration(Configuration && config);

	Q_SIGNALS:
		void startedListening() const;
		void stoppedListening() const;
		void listeningStateChanged(bool listening) const;
		void connectionReceived(const QString & addr, uint16_t port) const;
		void connectionAccepted(const QString & addr, uint16_t port) const;
		void connectionRejected(const QString & addr, uint16_t port, const QString & msg) const;
		void requestConnectionPolicyDetermined(const QString & addr, uint16_t port, ConnectionPolicy policy) const;
		void requestActionTaken(const QString & addr, uint16_t port, const QString & resource, WebServerAction action) const;

	protected:
		virtual void incomingConnection(qintptr socket) override;

	private:
		Configuration m_config;
	};

}  // namespace Anansi

#endif
