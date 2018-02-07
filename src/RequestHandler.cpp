/** \file RequestHandler.cpp
 * \author darren Hatherley
 * \version 0.9.9
 * \date 19th June, 2012
 *
 * \brief Implementation of the RequestHandler class for EquitWebServer
 *
 * \todo
 * - configuration option to ignore hidden files in directory listing
 * - configuration option to order dirs first, then alpha in directory listing
 * - decide on application license
 *
 * \par Current Changes
 * - (2012-06-22) directory listings no longer need default action to be
 *   "serve". The config item for allowing directory listings effectively
 *   works as if the "directory" mime type was set to "Serve".
 * - (2012-06-19) file documentation created.
 */

#include "RequestHandler.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <regex>

#include <QApplication>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QHostAddress>
#include <QProcess>
#include <QRegExp>
#include <QStringList>
#include <QTcpSocket>
#include <QUrl>

#include "RequestHandlerResponseCodes.h"

#define EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_SYMLINK ""
#define EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_DIRECTORY                  \
	"data:image/"                                                                \
	"png;base64,"                                                                \
	"iVBORw0KGgoAAAANSUhEUgAAABYAAAAWCAYAAADEtGw7AAAAAXNSR0IArs4c6QAAAAZiS0dEAP" \
	"8A/wD/"                                                                     \
	"oL2nkwAAAAlwSFlzAAAJhAAACYQBquJjeQAAAAd0SU1FB9sFARYXMYiH9TcAAALuSURBVDjLxZ" \
	"VPixxVFMV/99brYSDEMIMYhERFRMhCVyJBVxFBMbhL1gbBhTu/"                         \
	"geNX8BMIbnWnrjUQCATcGHES/JdMJqM908k401XT3VP1jov3XlcFdKEg1qKreO++c+4951Q1/"  \
	"F/XpY2v/tU5u/Thl9cWx90rlYHKIhAlwijsf/7RxbV/"                                \
	"Cnp54wvszQ8+"                                                               \
	"1YXX38DU4QXVDHfn66vX5SFRSlBZIrQlhBAggRtEwWgUtj7beOupcNQcspg13N4+"           \
	"QBgOuMTZJ1Z59eUXrY0qGEREjHmyKLoMKokoEMbm5ubZ9z6+"                           \
	"cSrUhxOao4b5rMbNqIAZ8ONWQyuhdDLJk/"                                         \
	"scGXRKBAbIEvHptVWOminbW5MqLOoD6rphcVRTmTFTr/"                               \
	"NsMeOPvW1mdUOHCJY6jEUxel8kcbB+kp3fx3z3jU9Di3Hn15/Z2d3PpWACEWkOH/L+u2/"      \
	"zzNNPLuWwYrIGyPm+Eow2mu7euTcOL50/"                                          \
	"zzuXX6OetwNTShfGb+MpP9ya9J39RbflOVROJ7Ob16+"                                \
	"eCnXT8e3NPRTVH8oaBkv6FVMjEIA2A5YkRMCB4y6y4hXCCMqRYfArpcJWLOcupIulYD2BAV1+"  \
	"nqf4ECyZTheNylJBlQutxMkSUcwak/edNBl5H7FMTwBQTOid+k6M/kAX+zVXevYMXlm/"       \
	"XyVjiFGEmAPeKi5HLKNZliQO1rrhfp6oHUyJDEmExJBOK4e9aFyuYo4GxlbFsEFaygQSBHfDvF" \
	"paUpU3aRCjKt8LWchZXo4/qHNzzIzw/d09fnpwA8upiFkC7NGs9tlOMfu7XBvG/"            \
	"v1dwvqJFU6uP5YtHwAMcmp5wXOu4+Cddj0qlbsTxwvCs2ce54VzzxOjkgE5UqY+"            \
	"mwJGntwX5fOZ92zQcULmF9shTJvj2e54f3UBjCqYd3AC6DwVmsE8JrBCOBK03svQ5glGBh3G5M" \
	"Hh1GDtOc6cu2Iuk3BDLmQoOmBIbmCSPH+FlKywiCHMu0RnMTsgJvc/+c/+K/"               \
	"8EgKCf2dOModEAAAAASUVORK5CYII="
#define EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_FILE                       \
	"data:image/"                                                                \
	"png;base64,"                                                                \
	"iVBORw0KGgoAAAANSUhEUgAAABYAAAAWCAMAAADzapwJAAABqlBMVEUAAAAKCgoKCgoAAAAAAA" \
	"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACGh4e5ubmVlJSZmJidm5uh" \
	"oKCgn5+hoKCAgIDi4ODl4+P9/"                                                  \
	"PwHBwcODg4fHx8iIiIjIyMlJSUnJycpKSktLS0xMTE0NDQ2NjY4ODg9PT0+"                \
	"Pj5MTExNTU1OTk5QUFBRUVFXV1ddXV1eXl5gYGBiYmJkZGRlZWVnZ2dtbW1ubm5xcXFzc3OJiY" \
	"mLi4ucnJyfn5+kpKSsq6uurq6vrq6xsbG1tbW6urq+vr6/v7/"                          \
	"Ew8PGxcXIxsbIx8fJyMjJycnKyMjKycnKysrLysrLy8vMy8vMzMzNzMzNzc3Ozc3Ozs7Pz8/"   \
	"Q0NDR0dHS0tLU1NTV1dXW1tbY2NjZ2dna2trc29ve3t7f3d3f3t7g39/"                   \
	"j4eHj4uLj4+Pk5OTl5OTl5eXm5eXm5ubn5+fo6Ojv7u7w7+/"                           \
	"y8vL09PT19fX19vb29PT29fX29vb39/f49/f5+Pj5+fn6+fn6+vr7+vr7+/v8+/v8/Pz9/Pz9/" \
	"f3+/f3+/v7//v7///9yKtF2AAAAHnRSTlMAFBUfIkZIS0xNTk9QUVNVWFrt7/T19fX29v7+/"   \
	"v51egtzAAABUElEQVR42l3M5VfCYBTH8QcLMcHuxO7GLuxAGQMeh92FCLN1wtizMef9n0XH3NH" \
	"vi/vic37nImQwZf3NZEDxzPA/8zdbYOVl9fV8/"                                     \
	"YJbf197PVs9AYvKjndXhI7RhBZonhIcGjt5N/F+YnlL9oq0SGm8QWgp6GEV/"               \
	"OmRAs5rjUVOIFGZKEoUIiTKaRzf4pDCMiwwgCX/UYIZ2Qfb4IOfq2D4w0dwGF/vw57+JIRZaO/" \
	"sbqN7xse6ThMsCGGJhxZrfZN/dGF+WFv73UHx9sBaV1NrH15a7N/"                       \
	"NVdkLWzKG6tbmqomGPltZforKGBgZk2n7KMigVBanosQTKhDx73SM9AJ3V16UhhL8JnHcM9jsk" \
	"+L9ZYkRaXxF3YQvmYHlKVJRmoF+2f3hIS5oHBziC01IZ1fMzW8+"                        \
	"zszOFaSj38zwePwED3uBUJ4R6SVl56hlJuv4BX3mgqU/G1J9AAAAAElFTkSuQmCC"
#define EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_UNKNOWN \
	EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_FILE

EquitWebServer::RequestHandler::RequestHandler(
  QTcpSocket * socket, const EquitWebServer::Configuration & opts,
  QObject * parent)
: QThread(parent), m_socketDescriptor(0), m_socket(socket), m_config(opts),
  m_stage(Response) {
	Q_ASSERT(m_socket);
	m_socket->moveToThread(this);
}


EquitWebServer::RequestHandler::~RequestHandler(void) {
	if(m_socket)
		disposeSocketObject();
}


void EquitWebServer::RequestHandler::disposeSocketObject(void) {
	if(m_socket) {
		m_socket->disconnectFromHost();
		m_socket->waitForDisconnected();
		delete m_socket;
		m_socket = nullptr;
	}
}


bool EquitWebServer::RequestHandler::sendData(const QByteArray & data) {
	if(m_socket->isWritable()) {
		qint64 bytes;
		int remain = data.size();
		const char * realData = data.data();

		/// TODO might need a timeout in case we continually write 0 bytes
		while(remain) {
			bytes = m_socket->write(realData, remain);

			if(bytes == -1) {
				std::cout << "bpWebServer::bpWebServerRequestHandler::sendData() - "
								 "error writing to TCP socket";
				return false;
			}

			realData += bytes;
			remain -= bytes;
		}

		return true;
	}

	std::cout << "bpWebServer::bpWebServerRequestHandler::sendData() - tcp socket "
					 "is not writable";
	return false;
}


QString EquitWebServer::RequestHandler::getDefaultResponseReason(int n) {
	switch(n) {
		case HTTP_CONTINUE:
			return "Continue";
		case HTTP_SWITCHING_PROTOCOLS:
			return "Switching Protocols";
		case HTTP_OK:
			return "OK";
		case HTTP_CREATED:
			return "Created";
		case HTTP_ACCEPTED:
			return "Accepted";
		case HTTP_NON_AUTHORITATIVE_INFORMATION:
			return "Non-Authoritative Information";
		case HTTP_NO_CONTENT:
			return "No Content";
		case HTTP_RESET_CONTENT:
			return "Reset Content";
		case HTTP_PARTIAL_CONTENT:
			return "Partial Content";
		case HTTP_MULTIPLE_CHOICES:
			return "Multiple Choices";
		case HTTP_MOVED_PERMANENTLY:
			return "Moved Permanently";
		case HTTP_FOUND:
			return "Found";
		case HTTP_SEE_OTHER:
			return "See Other";
		case HTTP_NOT_MODIFIED:
			return "Not Modified";
		case HTTP_USE_PROXY:
			return "Use Proxy";
		case HTTP_UNUSED_306:
			return "(Unused)";
		case HTTP_TEMPORARY_REDIRECT:
			return "Temporary Redirect";
		case HTTP_BAD_REQUEST:
			return "Bad Request";
		case HTTP_UNAUTHORISED:
			return "Unauthorised";
		case HTTP_PAYMENT_REQUIRED:
			return "Payment Required";
		case HTTP_FORBIDDEN:
			return "Forbidden";
		case HTTP_NOT_FOUND:
			return "Not Found";
		case HTTP_METHOD_NOT_ALLOWED:
			return "Method Not Allowed";
		case HTTP_NOT_ACCEPTABLE:
			return "Not Acceptable";
		case HTTP_PROXY_AUTHENTICATION_REQUIRED:
			return "Proxy Authentication Required";
		case HTTP_REQUEST_TIMEOUT:
			return "Request Timeout";
		case HTTP_CONFLICT:
			return "Conflict";
		case HTTP_GONE:
			return "Gone";
		case HTTP_LENGTH_REQUIRED:
			return "Length Required";
		case HTTP_PRECONDITION_FAILED:
			return "Precondition Failed";
		case HTTP_REQUEST_ENTITY_TOO_LARGE:
			return "Request Entity Too Large";
		case HTTP_REQUEST_URI_TOO_LONG:
			return "Request-URI Too Long";
		case HTTP_UNSUPPORTED_MEDIA_TYPE:
			return "Unsupported Media Type";
		case HTTP_REQUESTED_RANGE_NOT_SATISFIABLE:
			return "Requested Range Not Satisfiable";
		case HTTP_EXPECTATION_FAILED:
			return "Expectation Failed";
		case HTTP_INTERNAL_SERVER_ERROR:
			return "Internal Server Error";
		case HTTP_NOT_IMPLEMENTED:
			return "Not Implemented";
		case HTTP_BAD_GATEWAY:
			return "Bad Gateway";
		case HTTP_SERVICE_UNAVAILABLE:
			return "Service Unavailable";
		case HTTP_GATEWAY_TIMEOUT:
			return "Gateway Timeout";
		case HTTP_HTTP_VERSION_NOT_SUPPORTED:
			return "HTTP Version Not Supported";
	}

	return "Unknown";
}


QString EquitWebServer::RequestHandler::getDefaultResponseMessage(int n) {
	switch(n) {
		case 100:
			return "Continue";
		case 101:
			return "Switching Protocols";
		case HTTP_OK:
			return "The request was accepted and will be honoured.";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 203:
			return "Non-Authoritative Information";
		case 204:
			return "No Content";
		case 205:
			return "Reset Content";
		case 206:
			return "Partial Content";
		case 300:
			return "Multiple Choices";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 303:
			return "See Other";
		case 304:
			return "Not Modified";
		case 305:
			return "Use Proxy";
		case 306:
			return "(Unused)";
		case 307:
			return "Temporary Redirect";
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorised";
		case 402:
			return "Payment Required";
		case HTTP_FORBIDDEN:
			return "The request could not be fulfilled because you are not allowed to "
					 "access the resource requested.";
		case HTTP_NOT_FOUND:
			return "The resource requested could not be located on this server.";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 407:
			return "Proxy Authentication Required";
		case HTTP_REQUEST_TIMEOUT:
			return "The request could not be fulfilled because it took too long to "
					 "process. If the server is currently busy, it may be possible to "
					 "successfully fulfil the request later.";
		case 409:
			return "Conflict";
		case 410:
			return "The requested resource has been permanently removed from this "
					 "server.";
		case 411:
			return "Length Required";
		case 412:
			return "Precondition Failed";
		case 413:
			return "Request Entity Too Large";
		case 414:
			return "The request could not be fulfilled because the identifier of the "
					 "resource requested was too long to process.";
		case 415:
			return "Unsupported Media Type";
		case 416:
			return "Requested Range Not Satisfiable";
		case 417:
			return "Expectation Failed";
		case 500:
			return "The request could not be fulfilled because of an unexpected "
					 "internal error in the server.";
		case 501:
			return "The request could not be fulfilled because it is of an unsupported "
					 "type.";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
	}

	return "Unknown response code.";
}


bool EquitWebServer::RequestHandler::sendResponse(int n, const QString & title) {
	if(m_stage == Response) {
		QString responseTitle =
		  (title.isNull()
			  ? EquitWebServer::RequestHandler::getDefaultResponseReason(n)
			  : title);
		QByteArray data = "HTTP/1.1 ";
		data += QString::number(n) + " " + responseTitle + "\r\n";
		return sendData(data);
	}

	std::cout << "bpWebServer::bpWebServerRequestHandler::sendResponse() - cannot "
					 "send response code when headers and/or body already sent.";
	return false;
}


bool EquitWebServer::RequestHandler::sendHeader(const QString & header,
																const QString & value) {
	if(m_stage != Response && m_stage != Headers) {
		std::cout << "bpWebServer::bpWebServerRequestHandler::sendHeader() - cannot "
						 "send header after body content started.";
		return false;
	}

	m_stage = Headers;
	sendData(header.toUtf8() + ": " + value.toUtf8() + "\r\n");
	return true;
}


bool EquitWebServer::RequestHandler::sendDateHeader(const QDateTime & d) {
	QString date = d.toUTC().toString("ddd, d MMM yyyy hh:mm:ss") + " GMT";
	std::cout << "Sending Date header with date" << qPrintable(date) << "\n"
				 << std::flush;
	return sendHeader("Date", date);
}

bool EquitWebServer::RequestHandler::sendBody(const QByteArray & body) {
	if(m_stage == Completed) {
		std::cout << "bpWebServer::bpWebServerRequestHandler::sendHeader() - cannot "
						 "send body after request has been fulfilled.";
		return false;
	}

	if(m_stage != Body) {
		sendData("\r\n");
		m_stage = Body;
	}

	return sendData(body);
}


bool EquitWebServer::RequestHandler::sendError(int n, const QString & msg, const QString & title) {
	if(m_stage != Response) {
		std::cout << "bpWebServer::bpWebServerRequestHandler::sendError() - cannot "
						 "send a complete error response when header or body content "
						 "has already been sent.";
		return false;
	}

	QString realTitle = (title.isEmpty() ? EquitWebServer::RequestHandler::getDefaultResponseReason(n) : title);
	QString realMsg = (msg.isEmpty() ? EquitWebServer::RequestHandler::getDefaultResponseMessage(n) : msg);

	if(sendResponse(n, title) && sendHeader("Content-type", "text/html") &&
		sendDateHeader() &&
		sendBody((QString("<html><head><title>") + realTitle + "</title></head><body><h1>" + QString::number(n) + " " + realTitle + "</h1><p>" + realMsg + "</p></body></html>").toUtf8())) {
		m_stage = Completed;
		return true;
	}

	std::cout << "bpWebServer::bpWebServerRequestHandler::sendError() - sending "
					 "of response, header or body content for error failed.";
	return false;
}


void EquitWebServer::RequestHandler::run(void) {
	Q_ASSERT(m_socket);
	Q_EMIT handlingRequestFrom(m_socket->peerAddress().toString(), m_socket->peerPort());
	std::cout << "bpWebServer::bpWebServerRequestHandler::run() - request from " << qPrintable(m_socket->peerAddress().toString()) << ":" << m_socket->peerPort() << "\n"
				 << std::flush;

	/* check controls on remote IP */
	QString remoteIP = m_socket->peerAddress().toString();
	quint16 remotePort = m_socket->peerPort();
	EquitWebServer::Configuration::ConnectionPolicy policy = m_config.getIPAddressPolicy(remoteIP);
	emit(requestConnectionPolicyDetermined(remoteIP, remotePort, policy));

	switch(policy) {
		case EquitWebServer::Configuration::AcceptConnection:
			Q_EMIT acceptedRequestFrom(remoteIP, remotePort);
			break;

		case EquitWebServer::Configuration::NoConnectionPolicy:
		case EquitWebServer::Configuration::RejectConnection:
			std::cout << "Policy for " << qPrintable(remoteIP)
						 << " is to reject connection.";
			Q_EMIT rejectedRequestFrom(remoteIP, remotePort);
			// need to finish reading from socket here, otherwise client gets occasional
			// broken connections...
			sendError(HTTP_FORBIDDEN);
			m_socket->close();
			disposeSocketObject();
			return;
	}

	QByteArray data;
	int i = -1;

	/* read until we've got all the headers (may read beyond end of headers) */
	while(i == -1) {
		if(m_socket->waitForReadyRead()) {
			data += m_socket->readAll();
			i = data.indexOf("\r\n\r\n");
		}
		else {
			if(m_socket->error() != QAbstractSocket::SocketTimeoutError) {
				// an error, or socket exhausted of data without finding end of headers
				qWarning() << "socket stopped providing data while still expecting more headers";
				qWarning() << "socket error was " << qPrintable(m_socket->errorString());
				sendError(HTTP_BAD_REQUEST);
				m_socket->close();
				// TODO use a scope guard for this
				disposeSocketObject();
				return;
			}
		}
	}

	/* construct ONLY from header data */
	const auto nextHeader = [&data]() -> std::optional<std::string> {
		static int pos = 0;
		static auto len = data.length() - 2;
		static const char * myData = data.data();

		int start = pos;

		while(pos < len) {
			if('\r' == *(myData + pos) && '\n' == *(myData + pos + 1)) {
				break;
			}

			++pos;
		}

		if(pos < len) {
			const std::string ret = data.mid(start, pos - start).data();
			pos += 2;
			return ret;
		}

		return {};
	};

	const auto to_lower = [](const auto & str) -> decltype(auto) {
		typename std::remove_const<typename std::remove_reference<decltype(str)>::type>::type ret;

		std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), [](const auto & ch) {
			return std::tolower(ch);
		});

		return ret;
	};

	auto http = nextHeader();
	std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": request line: \"" << *http << "\"\n";
	std::regex headerRx("^(OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT) ([^ ]+) HTTP/([0-9](?:\\.[0-9]+)*)$");
	std::smatch captures;

	if(!http || !std::regex_match(*http, captures, headerRx)) {
		std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": invalid HTTP request (invalid request line)\n"
					 << std::flush;
		sendError(HTTP_BAD_REQUEST);
		// TODO use a scope guard for this
		disposeSocketObject();
		return;
	}

	std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": request line \"" << *http << "\"\n"
				 << std::flush;

	std::string method = captures[1];
	std::string uri = captures[2];
	std::string version = captures[3];
	std::cout << "parsed to method = \"" << method << "\", URI = \"" << uri << "\", version = \"" << version << "\"\n"
				 << std::flush;

	HttpHeaders headers;
	headerRx = "^([a-zA-Z][a-zA-Z\\-]*) *: *(.+)$";

	while(true) {
		auto headerLine = nextHeader();

		if(headerLine) {
			if("" == *headerLine) {
				// all headers read
				break;
			}

			if(std::regex_match(*headerLine, captures, headerRx)) {
				std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": found request header \"" << captures.str(1) << "\" : \"" << captures.str(2) << "\"\n"
							 << std::flush;
				headers.insert(HttpHeaders::value_type(to_lower(captures.str(1)), captures.str(2)));
				continue;
			}
		}

		std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": invalid HTTP request (invalid header)\n"
					 << std::flush;
		sendError(HTTP_BAD_REQUEST);
		// TODO use a scope guard for this
		disposeSocketObject();
		return;
	}

	/* whatever extra we already read beyond headers is body */
	const auto contentLengthIt = headers.find("content-length");

	if(contentLengthIt == headers.end()) {
		std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": invalid HTTP request (missing content-length header)"
					 << "\n"
					 << std::flush;
		sendError(HTTP_BAD_REQUEST);
		// TODO use a scope guard for this
		disposeSocketObject();
		return;
	}

	const char * contentLengthValue = contentLengthIt->second.data();
	char * end;
	auto contentLength = std::strtoul(contentLengthValue, &end, 10);

	if(end) {
		while(' ' == *end) {
			++end;
		}
	}

	if(!end || 0 != *end) {
		// conversion failure, or extraneous non-whitespace after content-length
		std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": invalid HTTP request (invalid content-length header)\n"
					 << std::flush;
		sendError(HTTP_BAD_REQUEST);
		// TODO use a scope guard for this
		disposeSocketObject();
		return;
	}

	QByteArray body = data.right(data.size() - i - 4);
	long stillToRead = static_cast<long>(contentLength) - body.size();

	/* read remainder of body */
	while(0 < stillToRead) {
		// TODO what happens when client sends too much body?
		if(m_socket->waitForReadyRead()) {
			data = m_socket->readAll();
			body += data;
			stillToRead -= data.size();
		}
		else {
			if(m_socket->error() != QAbstractSocket::SocketTimeoutError) {
				break;
			}
		}
	}

	if(0 < stillToRead) {
		// not enough body data
		qWarning() << "socket stopped providing data while still expecting " << stillToRead << " bytes";
		qWarning() << "socket error was " << qPrintable(m_socket->errorString());
		sendError(HTTP_BAD_REQUEST);
		m_socket->close();
		// TODO use a scope guard for this
		disposeSocketObject();
		return;
	}

	if(0 > stillToRead) {
		// read too much data (does not catch cases when data read from socket
		// hits requirement precisely but socket still has data to read)
		qWarning() << "socket provided more body data than expected (at least "
					  << (-stillToRead) << " bytes)";
	}

	handleHTTPRequest(version, method, uri, headers, body);
	disposeSocketObject();
}


void EquitWebServer::RequestHandler::handleHTTPRequest(const std::string & version, const std::string & method, const std::string & reqUri, const HttpHeaders & headers, const QByteArray & body) {
	std::cout << __PRETTY_FUNCTION__ << "\n";

	if(!m_socket) {
		std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << ": no socket\n";
		return;
	}

	QUrl uri(QString::fromStdString(reqUri));

	// will accept anything up to HTTP/1.1 and process it as HTTP/1.1
	if("1.0" != version && "1.1" != version) {
		std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << ": HTTP version (HTTP/" << version << ") is not supported\n";
		sendError(HTTP_HTTP_VERSION_NOT_SUPPORTED);
		return;
	}

	/* TODO for now we only support GET, HEAD and POST, which covers the only
  *REQUIRED HTTP/1.1 methods (GET, HEAD).
  ** in future we may need to support all HTTP 1.1 methods:
  *OPTIONS,GET,HEAD,POST,PUT,DELETE,TRACE,CONNECT
  */
	if(method != "GET" && method != "HEAD" && method != "POST") {
		std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << ": Request method" << method << "not supported\n";
		sendError(HTTP_NOT_IMPLEMENTED);
		return;
	}

	std::cout << "Request URI:" << reqUri << "\n"
				 << std::flush;
	const auto & md5It = headers.find("content-MD5");

	if(md5It != headers.end() && md5It->second != QCryptographicHash::hash(body, QCryptographicHash::Md5).toHex().constData()) {
		// checksum does not match
		std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << ": calculated MD5 of request body does not match Content-MD5 header";
		std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << ": calculated:" << QCryptographicHash::hash(body, QCryptographicHash::Md5).toHex().constData() << "; header:" << md5It->second;
		// TODO is this the correct response for this error case?
		sendError(HTTP_BAD_REQUEST);
		return;
	}

	QFileInfo docRoot(m_config.getDocumentRoot());
	QFileInfo resource(docRoot.absoluteFilePath() + "/" + uri.toLocalFile());
	QString resolvedResourcePath = resource.absoluteFilePath();

	// only serve request from inside doc root
	if(!resolvedResourcePath.startsWith(docRoot.absoluteFilePath())) {
		std::cout << "Resolved local resource would be outside document root."
					 << "\n"
					 << std::flush;
		std::cout << "Resource     :" << qPrintable(resource.absoluteFilePath()) << "\n"
					 << std::flush;
		std::cout << "Document Root:" << qPrintable(docRoot.absoluteFilePath()) << "\n"
					 << std::flush;
		sendError(HTTP_NOT_FOUND);
		return;
	}

	QString resourceExtension = resource.suffix();
	QVector<QString> mimeTypes =
	  m_config.getMIMETypesForFileExtension(resourceExtension);

	std::cout << "Resolved Local Resource:" << qPrintable(resolvedResourcePath) << "\n"
				 << std::flush;
	std::cout << "Resource Type Extension:" << qPrintable(resourceExtension) << "\n"
				 << std::flush;
	std::cout << "Extension has " << mimeTypes.size()
				 << " associated MIME type(s).";

	// some temporary useful info
	//	if(mimeTypes.size() > 0) {
	//		std::cout << "URI MIME Types: " << "\n" << std::flush;
	//		QVector<QString>::iterator s = mimeTypes.begin();

	//		while(s != mimeTypes.end()) {
	//			std::cout << (*s) << ", " << "\n" << std::flush;
	//			++s;
	//		}

	//		std::cout << "\n" << std::flush;
	//	}

	QVector<QString>::iterator s = mimeTypes.begin();
	bool processed = false;

	if(resource.isDir()) {
		if(m_config.isDirectoryListingAllowed()) {
			sendResponse(HTTP_OK);
			sendDateHeader();
			sendHeader("Content-type", "text/html");
			QFileInfoList entries = QDir(resolvedResourcePath).entryInfoList();

			/* strip trailing "/" from path */
			auto path = reqUri;
			auto pathIt = path.rbegin();
			const auto pathEnd = path.crend();

			while(pathIt != pathEnd && '/' == *pathIt) {
				++pathIt;
			}

			path.erase(pathIt.base(), path.cend());

			QByteArray plainPath = QUrl::fromPercentEncoding(path.data()).toUtf8();
			QByteArray responseBody = "<html>\n<head><title>Directory listing for " + plainPath +
											  "</title><style>body { width: 80%; margin: 0px auto; font-size: "
											  "10pt; color: #444; }\nem { font-style: italic; }\na{ "
											  "text-decoration: none; font-weight: bold; color: #222; }\n a:hover{ "
											  "text-decoration: underline; color: #888; }\n#content ul { display: "
											  "block; list-style-type: none; }\n#content li{ border-top: 1px "
											  "dotted #888; margin: 0em; padding: 0.25em 0em; line-height: 1em; "
											  "}\n#content li:first-child{ border-top: none; }\n#content li:hover "
											  "{ background-color: #f0f0f0; }\n#content li img { vertical-align: "
											  "middle; }#footer { border-top: 1px solid #444; padding: 0.25em 1em; "
											  "} #header { border-bottom: 1px solid #444; padding: 0.25em 1em; "
											  "}</style></head>\n<body>\n<div id=\"header\"><p>Directory listing "
											  "for <em>" +
											  plainPath + "</em></p></div>\n<div id=\"content\"><ul>";

			if("" != path) {
				auto parentPath = path;
				auto pos = parentPath.rfind('/');

				if(pos != decltype(parentPath)::npos) {
					parentPath.erase(pos);
				}

				responseBody += QByteArray("<li><img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_DIRECTORY "\" />&nbsp;<em><a href=\"") + ("" == parentPath ? "/" : parentPath.data()) + "\">&lt;parent&gt;</a></em></li>\n";
			}

			/* TODO configuration option to ignore hidden files */
			/* TODO configuration option to order dirs first, then alpha? */
			for(const auto & entry : entries) {
				QString fileName = entry.fileName();
				if("." == fileName || ".." == fileName)
					continue;

				/* TODO icons */
				responseBody += "<li>";

				if(entry.isSymLink())
					responseBody += "<img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_SYMLINK "\" />&nbsp;";
				else if(entry.isDir())
					responseBody += "<img "
										 "src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_DIRECTORY "\" />&nbsp;";
				else if(entry.isFile())
					responseBody += "<img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_FILE "\" />&nbsp;";
				else
					responseBody += "<img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_UNKNOWN "\" />&nbsp;";

				responseBody += "<a href=\"" + QByteArray(path.data()) + "/" + fileName + "\">" + fileName + "</a></li>\n";
			}

			responseBody += "</ul></div>\n<div id=\"footer\"><p>" + qApp->applicationName() + " v" + qApp->applicationVersion() + "</p></div></body>\n</html>";
			sendHeader("Content-length", QString::number(responseBody.size()));
			sendHeader("Content-MD5", QString(QCryptographicHash::hash(responseBody, QCryptographicHash::Md5).toHex()));

			/* don't send body if request is HEAD */
			if(method == "GET" || method == "POST") {
				/// TODO support gzip encoding? will require processing of request
				/// headers
				sendBody(responseBody);
				std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": finished sending body"
							 << "\n"
							 << std::flush;
			}
		}
		else {
			std::cout << "Directory listings not allowed - sending HTTP_FORBIDDEN"
						 << "\n"
						 << std::flush;
			sendError(HTTP_FORBIDDEN);
		}
	}
	else {
		while(!processed && s != mimeTypes.end()) {
			std::cout << "Checking action for MIME type" << qPrintable(*s) << "\n"
						 << std::flush;

			switch(m_config.getMIMETypeAction(*s)) {
				case EquitWebServer::Configuration::Ignore:
					std::cout << "Action found: Ignore"
								 << "\n"
								 << std::flush;
					// do nothing - just try the next MIME type for the resource
					break;

				case EquitWebServer::Configuration::Serve:
					std::cout << "Action found: Serve"
								 << "\n"
								 << std::flush;
					std::cout << "Serving" << qPrintable(resolvedResourcePath) << "\n"
								 << std::flush;

					/// TODO forbid serving from cgi-bin
					emit(requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), EquitWebServer::Configuration::Serve));

					if(resource.exists() && resource.isFile()) {
						sendResponse(HTTP_OK);
						sendDateHeader();
						sendHeader("Content-type", *s);
						sendHeader("Content-length", QString::number(resource.size()));

						if(method == "GET" || method == "POST") {
							QFile f(resolvedResourcePath);
							if(f.open(QIODevice::ReadOnly)) {
								QByteArray content(f.readAll());
								f.close();
								std::cout << "Sending Content-MD5 header:" << QCryptographicHash::hash(content, QCryptographicHash::Md5).toHex().constData() << "\n"
											 << std::flush;
								sendHeader("Content-MD5", QCryptographicHash::hash(content, QCryptographicHash::Md5).toHex());

								/// TODO support gzip encoding? will require processing of request
								/// headers
								/// TODO support ssi - will require a certain amount of parsing of
								/// body content
								sendBody(content);
								std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << ": finished sending body"
											 << "\n"
											 << std::flush;
							}
							else
								qWarning() << __PRETTY_FUNCTION__ << " " << __LINE__ << ": failed to open file" << resolvedResourcePath << "for reading";
						}
					}
					else {
						std::cout << "File not found - sending HTTP_NOT_FOUND"
									 << "\n"
									 << std::flush;
						sendError(HTTP_NOT_FOUND);
					}

					processed = true;
					break;

				case EquitWebServer::Configuration::CGI:
					std::cout << "Action found: CGI"
								 << "\n"
								 << std::flush;

					// null means no CGI execution
					if(m_config.getCGIBin().isNull()) {
						std::cout << "Server not configured for CGI support - sending HTTP_NOT_FOUND"
									 << "\n"
									 << std::flush;
						emit(requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), EquitWebServer::Configuration::Forbid));
						sendError(HTTP_NOT_FOUND);
					}
					else {
						/// TODO does this need to check that the resource path exists (i.e.
						/// the script is present), or do we not want
						/// to do this because sometimes resources are not literal file paths?
						QFileInfo cgiBin(m_config.getCGIBin());

						if(cgiBin.isRelative()) {
							cgiBin = QFileInfo(docRoot.absoluteFilePath() + "/" + cgiBin.filePath());
						}

						QString cgiExe = m_config.getMIMETypeCGI(*s);

						// empty means execute directly; null means do not execute through CGI
						if(!cgiExe.isNull()) {
							if(cgiExe.isEmpty()) {
								cgiExe = resolvedResourcePath;
							}
							else {
								cgiExe = QFileInfo(cgiBin.absoluteFilePath() + "/" + cgiExe).absoluteFilePath() + " \"" + resolvedResourcePath + "\"";
							}
						}

						// cgiExe is now fully-resolved path to executable
						std::cout << "CGI Command:" << qPrintable(cgiExe) << "\n"
									 << std::flush;

						// check CGI exe exists within cgiBin
						if(cgiExe.isNull() ||
							!cgiExe.startsWith(cgiBin.absoluteFilePath())) {
							emit(requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), EquitWebServer::Configuration::Forbid));
							sendError(HTTP_FORBIDDEN);
						}
						else {
							QStringList env;  // = QProcess::systemEnvironment();

							if(uri.hasQuery()) {
								env << QString("QUERY_STRING=") + uri.query(QUrl::FullyEncoded);
							}

							env << "GATEWAY_INTERFACE=CGI/1.1";
							env << QString("REMOTE_ADDR=") + m_socket->peerAddress().toString();
							env << QString("REMOTE_PORT=%1").arg(m_socket->peerPort());
							env << QString("REQUEST_METHOD=") + QString::fromStdString(method);
							env << QString("REQUEST_URI=") + uri.toLocalFile();
							env << QString("SCRIPT_NAME=") + uri.toLocalFile();
							env << QString("SCRIPT_FILENAME=") + resolvedResourcePath;
							// env << QString("SERVER_NAME=") + m_config.getListenAddress();
							env << QString("SERVER_ADDR=") + m_config.listenAddress();
							env << QString("SERVER_PORT=") + QString::number(m_config.port());
							env << QString("DOCUMENT_ROOT=") + docRoot.absoluteFilePath();
							env << QString("SERVER_PROTOCOL=HTTP/%1").arg(version.data());
							env << "SERVER_SOFTWARE=bpWebServerRequestHandler";
							env << QString("SERVER_SIGNATURE=<address> bpWebServerRequestHandler on %1 port %2</address>").arg(m_config.listenAddress()).arg(m_config.port());
							env << QString("SERVER_ADMIN=%1").arg(m_config.getAdminEmail());

							const auto contentTypeIter = headers.find("content-type");

							if(headers.cend() != contentTypeIter) {
								env << QString("CONTENT_TYPE=%1").arg(contentTypeIter->second.data());
								env << QString("CONTENT_LENGTH=%1").arg(body.size());
							}

							// put the HTTP headers into the CGI environment
							for(const auto & header : headers) {
								env << (QStringLiteral("HTTP_") + QString::fromStdString(header.first).replace('-', '_').toUpper() + "=" + header.second.data());
							}

							QProcess cgi;
							cgi.setEnvironment(env);
							cgi.setWorkingDirectory(
							  QFileInfo(resolvedResourcePath).absolutePath());
							std::cout << "CGI script working directory: " << qPrintable(cgi.workingDirectory());

							// temporarily dump the environment
							//						{
							//							std::cout << "CGI environment
							//follows...";
							//							QStringList::iterator s =
							//env.begin();
							//							while(s != env.end())
							//{
							//								std::cout <<
							//(*s);
							//								++s;
							//							}
							//						}

							emit(requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), EquitWebServer::Configuration::CGI));
							cgi.start(cgiExe, QIODevice::ReadWrite);

							if(!cgi.waitForStarted(m_config.getCGITimeout())) {
								std::cout << "Timeout waiting for CGI process to start."
											 << "\n"
											 << std::flush;
								sendError(HTTP_REQUEST_TIMEOUT);
							}
							else {
								std::cout << "Wrote " << cgi.write(body)
											 << " bytes to CGI process input stream.";

								if(!cgi.waitForFinished(m_config.getCGITimeout())) {
									std::cout << "Timeout waiting for CGI process to complete."
												 << "\n"
												 << std::flush;
									sendError(HTTP_REQUEST_TIMEOUT);
								}
								else {
									cgi.waitForReadyRead();
									sendResponse(HTTP_OK);
									sendDateHeader();
									QByteArray data = cgi.readAllStandardOutput();
									int i = data.indexOf("\r\n\r\n");
									sendData(data.left(i + 2));

									if(method == "GET" || method == "POST")
										sendData(data.right(data.size() - i - 2));
								}
							}

							cgi.close();
						}
					}

					processed = true;
					break;

				case EquitWebServer::Configuration::Forbid:
					std::cout << "Action found: Forbid"
								 << "\n"
								 << std::flush;
					Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), EquitWebServer::Configuration::Forbid);
					sendError(HTTP_FORBIDDEN);
					processed = true;
					break;
			}

			s++; /* check next MIME type for resource */
		}
	}

	if(!processed) {
		std::cout << "Web server is not configured to handle this URI."
					 << "\n"
					 << std::flush;
		Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), EquitWebServer::Configuration::Forbid);
		sendError(HTTP_NOT_FOUND);
	}

	m_stage = Completed;
}
