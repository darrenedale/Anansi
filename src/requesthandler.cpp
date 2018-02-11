/** \file RequestHandler.cpp
 * \author Darren Edale
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

#include "strings.h"
#include "scopeguard.h"


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


namespace EquitWebServer {


	std::string RequestHandler::m_dirListingCss;
	bool RequestHandler::m_staticInitDone = false;


	RequestHandler::RequestHandler(std::unique_ptr<QTcpSocket> socket, const Configuration & opts, QObject * parent)
	: QThread(parent),
	  m_socket(std::move(socket)),
	  m_config(opts),
	  m_stage(Response) {
		if(!m_staticInitDone) {
			staticInitilise();
		}

		Q_ASSERT(m_socket);
		m_socket->moveToThread(this);
	}


	RequestHandler::~RequestHandler() {
		disposeSocketObject();
	}


	void RequestHandler::disposeSocketObject() {
		if(m_socket) {
			m_socket->disconnectFromHost();
			m_socket->waitForDisconnected();
			m_socket.reset(nullptr);
		}
	}


	bool RequestHandler::sendData(const QByteArray & data) {
		if(m_socket->isWritable()) {
			qint64 bytes;
			int remain = data.size();
			const char * realData = data.data();

			/// TODO might need a timeout in case we continually write no data
			while(remain) {
				bytes = m_socket->write(realData, remain);

				if(bytes == -1) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error writing to TCP socket\n";
					return false;
				}

				realData += bytes;
				remain -= bytes;
			}

			return true;
		}

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: tcp socket  is not writable\n";
		return false;
	}


	QString RequestHandler::defaultResponseReason(HttpResponseCode code) {
		switch(code) {
			case HttpResponseCode::Continue:
				return "Continue";
			case HttpResponseCode::SwitchingProtocols:
				return "Switching Protocols";
			case HttpResponseCode::Ok:
				return "OK";
			case HttpResponseCode::Created:
				return "Created";
			case HttpResponseCode::Accepted:
				return "Accepted";
			case HttpResponseCode::NonAuthoritativeInformation:
				return "Non-Authoritative Information";
			case HttpResponseCode::NoContent:
				return "No Content";
			case HttpResponseCode::ResetContent:
				return "Reset Content";
			case HttpResponseCode::PartialContent:
				return "Partial Content";
			case HttpResponseCode::MultipleChoices:
				return "Multiple Choices";
			case HttpResponseCode::MovedPermanently:
				return "Moved Permanently";
			case HttpResponseCode::Found:
				return "Found";
			case HttpResponseCode::SeeOther:
				return "See Other";
			case HttpResponseCode::NotModified:
				return "Not Modified";
			case HttpResponseCode::UseProxy:
				return "Use Proxy";
			case HttpResponseCode::Code306Unused:
				return "(Unused)";
			case HttpResponseCode::TemporaryRedirect:
				return "Temporary Redirect";
			case HttpResponseCode::BadRequest:
				return "Bad Request";
			case HttpResponseCode::Unauthorised:
				return "Unauthorised";
			case HttpResponseCode::PaymentRequired:
				return "Payment Required";
			case HttpResponseCode::Forbidden:
				return "Forbidden";
			case HttpResponseCode::NotFound:
				return "Not Found";
			case HttpResponseCode::MethodNotAllowed:
				return "Method Not Allowed";
			case HttpResponseCode::NotAcceptable:
				return "Not Acceptable";
			case HttpResponseCode::ProxyAuthenticationRequired:
				return "Proxy Authentication Required";
			case HttpResponseCode::RequestTimeout:
				return "Request Timeout";
			case HttpResponseCode::Conflict:
				return "Conflict";
			case HttpResponseCode::Gone:
				return "Gone";
			case HttpResponseCode::LengthRequired:
				return "Length Required";
			case HttpResponseCode::PreconditionFailed:
				return "Precondition Failed";
			case HttpResponseCode::RequestEntityTooLarge:
				return "Request Entity Too Large";
			case HttpResponseCode::RequestUriTooLong:
				return "Request-URI Too Long";
			case HttpResponseCode::UnsupportedMediaType:
				return "Unsupported Media Type";
			case HttpResponseCode::RequestRangeNotSatisfiable:
				return "Requested Range Not Satisfiable";
			case HttpResponseCode::ExpectationFailed:
				return "Expectation Failed";
			case HttpResponseCode::InternalServerError:
				return "Internal Server Error";
			case HttpResponseCode::NotImplemented:
				return "Not Implemented";
			case HttpResponseCode::BadGateway:
				return "Bad Gateway";
			case HttpResponseCode::ServiceUnavailable:
				return "Service Unavailable";
			case HttpResponseCode::GatewayTimeout:
				return "Gateway Timeout";
			case HttpResponseCode::HttpVersionNotSupported:
				return "HTTP Version Not Supported";
		}

		return "Unknown";
	}


	QString RequestHandler::defaultResponseMessage(HttpResponseCode code) {
		switch(code) {
			case HttpResponseCode::Continue:
				return "Continue";
			case HttpResponseCode::SwitchingProtocols:
				return "Switching Protocols";
			case HttpResponseCode::Ok:
				return "The request was accepted and will be honoured.";
			case HttpResponseCode::Created:
				return "Created";
			case HttpResponseCode::Accepted:
				return "Accepted";
			case HttpResponseCode::NonAuthoritativeInformation:
				return "Non-Authoritative Information";
			case HttpResponseCode::NoContent:
				return "No Content";
			case HttpResponseCode::ResetContent:
				return "Reset Content";
			case HttpResponseCode::PartialContent:
				return "Partial Content";
			case HttpResponseCode::MultipleChoices:
				return "Multiple Choices";
			case HttpResponseCode::MovedPermanently:
				return "Moved Permanently";
			case HttpResponseCode::Found:
				return "Found";
			case HttpResponseCode::SeeOther:
				return "See Other";
			case HttpResponseCode::NotModified:
				return "Not Modified";
			case HttpResponseCode::UseProxy:
				return "Use Proxy";
			case HttpResponseCode::Code306Unused:
				return "(Unused)";
			case HttpResponseCode::TemporaryRedirect:
				return "Temporary Redirect";
			case HttpResponseCode::BadRequest:
				return "Bad Request";
			case HttpResponseCode::Unauthorised:
				return "Unauthorised";
			case HttpResponseCode::PaymentRequired:
				return "Payment Required";
			case HttpResponseCode::Forbidden:
				return "The request could not be fulfilled because you are not allowed to access the resource requested.";
			case HttpResponseCode::NotFound:
				return "The resource requested could not be located on this server.";
			case HttpResponseCode::MethodNotAllowed:
				return "Method Not Allowed";
			case HttpResponseCode::NotAcceptable:
				return "Not Acceptable";
			case HttpResponseCode::ProxyAuthenticationRequired:
				return "Proxy Authentication Required";
			case HttpResponseCode::RequestTimeout:
				return "The request could not be fulfilled because it took too long to process. If the server is currently busy, it may be possible to successfully fulfil the request later.";
			case HttpResponseCode::Conflict:
				return "Conflict";
			case HttpResponseCode::Gone:
				return "The requested resource has been permanently removed from this server.";
			case HttpResponseCode::LengthRequired:
				return "Length Required";
			case HttpResponseCode::PreconditionFailed:
				return "Precondition Failed";
			case HttpResponseCode::RequestEntityTooLarge:
				return "Request Entity Too Large";
			case HttpResponseCode::RequestUriTooLong:
				return "The request could not be fulfilled because the identifier of the resource requested was too long to process.";
			case HttpResponseCode::UnsupportedMediaType:
				return "Unsupported Media Type";
			case HttpResponseCode::RequestRangeNotSatisfiable:
				return "Requested Range Not Satisfiable";
			case HttpResponseCode::ExpectationFailed:
				return "Expectation Failed";
			case HttpResponseCode::InternalServerError:
				return "The request could not be fulfilled because of an unexpected internal error in the server.";
			case HttpResponseCode::NotImplemented:
				return "The request could not be fulfilled because it is of an unsupported type.";
			case HttpResponseCode::BadGateway:
				return "Bad Gateway";
			case HttpResponseCode::ServiceUnavailable:
				return "Service Unavailable";
			case HttpResponseCode::GatewayTimeout:
				return "Gateway Timeout";
			case HttpResponseCode::HttpVersionNotSupported:
				return "HTTP Version Not Supported";
		}

		return "Unknown response code.";
	}


	bool RequestHandler::sendResponse(HttpResponseCode code, const QString & title) {
		if(m_stage == Response) {
			QString responseTitle = (title.isNull() ? RequestHandler::defaultResponseReason(code) : title);
			QByteArray data = "HTTP/1.1 ";
			data += QString::number(static_cast<unsigned int>(code)) + " " + responseTitle + "\r\n";
			return sendData(data);
		}

		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: cannot send response code when headers and/or body already sent.\n";
		return false;
	}


	bool RequestHandler::sendHeader(const QString & header, const QString & value) {
		if(m_stage != Response && m_stage != Headers) {
			std::cerr << __PRETTY_FUNCTION__ << ": cannot send header after body content started.\n";
			return false;
		}

		m_stage = Headers;
		sendData(header.toUtf8() + ": " + value.toUtf8() + "\r\n");
		return true;
	}


	bool RequestHandler::sendDateHeader(const QDateTime & d) {
		return sendHeader("Date", d.toUTC().toString("ddd, d MMM yyyy hh:mm:ss") + " GMT");
	}


	bool RequestHandler::sendBody(const QByteArray & body) {
		if(m_stage == Completed) {
			std::cerr << __PRETTY_FUNCTION__ << ": cannot send body after request has been fulfilled.\n";
			return false;
		}

		if(m_stage != Body) {
			sendData("\r\n");
			m_stage = Body;
		}

		return sendData(body);
	}


	bool RequestHandler::sendError(HttpResponseCode code, const QString & msg, const QString & title) {
		if(m_stage != Response) {
			std::cerr << __PRETTY_FUNCTION__ << ": cannot send a complete error response when header or body content has already been sent.\n";
			return false;
		}

		QString realTitle = (title.isEmpty() ? RequestHandler::defaultResponseReason(code) : title);
		QString realMsg = (msg.isEmpty() ? RequestHandler::defaultResponseMessage(code) : msg);

		if(sendResponse(code, title) && sendHeader("Content-type", "text/html") &&
			sendDateHeader() &&
			sendBody((QString("<html><head><title>") + realTitle + "</title></head><body><h1>" + QString::number(static_cast<unsigned int>(code)) + " " + realTitle + "</h1><p>" + realMsg + "</p></body></html>").toUtf8())) {
			m_stage = Completed;
			return true;
		}

		std::cout << __PRETTY_FUNCTION__ << ": sending of response, header or body content for error failed.\n";
		return false;
	}


	void RequestHandler::staticInitilise() {
		QFile cssFile(QStringLiteral(":/stylesheets/directory-listing"));

		if(!cssFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read built-in directory listing stylesheet (couldn't open resource file)\n";
			return;
		}

		while(!cssFile.atEnd()) {
			m_dirListingCss += cssFile.readAll().constData();
		}

		// TODO dir listing icons from resources

		m_staticInitDone = true;
	}


	void RequestHandler::run(void) {
		Q_ASSERT(m_socket);

		// scope guard automatically does all cleanup on all exit paths
		auto cleanup = Equit::ScopeGuard{[this](void) {
			m_socket->close();
			disposeSocketObject();
		}};

		Q_EMIT handlingRequestFrom(m_socket->peerAddress().toString(), m_socket->peerPort());

		/* check controls on remote IP */
		QString remoteIP = m_socket->peerAddress().toString();
		quint16 remotePort = m_socket->peerPort();
		Configuration::ConnectionPolicy policy = m_config.ipAddressPolicy(remoteIP);
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: emitting RequestHandler::requestConnectionPolicyDetermined()\n";
		Q_EMIT requestConnectionPolicyDetermined(remoteIP, remotePort, policy);

		switch(policy) {
			case Configuration::AcceptConnection:
				Q_EMIT acceptedRequestFrom(remoteIP, remotePort);
				break;

			case Configuration::NoConnectionPolicy:
			case Configuration::RejectConnection:
				std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Policy for " << qPrintable(remoteIP) << " is to reject connection.\n";
				Q_EMIT rejectedRequestFrom(remoteIP, remotePort, tr("Policy for this IP address is Reject"));
				// need to finish reading from socket here, otherwise client gets occasional
				// broken connections...
				sendError(HttpResponseCode::Forbidden);
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
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket stopped providing data while still expecting more headers\n";
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket error was " << qPrintable(m_socket->errorString()) << "\n";
					sendError(HttpResponseCode::BadRequest);
					return;
				}
			}
		}

		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: received data:\n" << qPrintable(data) << "\n";
		int pos = 0;

		const auto nextHeaderLine = [&data, myData = data.data(), &pos, len = data.length() - 2]()->std::optional<std::string> {
			int start = pos;

			while(pos < len) {
				if('\r' == *(myData + pos) && '\n' == *(myData + pos + 1)) {
					break;
				}

				++pos;
			}

			if(pos <= len) {
				const std::string ret = data.mid(start, pos - start).data();
				pos += 2;
				return ret;
			}

			return {};
		};

		auto http = nextHeaderLine();
		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: request line: \"" << *http << "\"\n";
		std::regex headerRx("^(OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT) ([^ ]+) HTTP/([0-9](?:\\.[0-9]+)*)$");
		std::smatch captures;

		if(!http || !std::regex_match(*http, captures, headerRx)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid request line)\n"
						 << std::flush;
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		std::string method = captures[1];
		std::string uri = captures[2];
		std::string version = captures[3];
		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: parsed to method = \"" << method << "\", URI = \"" << uri << "\", version = \"" << version << "\"\n"
		//					 << std::flush;

		HttpHeaders headers;
		headerRx = "^([a-zA-Z][a-zA-Z\\-]*) *: *(.+)$";

		while(true) {
			auto headerLine = nextHeaderLine();

			if(headerLine) {
				if(headerLine->empty()) {
					// all headers read
					break;
				}

				if(std::regex_match(*headerLine, captures, headerRx)) {
					//					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: found request header \"" << captures.str(1) << "\" : \"" << captures.str(2) << "\"\n"
					//								 << std::flush;
					headers.emplace(to_lower(captures.str(1)), captures.str(2));
					continue;
				}
			}

			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid header)\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		/* whatever extra we already read beyond headers is body */
		const auto contentLengthIt = headers.find("content-length");
		auto contentLength = 0UL;

		if(contentLengthIt != headers.end()) {
			const char * contentLengthValue = contentLengthIt->second.data();
			char * end;
			contentLength = std::strtoul(contentLengthValue, &end, 10);

			if(end) {
				while(' ' == *end) {
					++end;
				}
			}

			if(!end || 0 != *end) {
				// conversion failure, or extraneous non-whitespace after content-length
				std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid content-length header)\n"
							 << std::flush;
				sendError(HttpResponseCode::BadRequest);
				return;
			}
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
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket stopped providing data while still expecting " << stillToRead << " bytes";
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket error was " << qPrintable(m_socket->errorString());
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		if(0 > stillToRead) {
			// read too much data (does not catch cases when data read from socket
			// hits requirement precisely but socket still has data to read)
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket provided more body data than expected (at least " << (-stillToRead) << " bytes)\n";
		}

		handleHttpRequest(version, method, uri, headers, body);
	}


	void RequestHandler::handleHttpRequest(const std::string & version, const std::string & method, const std::string & reqUri, const HttpHeaders & headers, const QByteArray & body) {
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

		auto uri = QUrl::fromLocalFile(QString::fromStdString(uriPath));
		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Request URI:" << reqUri << "\n"
		//					 << std::flush;
		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Request URI path:" << uriPath << "\n"
		//					 << std::flush;
		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Request URI query:" << uriQuery << "\n"
		//					 << std::flush;
		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Request URI fragment:" << uriFragment << "\n"
		//					 << std::flush;
		const auto & md5It = headers.find("content-md5");

		if(md5It != headers.end() && md5It->second != QCryptographicHash::hash(body, QCryptographicHash::Md5).toHex().constData()) {
			// checksum does not match
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: calculated MD5 of request body does not match Content-MD5 header\n";
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: calculated:" << QCryptographicHash::hash(body, QCryptographicHash::Md5).toHex().constData() << "; header:" << md5It->second << "\n";
			// TODO is this the correct response for this error case?
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		QFileInfo docRoot(m_config.documentRoot());
		QFileInfo resource(docRoot.absoluteFilePath() + "/" + uri.toLocalFile());
		QString resolvedResourcePath = resource.absoluteFilePath();

		// only serve request from inside doc root
		if(!resolvedResourcePath.startsWith(docRoot.absoluteFilePath())) {
			//			std::cout << __PRETTY_FUNCTION__ << "Resolved local resource would be outside document root.\n"
			//						 << std::flush;
			//			std::cout << __PRETTY_FUNCTION__ << "Resource     :" << qPrintable(resource.absoluteFilePath()) << "\n"
			//						 << std::flush;
			//			std::cout << __PRETTY_FUNCTION__ << "Document Root:" << qPrintable(docRoot.absoluteFilePath()) << "\n"
			//						 << std::flush;
			sendError(HttpResponseCode::NotFound);
			return;
		}

		QString resourceExtension = resource.suffix();
		QVector<QString> mimeTypes = m_config.mimeTypesForFileExtension(resourceExtension);

		//		std::cout << __PRETTY_FUNCTION__ << "Resolved Local Resource:" << qPrintable(resolvedResourcePath) << "\n"
		//					 << std::flush;
		//		std::cout << __PRETTY_FUNCTION__ << "Resource Type Extension:" << qPrintable(resourceExtension) << "\n"
		//					 << std::flush;
		//		std::cout << __PRETTY_FUNCTION__ << "Extension has " << mimeTypes.size() << " associated MIME type(s).\n";


		if(resource.isDir()) {
			if(!m_config.isDirectoryListingAllowed()) {
				sendError(HttpResponseCode::Forbidden);
				Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::Forbid);
				return;
			}

			Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::Serve);
			sendResponse(HttpResponseCode::Ok);
			sendDateHeader();
			sendHeader("Content-type", "text/html");

			/* strip trailing "/" from path */
			auto path = reqUri;
			auto pathIt = path.rbegin();
			const auto pathEnd = path.crend();

			while(pathIt != pathEnd && '/' == *pathIt) {
				++pathIt;
			}

			path.erase(pathIt.base(), path.cend());

			QByteArray plainPath = QUrl::fromPercentEncoding(path.data()).toUtf8();
			QByteArray responseBody = "<html>\n<head><title>Directory listing for " + plainPath + "</title><style>" + m_dirListingCss.c_str() + "</style></head>\n<body>\n<div id=\"header\"><p>Directory listing for <em>" + plainPath + "/</em></p></div>\n<div id=\"content\"><ul>";

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
			for(const auto & entry : QDir(resolvedResourcePath).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst)) {
				QString fileName = entry.fileName();

				responseBody += "<li>";

				if(entry.isSymLink()) {
					responseBody += "<img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_SYMLINK "\" />&nbsp;";
				}
				else if(entry.isDir()) {
					responseBody += "<img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_DIRECTORY "\" />&nbsp;";
				}
				else if(entry.isFile()) {
					responseBody += "<img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_FILE "\" />&nbsp;";
				}
				else {
					responseBody += "<img src=\"" EQUITWEBSERVER_REQUESTHANDLER_DIRLISTINGICON_UNKNOWN "\" />&nbsp;";
				}

				/* TODO MIME icons */
				responseBody += "<a href=\"" + QByteArray(path.data()) + "/" + fileName + "\">" + fileName + "</a></li>\n";
			}

			responseBody += "</ul></div>\n<div id=\"footer\"><p>" + qApp->applicationDisplayName() + " v" + qApp->applicationVersion() + "</p></div></body>\n</html>";
			sendHeader("Content-length", QString::number(responseBody.size()));
			sendHeader("Content-MD5", QString(QCryptographicHash::hash(responseBody, QCryptographicHash::Md5).toHex()));

			/* don't send body if request is HEAD */
			if(method == "GET" || method == "POST") {
				/// TODO support gzip encoding? will require processing of request
				/// headers
				sendBody(responseBody);
			}

			m_stage = Completed;
			return;
		}


		const auto mimeTypesEnd = mimeTypes.cend();

		for(auto mimeType = mimeTypes.begin(); mimeType != mimeTypesEnd; ++mimeType) {
			switch(m_config.mimeTypeAction(*mimeType)) {
				case Configuration::Ignore:
					// do nothing - just try the next MIME type for the resource
					break;

				case Configuration::Serve:
					// TODO forbid serving from cgi-bin
					Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::Serve);

					if(resource.exists() && resource.isFile()) {
						sendResponse(HttpResponseCode::Ok);
						sendDateHeader();
						sendHeader("Content-type", *mimeType);
						sendHeader("Content-length", QString::number(resource.size()));

						if(method == "GET" || method == "POST") {
							QFile f(resolvedResourcePath);
							if(f.open(QIODevice::ReadOnly)) {
								QByteArray content = f.readAll();
								f.close();
								sendHeader("Content-MD5", QCryptographicHash::hash(content, QCryptographicHash::Md5).toHex());

								/// TODO support gzip encoding? will require processing of request
								/// headers
								/// TODO support ssi? - will require a certain amount of parsing of
								/// body content
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

					m_stage = Completed;
					return;

				case Configuration::CGI: {
					// null means no CGI execution
					if(m_config.cgiBin().isNull()) {
						std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Server not configured for CGI support - sending HTTP_NOT_FOUND\n";
						Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::Forbid);
						sendError(HttpResponseCode::NotFound);
						return;
					}

					/// TODO does this need to check that the resource path exists (i.e.
					/// the script is present), or do we not want
					/// to do this because sometimes resources are not literal file paths?
					QFileInfo cgiBin(m_config.cgiBin());

					if(cgiBin.isRelative()) {
						cgiBin = QFileInfo(docRoot.absoluteFilePath() + "/" + cgiBin.filePath());
					}

					QString cgiExe = m_config.mimeTypeCGI(*mimeType);

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

					// check CGI exe exists within cgiBin
					if(cgiExe.isNull() || !cgiExe.startsWith(cgiBin.absoluteFilePath())) {
						Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::Forbid);
						sendError(HttpResponseCode::Forbidden);
						return;
					}

					QStringList env;  // = QProcess::systemEnvironment();

					if(!uriQuery.empty()) {
						env << QStringLiteral("QUERY_STRING=") + QString::fromStdString(uriQuery);
					}

					env << QStringLiteral("GATEWAY_INTERFACE=CGI/1.1");
					env << QStringLiteral("REDIRECT_STATUS=1");  // non-standard, but since 5.3 is required to make PHP happy
					env << QStringLiteral("REMOTE_ADDR=") + m_socket->peerAddress().toString();
					env << QStringLiteral("REMOTE_PORT=%1").arg(m_socket->peerPort());
					env << QStringLiteral("REQUEST_METHOD=") + QString::fromStdString(method);
					env << QStringLiteral("REQUEST_URI=") + uri.toLocalFile();
					env << QStringLiteral("SCRIPT_NAME=") + uri.toLocalFile();
					env << QStringLiteral("SCRIPT_FILENAME=") + resolvedResourcePath;
					// env << QStringLiteral("SERVER_NAME=") + m_config.listenAddress();
					env << QStringLiteral("SERVER_ADDR=") + m_config.listenAddress();
					env << QStringLiteral("SERVER_PORT=") + QString::number(m_config.port());
					env << QStringLiteral("DOCUMENT_ROOT=") + docRoot.absoluteFilePath();
					env << QStringLiteral("SERVER_PROTOCOL=HTTP/%1").arg(version.data());
					env << QStringLiteral("SERVER_SOFTWARE=%1").arg(qApp->applicationName());
					env << QStringLiteral("SERVER_SIGNATURE=EquitWebServerRequestHandler on %1 port %2").arg(m_config.listenAddress()).arg(m_config.port());
					env << QStringLiteral("SERVER_ADMIN=%1").arg(m_config.adminEmail());

					const auto contentTypeIter = headers.find("content-type");

					if(headers.cend() != contentTypeIter) {
						env << QStringLiteral("CONTENT_TYPE=%1").arg(contentTypeIter->second.data());
						env << QStringLiteral("CONTENT_LENGTH=%1").arg(body.size());
					}

					// put the HTTP headers into the CGI environment
					for(const auto & header : headers) {
						env << (QStringLiteral("HTTP_") + QString::fromStdString(header.first).replace('-', '_').toUpper() + "=" + header.second.data());
					}

					QProcess cgi;
					cgi.setEnvironment(env);
					cgi.setWorkingDirectory(QFileInfo(resolvedResourcePath).absolutePath());

					Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::CGI);
					cgi.start(cgiExe, QIODevice::ReadWrite);

					if(!cgi.waitForStarted(m_config.cgiTimeout())) {
						std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Timeout waiting for CGI process to start.\n";
						sendError(HttpResponseCode::RequestTimeout);
					}
					else {
						//						std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Wrote " << cgi.write(body) << " bytes to CGI process input stream.\n";

						if(!cgi.waitForFinished(m_config.cgiTimeout())) {
							std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Timeout waiting for CGI process to complete.\n";
							sendError(HttpResponseCode::RequestTimeout);
						}
						else {
							cgi.waitForReadyRead();

							if(0 != cgi.exitCode()) {
								std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: CGI process returned error status " << cgi.exitCode() << "\n";
								std::cerr << qPrintable(cgi.readAllStandardError()) << "\n";
							}

							sendResponse(HttpResponseCode::Ok);
							sendDateHeader();

							QByteArray data = cgi.readAllStandardOutput();
							// send headers
							int pos = data.indexOf("\r\n\r\n");
							sendData(data.left(pos + 2));

							if(method == "GET" || method == "POST") {
								// send body
								sendData(data.right(data.size() - pos - 2));
							}
						}
					}

					cgi.close();
					m_stage = Completed;
					return;
				}

				case Configuration::Forbid:
					Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::Forbid);
					sendError(HttpResponseCode::Forbidden);
					return;
			}
		}

		Q_EMIT requestActionTaken(m_socket->peerAddress().toString(), m_socket->peerPort(), QString::fromStdString(reqUri), Configuration::Forbid);
		sendError(HttpResponseCode::NotFound);
	}


}  // namespace EquitWebServer
