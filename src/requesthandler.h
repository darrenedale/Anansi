/** \file RequestHandler.h
 * \author Darren Edale
 * \version 0.9.9
 * \date 19th June, 2012
 *
 * \brief Definition of the RequestHandler class for EquitWebServer
 *
 * \todo
 * - class documentation.
 * - decide on application license.
 *
 * \par Changes
 * - (2012-06-19) file documentation created.
 *
 */

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

		/**
		 * \brief Constructs a new request handler thread.
		 *
		 * \param socket is the QTcpSocket for the incoming request. It is guaranteed
		 * to be connected, open and read-write.
		 * \param opts is the configuration of the web server handling the request.
		 * \param parent is the parent object for the handler, usually the server
		 * object.
		 *
		 * \note If you create subclasses you MUST call this constructor in your
		 * derived
		 * class constructors otherwise the socket may not be properly initialised to
		 * work in your handler.
		 * \note If you create subclasses of bpWebServer you MUST ensure that the
		 * spawned
		 * handler threads receive sockets in the appropriate state.
		 */
		RequestHandler(std::unique_ptr<QTcpSocket> socket, const Configuration & opts, QObject * parent);

		/**
		 * \brief Destructor.
		 *
		 * The desctructor does little of note beyond ensuring any dynamically-allocated
		 * resources are freed.
		 */
		virtual ~RequestHandler();

		/**
		 * \brief Provides a default title for an HTTP response code.
		 *
		 * \param n is the HTTP response code.
		 *
		 * HTTP 1.1 defines the following response codes:
		 *
		 * - 100 Continue
		 * - 101 Switching Protocols
		 * - 200 OK
		 * - 201 Created
		 * - 202 Accepted
		 * - 203 Non-Authoritative Information
		 * - 204 No Content
		 * - 205 Reset Content
		 * - 206 Partial Content
		 * - 300 Multiple Choices
		 * - 301 Moved Permanently
		 * - 302 Found
		 * - 303 See Other
		 * - 304 Not Modified
		 * - 305 Use Proxy
		 * - 306 (Unused)
		 * - 307 Temporary Redirect
		 * - 400 Bad Request
		 * - 401 Unauthorised
		 * - 402 Payment Required
		 * - 403 Forbidden
		 * - 404 Not Found
		 * - 405 Method Not Allowed
		 * - 406 Not Acceptable
		 * - 407 Proxy Authentication Required
		 * - 408 Request Timeout
		 * - 409 Conflict
		 * - 410 Gone
		 * - 411 Length Required
		 * - 412 Precondition Failed
		 * - 413 Request Entity Too Large
		 * - 414 Request-URI Too Long
		 * - 415 Unsupported Media Type
		 * - 416 Requested Range Not Satisfiable
		 * - 417 Expectation Failed
		 * - 500 Internal Server Error
		 * - 501 Not Implemented
		 * - 502 Bad Gateway
		 * - 503 Service Unavailable
		 * - 504 Gateway Timeout
		 * - 505 HTTP Version Not Supported
		 *
		 * \return The default title for the response code, or \c Unknown if the
		 * response code
		 * is not recognised.
		 */
		static QString defaultResponseReason(HttpResponseCode code);
		static QString defaultResponseMessage(HttpResponseCode code);

		/**
		 * \brief Point of entry for the thread.
		 *
		 * This is where the handler starts execution. This method simply sets up the
		 * socket
		 * object, reads and parses the request line from the socket, and passes the
		 * details on
		 * to the handleHTTPRequest() method.
		 */
		void run();

		/**
		 * \brief Handle a parsed HTTP request.
		 *
		 * \param request is the request to handle.
		 * \param body is the message body sent with the request.
		 *
		 * The default implementation handles HTTP 1.1 requests. Future or later
		 * versions of
		 * the protocol can be handled using subclasses.
		 *
		 * \note At present, only requests using the GET method are accepted.
		 * \note This method assumes that the member m_socket is initialised and is
		 * both
		 * readable and writeable.
		 */
		void handleHttpRequest(const std::string & version, const std::string & method, const std::string & uri, const HttpHeaders & headers, const QByteArray & body = "");

		/**
		 * \brief Sends a HTTP response to the client.
		 *
		 * \param code is the HTTP response code. See the HTTP protocol documentation
		 * for details.
		 * \param title is the optional custom title for the response. If missing, the
		 * default response title will be used.
		 *
		 * \return @c true if the response was sent, \c false otherwise.
		 */
		bool sendResponse(HttpResponseCode code, const QString & title = QString::null);

		/**
		 * \brief Sends a HTTP header to the client.
		 *
		 * \param header is the header to send.
		 * \param value is the value to send for the header.
		 *
		 * A call to this method will put the handler into the \c Headers stage. If
		 * the handler is already beyond this stage, the call will fail. After a
		 * successful call to this method, no more response codes may be sent.
		 *
		 * \return @c true if the header was sent, \c false otherwise.
		 */
		bool sendHeader(const QString & header, const QString & value);

		/**
		 * \brief Convenience method to send a date header.
		 *
		 * \param d is the date to send in the header.
		 *
		 * \return @c true if the header was sent, \c false otherwise.
		 */
		bool sendDateHeader(const QDateTime & d = QDateTime::currentDateTime());

		/**
		 * \brief Sends some body content to the client.
		 *
		 * \param body is the content to send.
		 *
		 * A call to this method will put the handler in the \c Body stage.
		 * Subsequently,
		 * no more headers or responses can be sent.
		 *
		 * Body content may be sent in more than one chunk, using multiple calls to
		 * this
		 * method. The call will fail if the handler is already in the Completed
		 * stage.
		 *
		 * \return @c true if the body content was sent, \c false otherwise.
		 */
		bool sendBody(const QByteArray & body);

		/**
		 * \brief Sends a complete error response to the client.
		 *
		 * \param code is the error code. See the HTTP protocol documentation for
		 * details.
		 * \param msg is a text message to send in the body of the response. The
		 * message
		 * will be enclosed in a paragraph in the body section of the HTML.
		 * \param title is a custom title to use for the error.
		 *
		 * Both the message and title are optional. If not provided, the default
		 * message
		 * title will be used. If just the title is provided, the custom title will be
		 * used
		 * for the message also.
		 *
		 * This method will send a complete HTTP response, including response number,
		 * headers
		 * and body content. If the handler is already beyond the Response stage, the
		 * call
		 * will fail. If the call succeeds, the handler will be put in the Completed
		 * stage.
		 *
		 * \return @c true if the error was sent, \c false otherwise.
		 */
		bool sendError(HttpResponseCode code, const QString & msg = QString::null, const QString & title = QString::null);

	Q_SIGNALS:
		void socketError(QTcpSocket::SocketError e);
		void handlingRequestFrom(const QString &, quint16);
		void acceptedRequestFrom(QString, quint16);
		void rejectedRequestFrom(const QString & ip, quint16 port, const QString & msg);
		void requestConnectionPolicyDetermined(QString, quint16, Configuration::ConnectionPolicy);
		void requestActionTaken(QString, quint16, QString, Configuration::WebServerAction);

	protected:
		/// Sends raw data over the TCP socket
		bool sendData(const QByteArray & data);

	private:
		void staticInitilise();

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

		// the default CSS for directory listings
		static std::string m_dirListingCss;
		static bool m_staticInitDone;

		/*
		 * needs to be allocated on the heap because stack allocation would occur at
		 * contruction of the handler object, which means the socket would be owned by
		 * the server thread, not the handler thread, and sockets cannot be controlled
		 * between threads. in other words, the handler thread must own the socket object.
		 */
		/// The TCP socket for the request being handled.
		std::unique_ptr<QTcpSocket> m_socket;

		/// The configuration of the server responding to the request.
		Configuration m_config;

		/// The current stage of the handler's response.
		ResponseStage m_stage;

		/// Disposes the socket object.
		void disposeSocketObject();
	};

}  // namespace EquitWebServer

#endif /* EQUITWEBSERVER_REQUESTHANDLER_H */
