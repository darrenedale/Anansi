/** \file Server.cpp
 * \author Darren Edale
 * \version 0.9.9
 * \date 19th June, 2012
 *
 * \brief Implementation of the Server class for EquitWebServer
 *
 * \par Changes
 * - (2012-06-19) file documentation created.
 */

#include "server.h"

#include <iostream>

#include <QStringBuilder>
#include <QFile>
#include <QIcon>
#include <QBuffer>

#include "requesthandler.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);
Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);


namespace EquitWebServer {


	Server::Server(const Configuration & opts) {
		setConfiguration(opts);
	}


	Server::~Server() = default;


	bool Server::listen() {
		if(!QTcpServer::listen(QHostAddress(m_config.listenAddress()), static_cast<quint16>(m_config.port()))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to bind to port " << m_config.port() << " on address " << qPrintable(m_config.listenAddress()) << " (" << qPrintable(errorString()) << ")\n";
			return false;
		}

		return true;
	}


	void Server::close() {
		QTcpServer::close();

		if(isListening()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: could not stop listening on" << qPrintable(m_config.listenAddress()) << ":" << m_config.port();
		}
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
		RequestHandler * h = new RequestHandler(std::move(socket), m_config, this);
		connect(h, &RequestHandler::finished, h, &RequestHandler::deleteLater);

		// pass signals from handler through signals from server
		// TODO why do these work as lambdas but not as a directly-connected slots?
		// The slot is being connected successfully because the QMetaObject::Connection returned is valid.
		// But the slot is never called; contrarily, in the lambda, the lambda is invoked and the AccessLogWidget
		// slot is called successfully
		//		connect(h, &RequestHandler::handlingRequestFrom, [this](const QString & addr, quint16 port) {
		//			Q_EMIT connectionReceived(addr, port);
		//		});

		connect(h, &RequestHandler::acceptedRequestFrom, [this](const QString & addr, quint16 port) {
			Q_EMIT connectionAccepted(addr, port);
		});

		connect(h, &RequestHandler::rejectedRequestFrom, [this](const QString & addr, quint16 port, const QString & msg) {
			Q_EMIT connectionRejected(addr, port, msg);
		});

		connect(h, &RequestHandler::requestConnectionPolicyDetermined, [this](const QString & addr, quint16 port, ConnectionPolicy policy) {
			Q_EMIT requestConnectionPolicyDetermined(addr, port, policy);
		});

		connect(h, &RequestHandler::requestActionTaken, [this](const QString & addr, quint16 port, const QString & resource, WebServerAction action) {
			Q_EMIT requestActionTaken(addr, port, resource, action);
		});

		h->start();
	}


	Configuration & Server::configuration() {
		return m_config;
	}


	bool Server::setConfiguration(const Configuration & opts) {
		Configuration realOpts = opts;

		if(isListening() && (opts.listenAddress() != m_config.listenAddress() || opts.port() != m_config.port())) {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: server already started - listen address and port changes will not take effect until server restart.\n";
		}

		m_config = realOpts;
		return true;
	}


}  // namespace EquitWebServer
