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

/// \file requesthandler.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the RequestHandler class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_REQUESTHANDLER_H
#define ANANSI_REQUESTHANDLER_H

#include <memory>
#include <unordered_map>

#include <QDateTime>
#include <QTcpSocket>
#include <QThread>
#include <QUrl>

#include "types.h"
#include "configuration.h"

class QTcpSocket;

namespace Anansi {

	class ContentEncoder;

	class RequestHandler final : public QThread {
		Q_OBJECT

	public:
		RequestHandler(std::unique_ptr<QTcpSocket> socket, const Configuration & opts, QObject * parent = nullptr);
		virtual ~RequestHandler() override;

		static QString defaultResponseReason(HttpResponseCode code);
		static QString defaultResponseMessage(HttpResponseCode code);

		void run() override;
		void handleHttpRequest();


	Q_SIGNALS:
		void socketError(QTcpSocket::SocketError e);
		void handlingRequestFrom(const QString &, quint16);
		void acceptedRequestFrom(const QString &, quint16);
		void rejectedRequestFrom(const QString &, quint16, const QString & msg);
		void requestConnectionPolicyDetermined(const QString &, quint16, ConnectionPolicy);
		void requestActionTaken(const QString &, quint16, const QString &, WebServerAction);

	private:
		/**
		 * \brief Enumerates the stages of response through which the handler passes.
		 *
		 * In each stage certain actions are valid. Actions for later stages may occur
		 * while the handler is in earlier stages, but actions for earlier stages may
		 * NOT occur when the handler is in later stages.
		 *
		 * For example, sending body content is valid when the handler is in the `Headers`
		 * stage, but sending headers is not valid when the handler is in the `Body` stage.
		 * Successfully performing any action tied to a stage will place the handler
		 * in that stage.
		 */
		enum class ResponseStage {
			SendingResponse = 0,
			SendingHeaders,
			SendingBody,
			Completed
		};


		bool sendData(const QByteArray & data);

		bool sendResponseCode(HttpResponseCode code, const QString & title = QString::null);

		template<class StringType>
		bool sendHeader(const StringType & header, const StringType & value);

		inline bool sendHeader(const HttpHeaders::value_type & header) {
			return sendHeader(header.first, header.second);
		}

		inline bool sendHeaders(const HttpHeaders & headers) {
			for(const auto & header : headers) {
				if(!sendHeader(header)) {
					return false;
				}
			}

			return true;
		}

		bool sendDateHeader(const QDateTime & d = QDateTime::currentDateTime());

		bool sendBody(const QByteArray & body);
		bool sendBody(QIODevice & in, const std::optional<int> & bytes = {});

		bool sendError(HttpResponseCode code, QString msg = {}, const QString & title = {});

		void sendDirectoryListing(const QString & localPath);
		void sendFile(const QString & localPath, const QString & mimeType);
		void doCgi(const QString & localPath, const QString & mimeType);

		/// Disposes of the socket object.
		void disposeSocket();

		ConnectionPolicy determineConnectionPolicy();

		// Read the incoming request details
		std::optional<std::tuple<std::string, std::string, std::string>> readHttpRequestLine();
		bool readRequestHeaders();
		std::optional<int> parseRequestContentLength();
		bool readRequestBody(int contentLength = -1);

		/// Work out which content-encoding to use when sending body content
		bool determineResponseEncoding();

		/// The TCP socket for the request being handled.
		std::unique_ptr<QTcpSocket> m_socket;

		/// The configuration of the server responding to the request.
		const Configuration & m_config;

		/// The current stage of the handler's response.
		ResponseStage m_stage;

		/// The headers parsed from the request
		HttpHeaders m_requestHeaders;
		HttpMethod m_requestMethod;
		std::string m_requestMethodString;
		std::string m_requestHttpVersion;
		std::string m_requestUri;
		std::string m_requestUriPath;
		std::string m_requestUriQuery;
		std::string m_requestUriFragment;
		std::string m_requestBody;

		/// The encoding being used for the response.
		ContentEncoding m_responseEncoding;
		std::unique_ptr<ContentEncoder> m_encoder;
	};

}  // namespace Anansi

#endif /* ANANSI_REQUESTHANDLER_H */
