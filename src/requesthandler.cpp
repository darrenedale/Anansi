/** \file RequestHandler.cpp
 * \author Darren Edale
 * \version 0.9.9
 * \date 19th June, 2012
 *
 * \brief Implementation of the RequestHandler class for EquitWebServer
 *
 * \todo configuration option to ignore hidden files in directory listing
 * \todo configuration option to order dirs first, then alpha in directory listing
 *
 * \par Changes
 * - (2012-06-22) directory listings no longer need default action to be
 *   "serve". The config item for allowing directory listings effectively
 *   works as if the "directory" mime type was set to "Serve".
 * - (2012-06-19) file documentation created.
 */

#include "requesthandler.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <regex>

#include <QApplication>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QProcess>
#include <QStringList>
#include <QStringBuilder>
#include <QTcpSocket>
#include <QUrl>

#include "server.h"
#include "strings.h"
#include "scopeguard.h"
#include "mimeicons.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);
Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);


namespace EquitWebServer {


	static constexpr const int MaxConsecutiveTimeouts = 3;
	static const QByteArray EOL = QByteArrayLiteral("\r\n");

	static std::string dirListingCss = ([]() -> std::string {
		QFile staticResourceFile(QStringLiteral(":/stylesheets/directory-listing"));

		if(!staticResourceFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read built-in directory listing stylesheet (couldn't open resource file)\n";
			return {};
		}

		std::string ret;
		ret.reserve(static_cast<std::string::size_type>(staticResourceFile.size() + 1));

		while(!staticResourceFile.atEnd()) {
			ret += staticResourceFile.readAll().constData();
		}

		return ret;
	})();


	/**
	 * \brief Constructs a new request handler thread.
	 *
	 * \param socket is the QTcpSocket for the incoming request. It is guaranteed
	 * to be connected, open and read-write.
	 * \param opts is the configuration of the web server handling the request.
	 * \param parent is the parent object for the handler, usually the server
	 * object.
	 *
	 * \warning The Configuration provided must be guaranteed to exist for the duration
	 * of the request handlers lifetime.
	 * \note If you create subclasses you MUST call this constructor in your derived
	 * class constructors otherwise the socket may not be properly initialised to work
	 * in your handler.
	 * \note If you create subclasses of bpWebServer you MUST ensure that the spawned
	 * handler threads receive sockets in the appropriate state.
	 */
	RequestHandler::RequestHandler(std::unique_ptr<QTcpSocket> socket, const Configuration & opts, QObject * parent)
	: QThread(parent),
	  m_socket(std::move(socket)),
	  m_config(opts),
	  m_stage(ResponseStage::SendingResponse) {
		Q_ASSERT(m_socket);
		m_socket->moveToThread(this);
	}


	/**
	 * \brief Destructor.
	 *
	 * The desctructor does little of note beyond ensuring any dynamically-allocated
	 * resources are freed.
	 */
	RequestHandler::~RequestHandler() {
		disposeSocket();
	}


	void RequestHandler::disposeSocket() {
		if(m_socket) {
			if(QAbstractSocket::ConnectedState == m_socket->state()) {
				m_socket->disconnectFromHost();

				if(!m_socket->waitForDisconnected()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error disconnecting socket (" << qPrintable(m_socket->errorString()) << ")\n";
				}
			}

			m_socket.reset(nullptr);
		}
	}


	/// Sends raw data over the TCP socket
	bool RequestHandler::sendData(const QByteArray & data) {
		if(!m_socket->isWritable()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: tcp socket  is not writable\n";
			return false;
		}

		int64_t bytes;
		int remaining = data.size();
		const char * dataToWrite = data.data();

		/// TODO might need a timeout in case we continually write no data
		while(0 < remaining) {
			bytes = m_socket->write(dataToWrite, remaining);

			if(bytes == -1) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error writing to TCP socket (\"" << qPrintable(m_socket->errorString()) << "\")\n";
				return false;
			}

			dataToWrite += bytes;
			remaining -= bytes;
		}

		return true;
	}


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
	QString RequestHandler::defaultResponseReason(HttpResponseCode code) {
		switch(code) {
			case HttpResponseCode::Continue:
				return QApplication::tr("Continue");

			case HttpResponseCode::SwitchingProtocols:
				return QApplication::tr("Switching Protocols");

			case HttpResponseCode::Ok:
				return QApplication::tr("OK");

			case HttpResponseCode::Created:
				return QApplication::tr("Created");

			case HttpResponseCode::Accepted:
				return QApplication::tr("Accepted");

			case HttpResponseCode::NonAuthoritativeInformation:
				return QApplication::tr("Non-Authoritative Information");

			case HttpResponseCode::NoContent:
				return QApplication::tr("No Content");

			case HttpResponseCode::ResetContent:
				return QApplication::tr("Reset Content");

			case HttpResponseCode::PartialContent:
				return QApplication::tr("Partial Content");

			case HttpResponseCode::MultipleChoices:
				return QApplication::tr("Multiple Choices");

			case HttpResponseCode::MovedPermanently:
				return QApplication::tr("Moved Permanently");

			case HttpResponseCode::Found:
				return QApplication::tr("Found");

			case HttpResponseCode::SeeOther:
				return QApplication::tr("See Other");

			case HttpResponseCode::NotModified:
				return QApplication::tr("Not Modified");

			case HttpResponseCode::UseProxy:
				return QApplication::tr("Use Proxy");

			case HttpResponseCode::Code306Unused:
				return QApplication::tr("(Unused)");

			case HttpResponseCode::TemporaryRedirect:
				return QApplication::tr("Temporary Redirect");

			case HttpResponseCode::BadRequest:
				return QApplication::tr("Bad Request");

			case HttpResponseCode::Unauthorised:
				return QApplication::tr("Unauthorised");

			case HttpResponseCode::PaymentRequired:
				return QApplication::tr("Payment Required");

			case HttpResponseCode::Forbidden:
				return QApplication::tr("Forbidden");

			case HttpResponseCode::NotFound:
				return QApplication::tr("Not Found");

			case HttpResponseCode::MethodNotAllowed:
				return QApplication::tr("Method Not Allowed");

			case HttpResponseCode::NotAcceptable:
				return QApplication::tr("Not Acceptable");

			case HttpResponseCode::ProxyAuthenticationRequired:
				return QApplication::tr("Proxy Authentication Required");

			case HttpResponseCode::RequestTimeout:
				return QApplication::tr("Request Timeout");

			case HttpResponseCode::Conflict:
				return QApplication::tr("Conflict");

			case HttpResponseCode::Gone:
				return QApplication::tr("Gone");

			case HttpResponseCode::LengthRequired:
				return QApplication::tr("Length Required");

			case HttpResponseCode::PreconditionFailed:
				return QApplication::tr("Precondition Failed");

			case HttpResponseCode::RequestEntityTooLarge:
				return QApplication::tr("Request Entity Too Large");

			case HttpResponseCode::RequestUriTooLong:
				return QApplication::tr("Request-URI Too Long");

			case HttpResponseCode::UnsupportedMediaType:
				return QApplication::tr("Unsupported Media Type");

			case HttpResponseCode::RequestRangeNotSatisfiable:
				return QApplication::tr("Requested Range Not Satisfiable");

			case HttpResponseCode::ExpectationFailed:
				return QApplication::tr("Expectation Failed");

			case HttpResponseCode::InternalServerError:
				return QApplication::tr("Internal Server Error");

			case HttpResponseCode::NotImplemented:
				return QApplication::tr("Not Implemented");

			case HttpResponseCode::BadGateway:
				return QApplication::tr("Bad Gateway");

			case HttpResponseCode::ServiceUnavailable:
				return QApplication::tr("Service Unavailable");

			case HttpResponseCode::GatewayTimeout:
				return QApplication::tr("Gateway Timeout");

			case HttpResponseCode::HttpVersionNotSupported:
				return QApplication::tr("HTTP Version Not Supported");
		}

		return QApplication::tr("Unknown");
	}


	QString RequestHandler::defaultResponseMessage(HttpResponseCode code) {
		switch(code) {
			case HttpResponseCode::Continue:
				return QApplication::tr("Continue");

			case HttpResponseCode::SwitchingProtocols:
				return QApplication::tr("Switching Protocols");

			case HttpResponseCode::Ok:
				return QApplication::tr("The request was accepted and will be honoured.");

			case HttpResponseCode::Created:
				return QApplication::tr("Created");

			case HttpResponseCode::Accepted:
				return QApplication::tr("Accepted");

			case HttpResponseCode::NonAuthoritativeInformation:
				return QApplication::tr("Non-Authoritative Information");

			case HttpResponseCode::NoContent:
				return QApplication::tr("No Content");

			case HttpResponseCode::ResetContent:
				return QApplication::tr("Reset Content");

			case HttpResponseCode::PartialContent:
				return QApplication::tr("Partial Content");

			case HttpResponseCode::MultipleChoices:
				return QApplication::tr("Multiple Choices");

			case HttpResponseCode::MovedPermanently:
				return QApplication::tr("Moved Permanently");

			case HttpResponseCode::Found:
				return QApplication::tr("Found");

			case HttpResponseCode::SeeOther:
				return QApplication::tr("See Other");

			case HttpResponseCode::NotModified:
				return QApplication::tr("Not Modified");

			case HttpResponseCode::UseProxy:
				return QApplication::tr("Use Proxy");

			case HttpResponseCode::Code306Unused:
				return QApplication::tr("(Unused)");

			case HttpResponseCode::TemporaryRedirect:
				return QApplication::tr("Temporary Redirect");

			case HttpResponseCode::BadRequest:
				return QApplication::tr("Bad Request");

			case HttpResponseCode::Unauthorised:
				return QApplication::tr("Unauthorised");

			case HttpResponseCode::PaymentRequired:
				return QApplication::tr("Payment Required");

			case HttpResponseCode::Forbidden:
				return QApplication::tr("The request could not be fulfilled because you are not allowed to access the resource requested.");

			case HttpResponseCode::NotFound:
				return QApplication::tr("The resource requested could not be located on this server.");

			case HttpResponseCode::MethodNotAllowed:
				return QApplication::tr("Method Not Allowed");

			case HttpResponseCode::NotAcceptable:
				return QApplication::tr("Not Acceptable");

			case HttpResponseCode::ProxyAuthenticationRequired:
				return QApplication::tr("Proxy Authentication Required");

			case HttpResponseCode::RequestTimeout:
				return QApplication::tr("The request could not be fulfilled because it took too long to process. If the server is currently busy, it may be possible to successfully fulfil the request later.");

			case HttpResponseCode::Conflict:
				return QApplication::tr("Conflict");

			case HttpResponseCode::Gone:
				return QApplication::tr("The requested resource has been permanently removed from this server.");

			case HttpResponseCode::LengthRequired:
				return QApplication::tr("Length Required");

			case HttpResponseCode::PreconditionFailed:
				return QApplication::tr("Precondition Failed");

			case HttpResponseCode::RequestEntityTooLarge:
				return QApplication::tr("Request Entity Too Large");

			case HttpResponseCode::RequestUriTooLong:
				return QApplication::tr("The request could not be fulfilled because the identifier of the resource requested was too long to process.");

			case HttpResponseCode::UnsupportedMediaType:
				return QApplication::tr("Unsupported Media Type");

			case HttpResponseCode::RequestRangeNotSatisfiable:
				return QApplication::tr("Requested Range Not Satisfiable");

			case HttpResponseCode::ExpectationFailed:
				return QApplication::tr("Expectation Failed");

			case HttpResponseCode::InternalServerError:
				return QApplication::tr("The request could not be fulfilled because of an unexpected internal error in the server.");

			case HttpResponseCode::NotImplemented:
				return QApplication::tr("The request could not be fulfilled because it is of an unsupported type.");

			case HttpResponseCode::BadGateway:
				return QApplication::tr("Bad Gateway");

			case HttpResponseCode::ServiceUnavailable:
				return QApplication::tr("Service Unavailable");

			case HttpResponseCode::GatewayTimeout:
				return QApplication::tr("Gateway Timeout");

			case HttpResponseCode::HttpVersionNotSupported:
				return QApplication::tr("HTTP Version Not Supported");
		}

		return QApplication::tr("Unknown response code.");
	}


	/**
	 * \brief Sends a HTTP response to the client.
	 *
	 * \param code is the HTTP response code. See the HTTP protocol documentation
	 * for details.
	 * \param title is the optional custom title for the response. If missing, the
	 * default response title will be used.
	 *
	 * The request handler must be in the SendingResponse stage. The outcome is
	 * undefined if this is not the case. (Request handlers are in the appropriate
	 * stage upon construction and move through other stages as the response, headers
	 * and body data are send.)
	 *
	 * \return `true` if the response was sent, `false` otherwise.
	 */
	bool RequestHandler::sendResponse(HttpResponseCode code, const QString & title) {
		Q_ASSERT_X(ResponseStage::SendingResponse == m_stage, __PRETTY_FUNCTION__, "must be in SendingResponse stage to send the HTTP response header");
		return sendData(QByteArrayLiteral("HTTP/1.1 ") % QString::number(static_cast<unsigned int>(code)).toUtf8() % ' ' % (title.isNull() ? RequestHandler::defaultResponseReason(code).toUtf8() : title.toUtf8()) + EOL);
	}


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
	bool RequestHandler::sendHeader(const QString & header, const QString & value) {
		Q_ASSERT_X(ResponseStage::SendingResponse == m_stage || ResponseStage::SendingHeaders == m_stage, __PRETTY_FUNCTION__, "must be in SendingResponse or SendingHeaders stage to send a header");
		m_stage = ResponseStage::SendingHeaders;
		return sendData(header.toUtf8() % QByteArrayLiteral(": ") % value.toUtf8() % EOL);
	}


	/**
	 * \brief Convenience method to send a date header.
	 *
	 * \param d is the date to send in the header.
	 *
	 * \return @c true if the header was sent, \c false otherwise.
	 */
	bool RequestHandler::sendDateHeader(const QDateTime & d) {
		return sendHeader(QStringLiteral("Date"), d.toUTC().toString(QStringLiteral("ddd, d MMM yyyy hh:mm:ss")) % QStringLiteral(" GMT"));
	}


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
	bool RequestHandler::sendBody(const QByteArray & body) {
		Q_ASSERT_X(m_stage != ResponseStage::Completed, __PRETTY_FUNCTION__, "cannot send body after request response has been fulfilled");

		if(m_stage != ResponseStage::SendingBody) {
			sendData(EOL);
			m_stage = ResponseStage::SendingBody;
		}

		return sendData(body);
	}


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
	bool RequestHandler::sendError(HttpResponseCode code, const QString & msg, const QString & title) {
		Q_ASSERT_X(ResponseStage::SendingResponse == m_stage, __PRETTY_FUNCTION__, "cannot send a complete error response when header or body content has already been sent.");

		QString myTitle = (title.isEmpty() ? RequestHandler::defaultResponseReason(code) : title);

		if(!sendResponse(code, myTitle)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of response line for error failed.\n";
			return false;
		}

		if(!sendDateHeader() || !sendHeader("Content-type", "text/html")) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of header for error failed.\n";
			return false;
		}

		QString myMsg = (msg.isEmpty() ? RequestHandler::defaultResponseMessage(code) : msg);

		if(!sendBody(QByteArrayLiteral("<html><head><title>") % html_escape(myTitle).toUtf8() % QByteArrayLiteral("</title></head><body><h1>") % QByteArray::number(static_cast<unsigned int>(code)) % ' ' % html_escape(myTitle).toUtf8() % QByteArrayLiteral("</h1><p>") % html_escape(myMsg).toUtf8() % QByteArrayLiteral("</p></body></html>"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of body content for error failed.\n";
			return false;
		}

		m_stage = ResponseStage::Completed;
		return true;
	}


	/**
	 * \brief Point of entry for the thread.
	 *
	 * This is where the handler starts execution. This method simply sets up the
	 * socket
	 * object, reads and parses the request line from the socket, and passes the
	 * details on
	 * to the handleHTTPRequest() method.
	 */
	void RequestHandler::run() {
		Q_ASSERT_X(m_socket, __PRETTY_FUNCTION__, "null socket");

		// scope guard automatically does all cleanup on all exit paths
		auto cleanup = Equit::ScopeGuard([this]() {
			m_socket->flush();
			disposeSocket();
		});

		/* check controls on remote IP */
		QString clientAddress = m_socket->peerAddress().toString();
		uint16_t clientPort = m_socket->peerPort();
		Q_EMIT handlingRequestFrom(clientAddress, clientPort);
		ConnectionPolicy policy = m_config.ipAddressPolicy(clientAddress);
		Q_EMIT requestConnectionPolicyDetermined(clientAddress, clientPort, policy);

		switch(policy) {
			case ConnectionPolicy::Accept:
				Q_EMIT acceptedRequestFrom(clientAddress, clientPort);
				break;

			case ConnectionPolicy::None:
			case ConnectionPolicy::Reject:
				Q_EMIT rejectedRequestFrom(clientAddress, clientPort, tr("Policy for this IP address is Reject"));
				sendError(HttpResponseCode::Forbidden);
				return;
		}

		std::array<char, 100> socketReadBuffer;

		const auto nextHeaderLine = [this, &socketReadBuffer]() -> std::optional<std::string> {
			std::string line;
			int consecutiveTimeoutCount = 0;

			while(!m_socket->canReadLine()) {
				if(!m_socket->waitForReadyRead()) {
					if(m_socket->error() != QAbstractSocket::SocketTimeoutError) {
						std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket error reading header line (\"" << qPrintable(m_socket->errorString()) << "\"\n";
						return {};
					}

					++consecutiveTimeoutCount;

					if(MaxConsecutiveTimeouts < consecutiveTimeoutCount) {
						std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: too many timeouts attempting to read request header\n";
						return {};
					}
				}
				else {
					consecutiveTimeoutCount = 0;
				}
			}

			int64_t length = 0;

			do {
				// -1 should never happen because canReadLine is known to be true
				length += m_socket->readLine(socketReadBuffer.data(), socketReadBuffer.size());
				line += socketReadBuffer.data();
			} while(0 == length || '\n' != line.back());

			// no need to check for trailing \n - we know it's there because it's a read loop exit condition
			if(2 > length || '\r' != line[static_cast<std::string::size_type>(length) - 2]) {
				return {};
			}

			// trim off trailing \r\n
			const auto end = line.cend();
			line.erase(end - 2, end);
			return line;
		};

		auto requestLine = nextHeaderLine();

		if(!requestLine) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid request line)\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		std::regex headerRx("^(OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT) ([^ ]+) HTTP/([0-9](?:\\.[0-9]+)*)$");
		std::smatch captures;

		if(!std::regex_match(*requestLine, captures, headerRx)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid request line)\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		std::string method = captures[1];
		std::string uri = captures[2];
		std::string version = captures[3];

		HttpHeaders headers;
		headerRx = "^([a-zA-Z][a-zA-Z\\-]*) *: *(.+)$";

		while(true) {
			auto headerLine = nextHeaderLine();

			if(!headerLine) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid header)\n";
				sendError(HttpResponseCode::BadRequest);
				return;
			}

			if(headerLine->empty()) {
				// all headers read
				break;
			}

			if(!std::regex_match(*headerLine, captures, headerRx)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid header \"" << *headerLine << "\")\n";
				sendError(HttpResponseCode::BadRequest);
				return;
			}

			headers.emplace(to_lower(captures.str(1)), captures.str(2));
		}

		/* whatever extra we already read beyond headers is body */
		const auto contentLengthIt = headers.find("content-length");
		auto contentLength = -1L;

		if(contentLengthIt != headers.end()) {
			char * end;
			contentLength = static_cast<long>(std::strtoul(contentLengthIt->second.data(), &end, 10));

			if(end) {
				while(' ' == *end) {
					++end;
				}
			}

			if(!end || 0 != *end) {
				// conversion failure, or extraneous non-whitespace after content-length
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid content-length header)\n";
				sendError(HttpResponseCode::BadRequest);
				return;
			}
		}

		std::string body;
		auto bytesRemaining = contentLength;
		int consecutiveTimeoutCount = 0;

		if(0 < contentLength && body.capacity() < static_cast<std::string::size_type>(contentLength)) {
			body.reserve(static_cast<std::string::size_type>(contentLength));  // +1 for null?
		}

		while((-1 == contentLength || 0 < bytesRemaining) && !m_socket->atEnd()) {
			auto bytesRead = m_socket->read(&socketReadBuffer[0], socketReadBuffer.size());

			if(-1 == bytesRead) {
				if(QAbstractSocket::SocketTimeoutError != m_socket->error()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error reading body data from socket (" << qPrintable(m_socket->errorString()) << ")\n";
					sendError(HttpResponseCode::BadRequest);
					return;
				}

				++consecutiveTimeoutCount;

				if(MaxConsecutiveTimeouts < consecutiveTimeoutCount) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: too many timeouts attempting to read request body\n";
					sendError(HttpResponseCode::BadRequest);
					return;
				}
			}
			else {
				body.append(&socketReadBuffer[0], static_cast<std::string::size_type>(bytesRead));
				consecutiveTimeoutCount = 0;
			}
		}

		if(0 < bytesRemaining) {
			// not enough body data
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket stopped providing data while still expecting " << bytesRemaining << " bytes (\"" << qPrintable(m_socket->errorString()) << "\")\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		if(!m_socket->atEnd() || (-1 != contentLength && 0 > bytesRemaining)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket provided more body data than expected (at least " << (-bytesRemaining) << " surplus bytes)\n";
		}

		handleHttpRequest(version, method, uri, headers, body);
	}


	/**
	 * \brief Handle a parsed HTTP request.
	 *
	 * \param httpVersion is the HTTP version of the request (usually 1.1).
	 * \param method is the HTTP method the request is using.
	 * \param uri is the URI of the resource requested.
	 * \param headers is the set of headers sent with the request.
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
	void RequestHandler::handleHttpRequest(const std::string & version, const std::string & method, const std::string & reqUri, const HttpHeaders & headers, const std::string & body) {
		if(!m_socket) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no socket\n";
			return;
		}

		// will accept anything up to HTTP/1.1 and process it as HTTP/1.1
		if("1.0" != version && "1.1" != version) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: HTTP version (HTTP/" << version << ") is not supported\n";
			sendError(HttpResponseCode::HttpVersionNotSupported);
			return;
		}

		// TODO for now we only support GET, HEAD and POST, which covers the only REQUIRED HTTP/1.1 methods (GET, HEAD).
		// in future we may need to support all HTTP 1.1 methods: OPTIONS,GET,HEAD,POST,PUT,DELETE,TRACE,CONNECT
		if("GET" != method && "HEAD" != method && "POST" != method) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Request method" << method << "not supported\n";
			sendError(HttpResponseCode::NotImplemented);
			return;
		}

		std::regex rxUri("^([^?#]*)(?:\\?([^#]+))?(?:#(.*))?$");
		std::smatch captures;

		if(!std::regex_match(reqUri, captures, rxUri)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed parsing request URI \"" << reqUri << "\"\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		auto uriPath = percent_decode(captures[1].str());
		std::string uriQuery;
		std::string uriFragment;

		if(2 < captures.size()) {
			uriQuery = captures[2].str();
		}

		// TODO we should never receive a fragment, should we?
		if(3 < captures.size()) {
			uriFragment = captures[3].str();
		}

		const auto & md5It = headers.find("content-md5");

		if(md5It != headers.cend()) {
			QCryptographicHash hash(QCryptographicHash::Md5);
			hash.addData(body.data(), static_cast<int>(body.size()));

			if(md5It->second != hash.result().toHex().constData()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: calculated MD5 of request body does not match Content-MD5 header\n";
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: calculated:" << hash.result().toHex().constData() << "; header:" << md5It->second << "\n";
				// TODO is this the correct response for this error case?
				sendError(HttpResponseCode::BadRequest);
			}

			return;
		}

		QFileInfo docRoot(m_config.documentRoot());
		QFileInfo resource(docRoot.absoluteFilePath() + "/" + QString::fromStdString(uriPath));  //uri.toLocalFile());
		QString resolvedResourcePath = resource.absoluteFilePath();

		// only serve request from inside doc root
		if(!resolvedResourcePath.startsWith(docRoot.absoluteFilePath())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: requested local resource is outside document root.\n";
			sendError(HttpResponseCode::NotFound);
			return;
		}

		const auto clientAddr = m_socket->peerAddress().toString();
		const auto clientPort = m_socket->peerPort();

		if(resource.isDir()) {
			if(!m_config.isDirectoryListingAllowed()) {
				sendError(HttpResponseCode::Forbidden);
				Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::Forbid);
				return;
			}

			// TODO config option for charset
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::Serve);
			sendResponse(HttpResponseCode::Ok);
			sendDateHeader();
			sendHeader("Content-type", "text/html; charset=UTF-8");

			/* strip trailing "/" from path */
			auto path = reqUri;

			{
				auto pathIt = path.rbegin();
				const auto pathEnd = path.crend();

				while(pathIt != pathEnd && '/' == *pathIt) {
					++pathIt;
				}

				// TODO if pathIt == pathEnd, what is pathIt.base()?
				path.erase(pathIt.base(), path.cend());
			}

			const auto htmlPath = html_escape(QUrl::fromPercentEncoding(path.data()).toUtf8());
			QByteArray responseBody = QByteArrayLiteral("<html>\n<head><title>Directory listing for ") % htmlPath % QByteArrayLiteral("</title><style>") % QByteArray::fromStdString(dirListingCss) % QByteArrayLiteral("</style></head>\n<body>\n<div id=\"header\"><p>Directory listing for <em>") % htmlPath % QByteArrayLiteral("/</em></p></div>\n<div id=\"content\"><ul class=\"directory-listing\">");

			if("" != path) {
				auto parentPath = path;
				auto pos = parentPath.rfind('/');

				if(pos != decltype(parentPath)::npos) {
					parentPath.erase(pos);
				}

				responseBody += QByteArray("<li><img src=\"" % mimeIconUri("inode/directory").toUtf8() % "\" />&nbsp;<em><a href=\"") % ("" == parentPath ? "/" : parentPath.data()) % "\">&lt;" % tr("parent") % "&gt;</a></em></li>\n";
			}

			const auto addMimeIconToResponseBody = [&responseBody, this](const auto & ext) {
				if(!ext.isEmpty()) {
					for(const auto & mimeType : m_config.mimeTypesForFileExtension(ext)) {
						const auto mimeTypeIcon = mimeIconUri(mimeType);

						if(!mimeTypeIcon.isEmpty()) {
							responseBody += "<img src=\"" % mimeTypeIcon % "\" />&nbsp;";
							return;
						}
					}
				}

				responseBody += "<img src=\"" % mimeIconUri("application-octet-stream") % "\" />&nbsp;";
			};

			/* TODO configuration option to ignore hidden files */
			/* TODO configuration option to order dirs first, then alpha? */
			for(const auto & entry : QDir(resolvedResourcePath).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst)) {
				const auto htmlFileName = html_escape(entry.fileName());
				responseBody += "<li";

				if(entry.isSymLink()) {
					auto targetEntry = entry;

					// TODO detect and bail for circular links
					do {
						targetEntry = QFileInfo(targetEntry.symLinkTarget());
					} while(targetEntry.exists() && targetEntry.isSymLink());

					responseBody += " class=\"symlink\">";

					if(!targetEntry.exists()) {
						responseBody += "<img src=\"" % mimeIconUri("application-octet-stream").toUtf8() % "\" />&nbsp;";
					}
					else if(targetEntry.isDir()) {
						responseBody += "<img src=\"" % mimeIconUri("inode/directory").toUtf8() % "\" />&nbsp;";
					}
					else {
						addMimeIconToResponseBody(targetEntry.suffix());
					}
				}
				else if(entry.isDir()) {
					responseBody += " class=\"directory\"><img src=\"" % mimeIconUri("inode/directory").toUtf8() % "\" />&nbsp;";
				}
				else if(entry.isFile()) {
					responseBody += " class=\"file\">";
					addMimeIconToResponseBody(entry.suffix());
				}
				else {
					responseBody += "><img src=\"" % mimeIconUri("application-octet-stream").toUtf8() + "\" />&nbsp;";
				}

				responseBody += "<a href=\"" % htmlPath % "/" % htmlFileName % "\">" % htmlFileName % "</a></li>\n";
			}

			responseBody += "</ul></div>\n<div id=\"footer\"><p>" % html_escape(qApp->applicationDisplayName()) % QStringLiteral(" v") % html_escape(qApp->applicationVersion()) % "</p></div></body>\n</html>";
			sendHeader("Content-length", QString::number(responseBody.size()));
			sendHeader("Content-MD5", QString(QCryptographicHash::hash(responseBody, QCryptographicHash::Md5).toHex()));

			// TODO don't bother building body if request method is HEAD?
			if(method == "GET" || method == "POST") {
				/// TODO support gzip encoding? will require processing of request headers
				sendBody(responseBody);
			}

			m_stage = ResponseStage::Completed;
			return;
		}

		for(const auto & mimeType : m_config.mimeTypesForFileExtension(resource.suffix())) {
			switch(m_config.mimeTypeAction(mimeType)) {
				case WebServerAction::Ignore:
					// do nothing - just try the next MIME type for the resource
					break;

				case WebServerAction::Serve:
					// TODO forbid serving from cgi-bin
					Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::Serve);

					if(resource.exists() && resource.isFile()) {
						sendResponse(HttpResponseCode::Ok);
						sendDateHeader();
						// TODO charset for text/ content types
						sendHeader("Content-type", mimeType);
						sendHeader("Content-length", QString::number(resource.size()));

						if(method == "GET" || method == "POST") {
							QFile f(resolvedResourcePath);

							if(f.open(QIODevice::ReadOnly)) {
								QByteArray content = f.readAll();
								f.close();
								sendHeader("Content-MD5", QCryptographicHash::hash(content, QCryptographicHash::Md5).toHex());

								/// TODO support gzip encoding? will require processing of request headers
								/// TODO support ssi? - will require a certain amount of parsing of body content
								sendBody(content);
							}
							else {
								std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to open file" << qPrintable(resolvedResourcePath) << "for reading\n";
							}
						}
					}
					else {
						std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: File not found - sending HTTP_NOT_FOUND\n";
						sendError(HttpResponseCode::NotFound);
					}

					m_stage = ResponseStage::Completed;
					return;

				case WebServerAction::CGI: {
					// null means no CGI execution
					if(m_config.cgiBin().isNull()) {
						std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Server not configured for CGI support - sending HTTP_NOT_FOUND\n";
						Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::Forbid);
						sendError(HttpResponseCode::NotFound);
						return;
					}

					/// TODO does this need to check that the resource path exists (i.e. the script is present), or do
					/// we not want to do this because sometimes resources are not literal file paths?
					QFileInfo cgiBin(m_config.cgiBin());

					if(cgiBin.isRelative()) {
						cgiBin = QFileInfo(docRoot.absoluteFilePath() + "/" + cgiBin.filePath());
					}

					QString cgiExecutable = m_config.mimeTypeCgi(mimeType);

					// empty means execute directly; null means do not execute through CGI
					if(!cgiExecutable.isNull()) {
						if(cgiExecutable.isEmpty()) {
							cgiExecutable = resolvedResourcePath;
						}
						else {
							cgiExecutable = QFileInfo(cgiBin.absoluteFilePath() + "/" + cgiExecutable).absoluteFilePath() + " \"" + resolvedResourcePath + "\"";
						}
					}

					// cgiExecutable is now fully-resolved path to executable

					if(cgiExecutable.isNull() || !cgiExecutable.startsWith(cgiBin.absoluteFilePath())) {
						Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::Forbid);
						sendError(HttpResponseCode::Forbidden);
						return;
					}

					QStringList env;

					if(!uriQuery.empty()) {
						env.push_back(QStringLiteral("QUERY_STRING=") + uriQuery.data());
					}

					env.push_back(QStringLiteral("GATEWAY_INTERFACE=CGI/1.1"));
					env.push_back(QStringLiteral("REDIRECT_STATUS=1"));  // non-standard, but since 5.3 is required to make PHP happy
					env.push_back(QStringLiteral("REMOTE_ADDR=") + clientAddr);
					env.push_back(QStringLiteral("REMOTE_PORT=%1").arg(clientPort));
					env.push_back(QStringLiteral("REQUEST_METHOD=") + QString::fromStdString(method));
					env.push_back(QStringLiteral("REQUEST_URI=") + uriPath.data());
					env.push_back(QStringLiteral("SCRIPT_NAME=") + uriPath.data());
					env.push_back(QStringLiteral("SCRIPT_FILENAME=") + resolvedResourcePath);
					// env.push_back(QStringLiteral("SERVER_NAME=") + m_config.listenAddress());
					env.push_back(QStringLiteral("SERVER_ADDR=") + m_config.listenAddress());
					env.push_back(QStringLiteral("SERVER_PORT=") + QString::number(m_config.port()));
					env.push_back(QStringLiteral("DOCUMENT_ROOT=") + docRoot.absoluteFilePath());
					env.push_back(QStringLiteral("SERVER_PROTOCOL=HTTP/") + version.data());
					env.push_back(QStringLiteral("SERVER_SOFTWARE=") + qApp->applicationName());
					env.push_back(QStringLiteral("SERVER_SIGNATURE=EquitWebServerRequestHandler on %1 port %2").arg(m_config.listenAddress()).arg(m_config.port()));
					env.push_back(QStringLiteral("SERVER_ADMIN=") + m_config.adminEmail());

					const auto contentTypeIter = headers.find("content-type");

					if(headers.cend() != contentTypeIter) {
						env.push_back(QStringLiteral("CONTENT_TYPE=") + contentTypeIter->second.data());
						env.push_back(QStringLiteral("CONTENT_LENGTH=%1").arg(body.size()));
					}

					// put the HTTP headers into the CGI environment
					for(const auto & header : headers) {
						env.push_back(QStringLiteral("HTTP_") + QString::fromStdString(header.first).replace('-', '_').toUpper() + "=" + header.second.data());
					}

					QProcess cgiProcess;
					cgiProcess.setEnvironment(env);
					cgiProcess.setWorkingDirectory(QFileInfo(resolvedResourcePath).absolutePath());

					Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::CGI);
					cgiProcess.start(cgiExecutable, QIODevice::ReadWrite);

					if(!cgiProcess.waitForStarted(m_config.cgiTimeout())) {
						std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Timeout waiting for CGI process to start.\n";
						sendError(HttpResponseCode::RequestTimeout);
					}
					else {
						if(!cgiProcess.waitForFinished(m_config.cgiTimeout())) {
							std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Timeout waiting for CGI process to complete.\n";
							sendError(HttpResponseCode::RequestTimeout);
						}
						else {
							cgiProcess.waitForReadyRead();

							if(0 != cgiProcess.exitCode()) {
								std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: CGI process returned error status " << cgiProcess.exitCode() << "\n";
								std::cerr << qPrintable(cgiProcess.readAllStandardError()) << "\n";
							}

							sendResponse(HttpResponseCode::Ok);
							sendDateHeader();

							// TODO read in chunks
							QByteArray data = cgiProcess.readAllStandardOutput();
							// send headers
							int pos = data.indexOf("\r\n\r\n");
							sendData(data.left(pos + 2));

							if(method == "GET" || method == "POST") {
								// send body
								sendData(data.right(data.size() - pos - 2));
							}
						}
					}

					cgiProcess.close();
					m_stage = ResponseStage::Completed;
					return;
				}

				case WebServerAction::Forbid:
					Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::Forbid);
					sendError(HttpResponseCode::Forbidden);
					return;
			}
		}

		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(reqUri), WebServerAction::Forbid);
		sendError(HttpResponseCode::NotFound);
	}


}  // namespace EquitWebServer
