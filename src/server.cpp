/** \file Server.cpp
 * \author Darren Edale
 * \version 0.9.9
 * \date 19th June, 2012
 *
 * \brief Implementation of the Server class for EquitWebServer
 *
 * \todo
 * - decide on application license
 *
 * \par Changes
 * - (2012-06-19) file documentation created.
 */

#include "server.h"

#include <iostream>

#include <QDebug>

#include "requesthandler.h"


namespace EquitWebServer {


	Server::Server(const Configuration & opts) {
		setConfiguration(opts);
	}


	Server::~Server() = default;


	bool Server::listen() {
		if(QTcpServer::listen(QHostAddress(m_config.listenAddress()), static_cast<quint16>(m_config.port()))) {
			return true;
		}

		qDebug() << "failed to bind to port" << m_config.port() << "on ip address" << m_config.listenAddress() << ":";
		qDebug() << errorString();
		return false;
	}


	void Server::close() {
		QTcpServer::close();

		if(isListening()) {
			qDebug() << "could not stop listening on" << m_config.listenAddress() << ":" << m_config.port();
		}
	}


	void Server::incomingConnection(qintptr socketFd) {
		auto socket = std::make_unique<QTcpSocket>();

		if(!socket->setSocketDescriptor(socketFd)) {
			std::cout << __PRETTY_FUNCTION__ << ": failed to set socket descriptor\n";
			return;
		}

		/* handler takes ownership of the socket, moving it to its own thread. the handler is
		 * scheduled for deletion as soon as it completes. when it is deleted, it deletes the
		 * socket with it */
		std::cout << __PRETTY_FUNCTION__ << ": creating handler\n";
		Q_EMIT connectionReceived(socket->peerAddress().toString(), socket->peerPort());
		RequestHandler * h = new RequestHandler(std::move(socket), m_config, this);
		connect(h, &RequestHandler::finished, h, &RequestHandler::deleteLater);

		// pass signals from handler through signals from server
		// connect(h, &RequestHandler::handlingRequestFrom, this,
		// &Server::connectionReceived);
		connect(h, &RequestHandler::acceptedRequestFrom, this, &Server::connectionAccepted);
		connect(h, &RequestHandler::rejectedRequestFrom, this, &Server::connectionRejected);
		connect(h, &RequestHandler::requestConnectionPolicyDetermined, this, &Server::requestConnectionPolicyDetermined);
		connect(h, &RequestHandler::requestActionTaken, this, &Server::requestActionTaken);
		connect(h, &RequestHandler::requestActionTaken, this, &Server::onHandlerRequestActionTaken);
		std::cout << __PRETTY_FUNCTION__ << ": starting handler\n";
		h->start();
	}


	void Server::onHandlerRequestActionTaken(QString host, quint16 port, QString path, int action) {
		Q_UNUSED(host);
		Q_UNUSED(port);
		Q_UNUSED(path);
		Q_UNUSED(action);
	}


	Configuration & Server::configuration() {
		return m_config;
	}


	bool Server::setConfiguration(const Configuration & opts) {
		Configuration realOpts = opts;

		if(isListening() &&
			(opts.listenAddress() != m_config.listenAddress() ||
			 opts.port() != m_config.port())) {
			qDebug() << "server listening - listen address and port changes will not take effect until server restart.";
		}

		m_config = realOpts;
		return true;
	}


}  // namespace EquitWebServer
