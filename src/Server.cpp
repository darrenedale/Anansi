/** \file Server.cpp
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the Server class for EquitWebServer
  *
  * \todo
  * - decide on application license
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  */

#include "Server.h"

#include <QDebug>

#include "RequestHandler.h"


EquitWebServer::Server::Server( const EquitWebServer::Configuration & opts ) {
	setConfiguration(opts);
}


bool EquitWebServer::Server::listen( void ) {
	if(QTcpServer::listen(QHostAddress(m_config.getListenAddress()), m_config.port()))
		return true;
	
	qDebug() << "failed to bind to port" << m_config.port() << "on ip address" << m_config.getListenAddress() << ":";
	qDebug() << errorString();
	return false;
}


void EquitWebServer::Server::close( void ) {
	QTcpServer::close();
	if(isListening()) qDebug() << "could not stop listening on" << m_config.getListenAddress() << ":" << m_config.port();
}


void EquitWebServer::Server::incomingConnection( int socket ) {
	QTcpSocket * s = new QTcpSocket();
	
	if(!s->setSocketDescriptor(socket))
		return;

	/* handler takes ownership of the socket, moving it to its own thread. the
	   handler is scheduled for deletion as soon as it completes. when it is
	   deleted, it deletes the socket with it */
	emit(connectionReceived( s->peerAddress().toString(), s->peerPort() ));
	EquitWebServer::RequestHandler * h = new EquitWebServer::RequestHandler(s, m_config, this);
	connect(h, SIGNAL(finished()), h, SLOT(deleteLater()));
	
	// pass signals from handler through signals from server
	//connect(h, SIGNAL(handlingRequestFrom(QString, quint16)), this, SIGNAL(connectionReceived(QString, quint16)));
	connect(h, SIGNAL(acceptedRequestFrom(QString, quint16)), this, SIGNAL(connectionAccepted(QString, quint16)));
	connect(h, SIGNAL(rejectedRequestFrom(QString, quint16)), this, SIGNAL(connectionRejected(QString, quint16)));
	connect(h, SIGNAL(requestConnectionPolicyDetermined( QString, quint16, int )), this, SIGNAL(requestConnectionPolicyDetermined( QString, quint16, int )));
	connect(h, SIGNAL(requestActionTaken( QString, quint16, QString, int )), this, SIGNAL(requestActionTaken( QString, quint16, QString, int )));
	connect(h, SIGNAL(requestActionTaken( QString, quint16, QString, int )), this, SLOT(slotHandlerRequestActionTaken( QString, quint16, QString, int )));
	h->start();
}


void EquitWebServer::Server::slotHandlerRequestActionTaken( QString host, quint16 port, QString path, int action ) {
	Q_UNUSED(host);
	Q_UNUSED(port);
	Q_UNUSED(path);
	Q_UNUSED(action);
}


EquitWebServer::Configuration & EquitWebServer::Server::configuration( void ) {
	return m_config;
}


bool EquitWebServer::Server::setConfiguration( const EquitWebServer::Configuration & opts ) {
	EquitWebServer::Configuration realOpts = opts;
	
	if(isListening() && (opts.getListenAddress() != m_config.getListenAddress() || opts.port() != m_config.port()))
		qDebug() << "server listening - listen address and port changes will not take effect until server restart.";
	
	m_config = realOpts;
	return true;
}
