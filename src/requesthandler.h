/// \file requesthandler.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February 2018
///
/// \brief Declaration of the RequestHandler class for EquitWebServer
///
/// \par Changes
/// - (2018-02) first release.

#ifndef EQUITWEBSERVER_REQUESTHANDLER_H
#define EQUITWEBSERVER_REQUESTHANDLER_H

#include <memory>
#include <unordered_map>

#include <QDateTime>
#include <QTcpSocket>
#include <QThread>
#include <QUrl>

#include "configuration.h"

class QTcpSocket;

namespace EquitWebServer {

	using HttpHeaders = std::unordered_map<std::string, std::string>;

	class RequestHandler : public QThread {
		Q_OBJECT

	public:
		enum class HttpResponseCode : unsigned int {
			Continue = 100,
			SwitchingProtocols = 101,
			Ok = 200,
			Created = 201,
			Accepted = 202,
			NonAuthoritativeInformation = 203,
			NoContent = 204,
			ResetContent = 205,
			PartialContent = 206,
			MultipleChoices = 300,
			MovedPermanently = 301,
			Found = 302,
			SeeOther = 303,
			NotModified = 304,
			UseProxy = 305,
			Code306Unused = 306,
			TemporaryRedirect = 307,
			BadRequest = 400,
			Unauthorised = 401,
			PaymentRequired = 402,
			Forbidden = 403,
			NotFound = 404,
			MethodNotAllowed = 405,
			NotAcceptable = 406,
			ProxyAuthenticationRequired = 407,
			RequestTimeout = 408,
			Conflict = 409,
			Gone = 410,
			LengthRequired = 411,
			PreconditionFailed = 412,
			RequestEntityTooLarge = 413,
			RequestUriTooLong = 414,
			UnsupportedMediaType = 415,
			RequestRangeNotSatisfiable = 416,
			ExpectationFailed = 417,
			InternalServerError = 500,
			NotImplemented = 501,
			BadGateway = 502,
			ServiceUnavailable = 503,
			GatewayTimeout = 504,
			HttpVersionNotSupported = 505,
		};

		RequestHandler(std::unique_ptr<QTcpSocket> socket, const Configuration & opts, QObject * parent = nullptr);
		virtual ~RequestHandler() override;

		static QString defaultResponseReason(HttpResponseCode code);
		static QString defaultResponseMessage(HttpResponseCode code);

		void run() override;

		void handleHttpRequest(const std::string & httpVersion, const std::string & method, const std::string & uri, const std::string & body = {});

	Q_SIGNALS:
		void socketError(QTcpSocket::SocketError e);
		void handlingRequestFrom(const QString &, quint16);
		void acceptedRequestFrom(const QString &, quint16);
		void rejectedRequestFrom(const QString &, quint16, const QString & msg);
		void requestConnectionPolicyDetermined(const QString &, quint16, ConnectionPolicy);
		void requestActionTaken(const QString &, quint16, const QString &, WebServerAction);

	protected:
		bool sendResponse(HttpResponseCode code, const QString & title = QString::null);
		bool sendHeader(const QString & header, const QString & value);
		bool sendDateHeader(const QDateTime & d = QDateTime::currentDateTime());
		bool sendBody(const QByteArray & body);

		bool sendError(HttpResponseCode code, const QString & msg = QString::null, const QString & title = QString::null);

		bool sendData(const QByteArray & data);

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

		enum class Encoding {
			Identity = 0,
			Gzip,
		};

		/// Disposes of the socket object.
		void disposeSocket();

		/// Work out which content-encoding to use when sending body content
		void determineResponseEncoding();

		/// The TCP socket for the request being handled.
		std::unique_ptr<QTcpSocket> m_socket;

		/// The configuration of the server responding to the request.
		const Configuration & m_config;

		/// The current stage of the handler's response.
		ResponseStage m_stage;

		/// The headers parsed from the request
		HttpHeaders m_requestHeaders;

		/// The encoding being used for the response.
		Encoding m_responseEncoding;
	};

}  // namespace EquitWebServer

#endif /* EQUITWEBSERVER_REQUESTHANDLER_H */
