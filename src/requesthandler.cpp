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
#include <QStringBuilder>
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
	  m_stage(ResponseStage::SendingResponse) {
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
		if(!m_socket->isWritable()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: tcp socket  is not writable\n";
			return false;
		}

		qint64 bytes;
		int remain = data.size();
		const char * realData = data.data();

		/// TODO might need a timeout in case we continually write no data
		while(remain) {
			bytes = m_socket->write(realData, remain);

			if(bytes == -1) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error writing to TCP socket (\"" << qPrintable(m_socket->errorString()) << "\")\n";
				return false;
			}

			realData += bytes;
			remain -= bytes;
		}

		return true;
	}


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


	bool RequestHandler::sendResponse(HttpResponseCode code, const QString & title) {
		if(ResponseStage::SendingResponse != m_stage) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: cannot send response code when headers and/or body already sent.\n";
			return false;
		}

		QString responseTitle = (title.isNull() ? RequestHandler::defaultResponseReason(code) : title);
		QByteArray data = "HTTP/1.1 ";
		data += QString::number(static_cast<unsigned int>(code)) + " " + responseTitle + "\r\n";
		return sendData(data);
	}


	bool RequestHandler::sendHeader(const QString & header, const QString & value) {
		if(ResponseStage::SendingResponse != m_stage && ResponseStage::SendingHeaders != m_stage) {
			std::cerr << __PRETTY_FUNCTION__ << ": cannot send header after body content started.\n";
			return false;
		}

		m_stage = ResponseStage::SendingHeaders;
		sendData(header.toUtf8() + ": " + value.toUtf8() + "\r\n");
		return true;
	}


	bool RequestHandler::sendDateHeader(const QDateTime & d) {
		return sendHeader(QStringLiteral("Date"), d.toUTC().toString(QStringLiteral("ddd, d MMM yyyy hh:mm:ss")) % QStringLiteral(" GMT"));
	}


	bool RequestHandler::sendBody(const QByteArray & body) {
		if(m_stage == ResponseStage::Completed) {
			std::cerr << __PRETTY_FUNCTION__ << ": cannot send body after request has been fulfilled.\n";
			return false;
		}

		if(m_stage != ResponseStage::SendingBody) {
			sendData("\r\n");
			m_stage = ResponseStage::SendingBody;
		}

		return sendData(body);
	}


	bool RequestHandler::sendError(HttpResponseCode code, const QString & msg, const QString & title) {
		if(ResponseStage::SendingResponse != m_stage) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: cannot send a complete error response when header or body content has already been sent.\n";
			return false;
		}

		QString realTitle = (title.isEmpty() ? RequestHandler::defaultResponseReason(code) : title);
		QString realMsg = (msg.isEmpty() ? RequestHandler::defaultResponseMessage(code) : msg);

		if(sendResponse(code, title) && sendHeader("Content-type", "text/html") &&
			sendDateHeader() &&
			sendBody((QStringLiteral("<html><head><title>") % realTitle % QStringLiteral("</title></head><body><h1>") % QString::number(static_cast<unsigned int>(code)) % QStringLiteral(" ") % realTitle % QStringLiteral("</h1><p>") % realMsg % QStringLiteral("</p></body></html>")).toUtf8())) {
			m_stage = ResponseStage::Completed;
			return true;
		}

		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of response, header or body content for error failed.\n";
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
		QString clientAddress = m_socket->peerAddress().toString();
		quint16 clientPort = m_socket->peerPort();
		Configuration::ConnectionPolicy policy = m_config.ipAddressPolicy(clientAddress);
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: emitting RequestHandler::requestConnectionPolicyDetermined()\n";
		Q_EMIT requestConnectionPolicyDetermined(clientAddress, clientPort, policy);

		switch(policy) {
			case Configuration::AcceptConnection:
				Q_EMIT acceptedRequestFrom(clientAddress, clientPort);
				break;

			case Configuration::NoConnectionPolicy:
			case Configuration::RejectConnection:
				std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Policy for " << qPrintable(clientAddress) << " is to reject connection.\n";
				Q_EMIT rejectedRequestFrom(clientAddress, clientPort, tr("Policy for this IP address is Reject"));

				// finish reading from socket otherwise client gets occasional broken connections...
				while(!m_socket->atEnd()) {
					m_socket->read(100);
				}

				sendError(HttpResponseCode::Forbidden);
				return;
		}

		QByteArray data;
		int endOfHeader = -1;

		const auto nextHeaderLine = [this]() -> std::optional<std::string> {
			static std::array<char, 100> buffer;
			std::string data;

			do {
				if(!m_socket->waitForReadyRead()) {
					if(m_socket->error() != QAbstractSocket::SocketTimeoutError) {
						return {};
					}

					// TODO how many consecutive timeout errors before we abort?
					continue;
				}

				auto length = m_socket->readLine(buffer.data(), buffer.size());
				data += buffer.data();

				if(-1 != length) {
					// process a header line
					auto headerLength = data.size();

					if(2 > headerLength || '\r' != data[headerLength - 2] || '\n' != data[headerLength - 1]) {
						return {};
					}

					data[headerLength - 2] = 0;
					return data;
				}
			} while(true);
		};

		auto requestLine = nextHeaderLine();

		if(!requestLine) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid request line)\n"
						 << std::flush;
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: request line: \"" << *requestLine << "\"\n";
		std::regex headerRx("^(OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT) ([^ ]+) HTTP/([0-9](?:\\.[0-9]+)*)$");
		std::smatch captures;

		if(!std::regex_match(*requestLine, captures, headerRx)) {
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

		// TODO code from here has not yet been adapted to the fact that we no longer read all the socket
		// data before parsing

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

		QByteArray body = data.right(data.size() - endOfHeader - 4);
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
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket stopped providing data while still expecting " << stillToRead << " bytes (\"" << qPrintable(m_socket->errorString()) << "\")\n";
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

			m_stage = ResponseStage::Completed;
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

					m_stage = ResponseStage::Completed;
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
					m_stage = ResponseStage::Completed;
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
