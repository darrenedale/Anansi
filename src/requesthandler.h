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
/// \dep
/// - <memory>
/// - <QThread>
/// - <QString>
/// - <QTcpSocket>
/// - <QDateTime>
/// - configuration.h
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_REQUESTHANDLER_H
#define ANANSI_REQUESTHANDLER_H

#include <memory>

#include <QThread>
#include <QString>
#include <QTcpSocket>
#include <QDateTime>

#include "types.h"

class QByteArray;
class QIODevice;

namespace Anansi {

	class ContentEncoder;
	class Configuration;

	class RequestHandler : public QThread {
		Q_OBJECT

	public:
		RequestHandler(std::unique_ptr<QTcpSocket>, const Configuration &, QObject * = nullptr);
		virtual ~RequestHandler() override;

		static QString defaultResponseReason(HttpResponseCode);
		static QString defaultResponseMessage(HttpResponseCode);

		virtual void run() override;
		virtual void handleHttpRequest();

	Q_SIGNALS:
		void handlingRequestFrom(const QString &, uint16_t) const;
		void acceptedRequestFrom(const QString &, uint16_t) const;
		void rejectedRequestFrom(const QString &, uint16_t, const QString &) const;
		void requestConnectionPolicyDetermined(const QString &, uint16_t, ConnectionPolicy) const;
		void requestActionTaken(const QString &, uint16_t, const QString &, WebServerAction) const;

	private:
		enum class ResponseStage {
			SendingResponse = 0,
			SendingHeaders,
			SendingBody,
			Completed
		};

		struct HttpRequestLine {
			std::string method;
			std::string uri;
			std::string httpVersion;
		};

		struct HttpRequestUri {
			std::string path;
			std::string query;
			std::string fragment;
		};

		static std::optional<HttpRequestLine> parseHttpRequestLine(const std::string &);
		static std::optional<int> parseContentLengthValue(const std::string &);

		bool sendData(const QByteArray &);
		bool sendResponseCode(HttpResponseCode, const std::optional<QString> & = {});

		template<class StringType>
		bool sendHeader(const StringType &, const StringType &);

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

		bool sendDateHeader(const QDateTime & = QDateTime::currentDateTime());

		bool sendBody(const QByteArray &);
		bool sendBody(QIODevice &, const std::optional<int> & = {});

		bool sendError(HttpResponseCode, QString = {}, QString = {});
		void sendDirectoryListing(const QString &);
		void sendFile(const QString &, const QString &);
		void doCgi(const QString &, const QString &);

		void disposeSocket();

		ConnectionPolicy determineConnectionPolicy() const;
		bool readRequestHeaders();
		bool readRequestBody(std::optional<int> = {});
		bool determineResponseEncoding();

		std::unique_ptr<QTcpSocket> m_socket;
		const Configuration & m_config;
		ResponseStage m_stage;

		HttpHeaders m_requestHeaders;
		HttpRequestLine m_requestLine;
		HttpMethod m_requestMethod;
		HttpRequestUri m_requestUri;
		std::string m_requestBody;

		ContentEncoding m_responseEncoding;
		std::unique_ptr<ContentEncoder> m_encoder;
	};

}  // namespace Anansi

#endif /* ANANSI_REQUESTHANDLER_H */
