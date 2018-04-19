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

/// \file server.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date 19th June, 2012
///
/// \brief Implementation of the Server class for Anansi.
///
/// \dep
/// - server.h
/// - <iostream>
/// - <QHostAddress>
/// - <QString>
/// - assert.h
/// - requesthandler.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "server.h"

#include <iostream>

#include <QHostAddress>
#include <QString>

#include "eqassert.h"
#include "requesthandler.h"
#include "qtmetatypes.h"


namespace Anansi {


	/// NEXTRELEASE ipv6 support?
	/// NEXTRELEASE SSL support?


	Server::Server(const Configuration & config) {
		setConfiguration(config);
	}


	Server::Server(Configuration && config) {
		setConfiguration(std::move(config));
	}


	bool Server::listen() {
		eqAssert(!isListening(), "can't call listen() on a Server that is already listening");

		if(!QTcpServer::listen(QHostAddress(m_config.listenAddress()), static_cast<quint16>(m_config.port()))) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to listen on " << qPrintable(m_config.listenAddress()) << ":" << m_config.port() << " (" << qPrintable(errorString()) << ")\n";
			return false;
		}

		Q_EMIT startedListening();
		Q_EMIT listeningStateChanged(true);
		return true;
	}


	void Server::close() {
		QTcpServer::close();

		if(isListening()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to stop listening on" << qPrintable(m_config.listenAddress()) << ":" << m_config.port() << " (" << qPrintable(errorString()) << ")\n";
			return;
		}

		Q_EMIT stoppedListening();
		Q_EMIT listeningStateChanged(false);
	}


	void Server::incomingConnection(qintptr socketFd) {
		// we're not using the Pending Connections mechanism of QTcpServer so we
		// don't call addPendingConnection()
		auto socket = std::make_unique<QTcpSocket>();

		if(!socket->setSocketDescriptor(socketFd)) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to set socket descriptor (" << qPrintable(socket->errorString()) << ")\n";
			return;
		}

		// handler takes ownership of the socket, moving it to its own thread. the handler is
		// scheduled for deletion as soon as it completes. when it is deleted, it deletes the
		// socket with it
		Q_EMIT connectionReceived(socket->peerAddress().toString(), socket->peerPort());

		// still need to parent the handler so that if the Server is destroyed the handler
		// it spawned is also destroyed. this ensures that the handler doesn't outlive the
		// server and therefore does not attempt to read the Configuration that is loaned
		// to it by the server after the server has destroyed it
		RequestHandler * handler = new RequestHandler(std::move(socket), m_config, this);
		connect(handler, &RequestHandler::finished, handler, &RequestHandler::deleteLater);

		// pass signals from handler through signals from server
		connect(handler, &RequestHandler::acceptedRequestFrom, this, &Server::connectionAccepted);
		connect(handler, &RequestHandler::rejectedRequestFrom, this, &Server::connectionRejected);
		connect(handler, &RequestHandler::requestConnectionPolicyDetermined, this, &Server::requestConnectionPolicyDetermined, Qt::QueuedConnection);
		connect(handler, &RequestHandler::requestActionTaken, this, &Server::requestActionTaken, Qt::QueuedConnection);

		handler->start();
	}


	bool Server::setConfiguration(const Configuration & config) {
		if(isListening() && (config.listenAddress() != m_config.listenAddress() || config.port() != m_config.port())) {
			std::cout << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: server already started - listen address and port changes will not take effect until server restart.\n"
						 << std::flush;
		}

		m_config = config;
		return true;
	}


	bool Server::setConfiguration(Configuration && config) {
		if(isListening() && (config.listenAddress() != m_config.listenAddress() || config.port() != m_config.port())) {
			std::cout << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: server already started - listen address and port changes will not take effect until server restart.\n"
						 << std::flush;
		}

		m_config = std::move(config);
		return true;
	}


}  // namespace Anansi
