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
/// - <iostream>
/// - <QStringBuilder>
/// - <QFile>
/// - <QIcon>
/// - <QBuffer>
/// - requesthandler.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "server.h"

#include <iostream>

#include <QStringBuilder>
#include <QFile>
#include <QIcon>
#include <QBuffer>

#include "requesthandler.h"
#include "qtmetatypes.h"


namespace Anansi {


	/// NEXTRELEASE ipv6 support?
	/// NEXTRELEASE SSL support?


	Server::Server(const Configuration & opts) {
		setConfiguration(opts);
	}


	Server::~Server() = default;


	bool Server::listen() {
		if(!QTcpServer::listen(QHostAddress(m_config.listenAddress()), static_cast<quint16>(m_config.port()))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to bind to port " << m_config.port() << " on address " << qPrintable(m_config.listenAddress()) << " (" << qPrintable(errorString()) << ")\n";
			return false;
		}

		Q_EMIT startedListening();
		Q_EMIT listeningStateChanged(true);
		return true;
	}


	void Server::close() {
		QTcpServer::close();

		if(isListening()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: could not stop listening on" << qPrintable(m_config.listenAddress()) << ":" << m_config.port();
			return;
		}

		Q_EMIT stoppedListening();
		Q_EMIT listeningStateChanged(false);
	}


	void Server::incomingConnection(qintptr socketFd) {
		auto socket = std::make_unique<QTcpSocket>();

		if(!socket->setSocketDescriptor(socketFd)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to set socket descriptor (" << qPrintable(socket->errorString()) << ")\n";
			return;
		}

		/* handler takes ownership of the socket, moving it to its own thread. the handler is
		 * scheduled for deletion as soon as it completes. when it is deleted, it deletes the
		 * socket with it */
		Q_EMIT connectionReceived(socket->peerAddress().toString(), socket->peerPort());

		// there is a small problem here in that the configuration loaned to the handler will
		// disappear if the server is destroyed before the handler, resulting in UB if/when
		// the handler tries to read the config. at the moment, the only way this happens if
		// if the application quits while a handler is still processing a request, which is
		// not a huge problem, but it we should mitigate against it
		RequestHandler * handler = new RequestHandler(std::move(socket), m_config, this);
		connect(handler, &RequestHandler::finished, handler, &RequestHandler::deleteLater);

		// pass signals from handler through signals from server
		connect(handler, &RequestHandler::acceptedRequestFrom, this, &Server::connectionAccepted);
		connect(handler, &RequestHandler::rejectedRequestFrom, this, &Server::connectionRejected);
		connect(handler, &RequestHandler::requestConnectionPolicyDetermined, this, &Server::requestConnectionPolicyDetermined, Qt::QueuedConnection);
		connect(handler, &RequestHandler::requestActionTaken, this, &Server::requestActionTaken, Qt::QueuedConnection);

		handler->start();
	}


	Configuration & Server::configuration() {
		return m_config;
	}


	bool Server::setConfiguration(const Configuration & opts) {
		Configuration realOpts = opts;

		if(isListening() && (opts.listenAddress() != m_config.listenAddress() || opts.port() != m_config.port())) {
			std::clog << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: server already started - listen address and port changes will not take effect until server restart.\n";
		}

		m_config = realOpts;
		return true;
	}


}  // namespace Anansi
