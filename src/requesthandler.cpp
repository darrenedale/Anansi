/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file requesthandler.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date Marh 2018
///
/// \brief Implementation of the RequestHandler class for Anansi.
///
/// \dep
/// - <cctype>
/// - <cstdlib>
/// - <iostream>
/// - <optional>
/// - <set>
/// - <regex>
/// - <QApplication>
/// - <QByteArray>
/// - <QBuffer>
/// - <QCryptographicHash>
/// - <QDir>
/// - <QFile>
/// - <QFileInfo>
/// - <QHostAddress>
/// - <QProcess>
/// - <QStringList>
/// - <QStringBuilder>
/// - <QTcpSocket>
/// - <QUrl>
/// - types.h
/// - server.h
/// - strings.h
/// - scopeguard.h
/// - mimeicons.h
/// - deflatecontentencoder.h
/// - gzipcontentencoder.h
/// - identitycontentencoder.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "requesthandler.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <regex>

#include <QApplication>
#include <QByteArray>
#include <QBuffer>
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

#include "types.h"
#include "server.h"
#include "strings.h"
#include "scopeguard.h"
#include "mimeicons.h"
#include "deflatecontentencoder.h"
#include "gzipcontentencoder.h"
#include "identitycontentencoder.h"
#include "qtmetatypes.h"


namespace Anansi {


	/// \class RequestHandler
	///
	/// RequestHandler objects are *single-use only*. Once run() has returned,
	/// the handler can no longer be used.


	static constexpr const int MaxReadErrorCount = 3;
	static constexpr const unsigned int ReadBufferSize = 1024;
	static const QByteArray EOL = QByteArrayLiteral("\r\n");

	static const std::unordered_map<std::string, ContentEncoding> SupportedEncodings = {
	  {"deflate", ContentEncoding::Deflate},
	  {"gzip", ContentEncoding::Gzip},
	  {"identity", ContentEncoding::Identity},
	};


	static const std::string dirListingCss = ([]() -> std::string {
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


	template<class StringType = std::string>
	static std::optional<HttpMethod> parseHttpMethod(StringType str) {
		str = to_lower(str);

		if("options" == str) {
			return HttpMethod::Options;
		}

		if("get" == str) {
			return HttpMethod::Get;
		}

		if("head" == str) {
			return HttpMethod::Head;
		}

		if("post" == str) {
			return HttpMethod::Post;
		}

		if("put" == str) {
			return HttpMethod::Put;
		}

		if("delete" == str) {
			return HttpMethod::Delete;
		}

		if("trace" == str) {
			return HttpMethod::Trace;
		}

		if("connect" == str) {
			return HttpMethod::Connect;
		}

		return {};
	}


	static std::optional<std::string> readHeaderLine(QIODevice & in) {
		std::array<char, ReadBufferSize> readBuffer;
		std::string line;
		int consecutiveReadErrorCount = 0;

		while(!in.canReadLine()) {
			if(!in.waitForReadyRead(3000)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error reading header line (\"" << qPrintable(in.errorString()) << "\"\n";
				++consecutiveReadErrorCount;

				if(MaxReadErrorCount < consecutiveReadErrorCount) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: too many errors attempting to read header line\n";
					return {};
				}
			}
			else {
				consecutiveReadErrorCount = 0;
			}
		}

		int64_t length = 0;

		do {
			// -1 should never happen because canReadLine is known to be true
			length += in.readLine(&readBuffer[0], readBuffer.size());
			line.append(readBuffer.data());
		} while(0 == length || '\n' != line.back());

		// no need to check for trailing \n - we know it's there because it's a read loop exit condition
		if(2 > length || '\r' != line[static_cast<std::string::size_type>(length) - 2]) {
			return {};
		}

		// trim off trailing \r\n
		const auto end = line.cend();
		line.erase(end - 2, end);
		return line;
	}


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
	 * of the request handler's lifetime.
	 * \note If you create subclasses you MUST call this constructor in your derived
	 * class constructors otherwise the socket may not be properly initialised to work
	 * in your handler.
	 * \note If you create subclasses of Server you MUST ensure that the spawned
	 * handler threads receive sockets in the appropriate state.
	 */
	RequestHandler::RequestHandler(std::unique_ptr<QTcpSocket> socket, const Configuration & opts, QObject * parent)
	: QThread(parent),
	  m_socket(std::move(socket)),
	  m_config(opts),
	  m_stage(ResponseStage::SendingResponse),
	  m_responseEncoding(ContentEncoding::Identity),
	  m_encoder(nullptr) {
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

				if(QAbstractSocket::ConnectedState == m_socket->state() && !m_socket->waitForDisconnected()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error disconnecting socket (" << qPrintable(m_socket->errorString()) << ")\n";
				}
			}

			m_socket.reset(nullptr);
		}
	}


	bool RequestHandler::determineResponseEncoding() {
		//#warning Compiling RequestHandler with forced response content encoding for debug purposes
		//		m_responseEncoding = ContentEncoding::Deflate;
		//		return true;

		const auto acceptEncodingHeaderIt = m_requestHeaders.find("accept-encoding");

		// if no accept-encoding header, leave output encoding as it is (default is Identity)
		if(m_requestHeaders.cend() == acceptEncodingHeaderIt) {
			return true;
		}

		// NEXTRELEASE this doesn't ensure that there isn't nonsense between encodings
		const auto & acceptEncodingHeaderValue = acceptEncodingHeaderIt->second;
		static const auto acceptEncodingRx = std::regex("(?:^|,) *([a-z]+)(?:; *q *= *(0(?:\\.[0-9]{1,3})|1(?:\\.0{1,3})))?");
		const auto begin = std::sregex_iterator(acceptEncodingHeaderValue.begin(), acceptEncodingHeaderValue.end(), acceptEncodingRx);
		static const std::sregex_iterator end = {};

		struct AcceptEncodingEntry {
			std::string name;
			uint32_t qValue;  // really qValue * 1000
		};

		// build list containing supported encodings and their q-values,
		std::vector<AcceptEncodingEntry> acceptEncodingEntries;

		for(auto it = begin; it != end; ++it) {
			auto & match = *it;
			uint32_t qValue = 1;  // by default, the lowest

			if(2 < match.size() && 0 < match[2].length()) {
				// rx match guarantees it's between 0 and 1 and has at most 3dp
				// (i.e. this cannot fail)
				qValue = static_cast<uint16_t>(1000 * std::stof(match[2].str()));
			}

			acceptEncodingEntries.push_back({to_lower(match[1].str()), qValue});
		}

		// sort according to q-value and then set the response encoding to
		// the supported encoding with the highest q-value
		std::stable_sort(acceptEncodingEntries.begin(), acceptEncodingEntries.end(), [](const auto & firstEncoding, const auto & secondEncoding) {
			// we wan't higest qValue first
			return firstEncoding.qValue > secondEncoding.qValue;
		});

		//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: acceptable encodings in priority order:\n";

		//		for(const auto & encoding : acceptEncodingEntries) {
		//			std::cout << static_cast<float>(encoding.qValue / 1000) << " " << encoding.name << "\n";
		//		}

		//		std::cout << std::flush;

		// set the response encoding to the supported encoding with the highest q-value
		bool canFallBackOnIdentityEncoding = true;

		auto isAcceptableEncoding = [&acceptEncodingEntries](const auto & encoding) {
			static const auto & begin = acceptEncodingEntries.cbegin();
			static const auto & end = acceptEncodingEntries.cend();

			return end != std::find_if(begin, end, [&encodingName = encoding](const auto & otherEncoding)->bool {
						 return 0 != otherEncoding.qValue && otherEncoding.name == encodingName;
					 });
		};

		for(const auto & encoding : acceptEncodingEntries) {
			if(0 == encoding.qValue) {
				// NEXTRELEASE this logic is not quite right; if the first 0-qValue entry is
				// not * or identity, but * or identity is subseqently present, identity
				// is forbidden but won't be tagged as such
				if("*" == encoding.name || "identity" == encoding.name) {
					canFallBackOnIdentityEncoding = false;
				}

				// the first time we encounter a qValue of 0 we know all subsequent encodings
				// are also unacceptable because the list is sorted by qValue.
				break;
			}

			const auto supportedEncodingIt = SupportedEncodings.find(encoding.name);

			if(supportedEncodingIt != SupportedEncodings.cend()) {
				m_responseEncoding = supportedEncodingIt->second;
				return true;
			}

			if("*" == encoding.name) {
				// server's choice of any encoding it supports, as long as it's not
				// forbidden (qValue == 0)
				for(const auto & encoding : SupportedEncodings) {
					if(isAcceptableEncoding(encoding.first)) {
						m_responseEncoding = encoding.second;
						return true;
					}
				}
			}
		}

		if(!canFallBackOnIdentityEncoding && m_responseEncoding == ContentEncoding::Identity) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to find supported, acceptable encoding from \"" << acceptEncodingHeaderValue << "\"\n";
			return false;
		}

		return true;
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

		while(0 < remaining) {
			bytes = m_socket->write(dataToWrite, remaining);

			if(-1 == bytes) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error writing to TCP socket (\"" << qPrintable(m_socket->errorString()) << "\")\n";
				return false;
			}
#ifndef NDEBUG
			else if(0 == bytes) {
				/// socket buffers so we shouldn't receive 0-length writes, only
				/// successful writes or errors; if we do, we want to know how
				/// likely this is
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: zero-length write to socket (expecting to write up to " << remaining << " byttes)\n";
			}
#endif

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
	 * \return The default title for the response code, or `Unknown` if the
	 * response code is not recognised.
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
	bool RequestHandler::sendResponseCode(HttpResponseCode code, const QString & title) {
		Q_ASSERT_X(ResponseStage::SendingResponse == m_stage, __PRETTY_FUNCTION__, "must be in SendingResponse stage to send the HTTP response header");
		return sendData(QByteArrayLiteral("HTTP/1.1 ") % QByteArray::number(static_cast<unsigned int>(code)) % ' ' % (title.isNull() ? RequestHandler::defaultResponseReason(code).toUtf8() : title.toUtf8()) + EOL);
	}


	/// \fn RequestHandler::sendHeader()
	/// \brief Sends a HTTP header to the client.
	///
	/// \param header is the header to send.
	/// \param value is the value to send for the header.
	///
	/// A call to this method will put the handler into the \c Headers stage. If
	/// the handler is already beyond this stage, the call will fail. After a
	/// successful call to this method, no more response codes may be sent.
	///
	/// \return @c true if the header was sent, \c false otherwise.
	template<class StringType>
	inline bool RequestHandler::sendHeader(const StringType & header, const StringType & value) {
		return sendHeader(QByteArray{static_cast<const char *>(header.data()), static_cast<int>(header.size())}, QByteArray{static_cast<const char *>(value.data()), static_cast<int>(value.size())});
	}


	template<>
	bool RequestHandler::sendHeader(const QByteArray & header, const QByteArray & value) {
		Q_ASSERT_X(ResponseStage::SendingResponse == m_stage || ResponseStage::SendingHeaders == m_stage, __PRETTY_FUNCTION__, "must be in SendingResponse or SendingHeaders stage to send a header");
		m_stage = ResponseStage::SendingHeaders;
		return sendData(header % QByteArrayLiteral(": ") % value % EOL);
	}


	template<>
	inline bool RequestHandler::sendHeader(const QString & header, const QString & value) {
		return sendHeader(header.toUtf8(), value.toUtf8());
	}


	/**
	 * \brief Convenience method to send a date header.
	 *
	 * \param d is the date to send in the header.
	 *
	 * \return @c true if the header was sent, \c false otherwise.
	 */
	bool RequestHandler::sendDateHeader(const QDateTime & d) {
		return sendHeader(QStringLiteral("Date"), d.toUTC().toString(QStringLiteral("ddd, d MMM yyyy hh:mm:ss")) + QStringLiteral(" GMT"));
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
		Q_ASSERT_X(m_encoder, __PRETTY_FUNCTION__, "can't send body until content-encoding has been determined");

		if(ResponseStage::SendingBody != m_stage) {
			sendData(EOL);
			m_stage = ResponseStage::SendingBody;

			if(!m_encoder->startEncoding(*m_socket)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to start data encoding\n";
				return false;
			}
		}

		return m_encoder->encodeTo(*m_socket, body);
	}


	bool RequestHandler::sendBody(QIODevice & in, const std::optional<int> & size) {
		Q_ASSERT_X(m_stage != ResponseStage::Completed, __PRETTY_FUNCTION__, "cannot send body after request response has been fulfilled");
		Q_ASSERT_X(m_encoder, __PRETTY_FUNCTION__, "can't send body until content-encoding has been determined");

		if(ResponseStage::SendingBody != m_stage) {
			sendData(EOL);
			m_stage = ResponseStage::SendingBody;

			if(!m_encoder->startEncoding(*m_socket)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to start data encoding\n";
				return false;
			}
		}

		return m_encoder->encodeTo(*m_socket, in, size);
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
	bool RequestHandler::sendError(HttpResponseCode code, QString msg, const QString & title) {
		Q_ASSERT_X(ResponseStage::SendingResponse == m_stage, __PRETTY_FUNCTION__, "cannot send a complete error response when header or body content has already been sent.");

		QString myTitle = (title.isEmpty() ? RequestHandler::defaultResponseReason(code) : title);

		if(!sendResponseCode(code, myTitle)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of response line for error failed.\n";
			return false;
		}

		if(!sendDateHeader() || !sendHeader(QByteArrayLiteral("Content-type"), QByteArrayLiteral("text/html"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of header for error failed.\n";
			return false;
		}

		msg = (msg.isEmpty() ? RequestHandler::defaultResponseMessage(code) : msg);

		if(!sendData(QByteArrayLiteral("\r\n<html><head><title>") % html_escape(myTitle).toUtf8() % QByteArrayLiteral("</title></head><body><h1>") % QByteArray::number(static_cast<unsigned int>(code)) % ' ' % html_escape(myTitle).toUtf8() % QByteArrayLiteral("</h1><p>") % html_escape(msg).toUtf8() % QByteArrayLiteral("</p></body></html>"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of body content for error failed.\n";
			return false;
		}

		m_stage = ResponseStage::Completed;
		return true;
	}


	void RequestHandler::sendDirectoryListing(const QString & localPath) {
		const auto clientAddr = m_socket->peerAddress().toString();
		const auto clientPort = m_socket->peerPort();

		if(!m_config.directoryListingsAllowed()) {
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
			sendError(HttpResponseCode::Forbidden);
			return;
		}

		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Serve);
		sendResponseCode(HttpResponseCode::Ok);
		sendDateHeader();
		sendHeader(QByteArrayLiteral("Content-type"), QByteArrayLiteral("text/html; charset=UTF-8"));
		sendHeaders(m_encoder->headers());
		QByteArray responseBody = QByteArrayLiteral("<html>\n<head><title>Directory listing for ");
		QByteArray htmlPath(0, '\0');

		// show path in title, and create entry linking to parent dir
		{
			auto uriPath = m_requestUri;
			auto pathIt = uriPath.rbegin();
			const auto pathEnd = uriPath.crend();

			while(pathIt != pathEnd && '/' == *pathIt) {
				++pathIt;
			}

			// if pathIt == pathEnd, pathIt.base() == uriPath.begin()
			uriPath.erase(pathIt.base(), uriPath.cend());
			htmlPath = html_escape(QUrl::fromPercentEncoding(uriPath.data()).toUtf8());
			responseBody += htmlPath % QByteArrayLiteral("</title><style>") % QByteArray::fromStdString(dirListingCss) % QByteArrayLiteral("</style></head>\n<body>\n<div id=\"header\"><p>Directory listing for <em>") % htmlPath % QByteArrayLiteral("/</em></p></div>\n<div id=\"content\"><ul class=\"directory-listing\">");

			if("" != uriPath) {
				auto pos = uriPath.rfind('/');

				if(pos != decltype(uriPath)::npos) {
					uriPath.erase(pos);
				}

				responseBody += QByteArrayLiteral("<li><img src=\"") % mimeIconUri(QStringLiteral("inode/directory")).toUtf8() % QByteArrayLiteral("\" />&nbsp;<em><a href=\"") % (uriPath.empty() ? QByteArrayLiteral("/") : QByteArray(uriPath.data(), static_cast<int>(uriPath.size()))) % "\">&lt;" % tr("parent") % QByteArrayLiteral("&gt;</a></em></li>\n");
			}
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

		QDir::Filters dirListFilters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
		QDir::SortFlags dirSortFlags = QDir::Name;

		if(m_config.showHiddenFilesInDirectoryListings()) {
			dirListFilters |= QDir::Hidden;
		}

		switch(m_config.directoryListingSortOrder()) {
			case DirectoryListingSortOrder::AscendingDirectoriesFirst:
				dirSortFlags |= QDir::DirsFirst;
				break;

			case DirectoryListingSortOrder::AscendingFilesFirst:
				dirSortFlags |= QDir::DirsLast;
				break;

			case DirectoryListingSortOrder::Ascending:
				break;

			case DirectoryListingSortOrder::DescendingDirectoriesFirst:
				dirSortFlags |= QDir::DirsFirst | QDir::Reversed;
				break;

			case DirectoryListingSortOrder::DescendingFilesFirst:
				dirSortFlags |= QDir::DirsLast | QDir::Reversed;
				break;

			case DirectoryListingSortOrder::Descending:
				dirSortFlags |= QDir::Reversed;
				break;
		}

		for(const auto & entry : QDir(localPath).entryInfoList(dirListFilters, dirSortFlags)) {
			const auto htmlFileName = html_escape(entry.fileName());
			responseBody += QByteArrayLiteral("<li");

			if(entry.isSymLink()) {
				// NEXTRELEASE if target is outside doc root, suppress output of link?
				// canonicalFilePath() (on linux, at least) returns entry's path untouched if symlink target is circular
				auto targetEntry = QFileInfo(entry.canonicalFilePath());
				responseBody += QByteArrayLiteral(" class=\"symlink\">");

				if(!targetEntry.exists()) {
					responseBody += "<img src=\"" % mimeIconUri("application/octet-stream").toUtf8() % "\" />&nbsp;";
				}
				else if(targetEntry.isDir()) {
					responseBody += "<img src=\"" % mimeIconUri("inode/directory").toUtf8() % "\" />&nbsp;";
				}
				else if(targetEntry.isFile()) {
					addMimeIconToResponseBody(targetEntry.suffix());
				}
				else {
					responseBody += "<img src=\"" % mimeIconUri("application/octet-stream").toUtf8() % "\" />&nbsp;";
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
				responseBody += "><img src=\"" % mimeIconUri("application/octet-stream").toUtf8() + "\" />&nbsp;";
			}

			responseBody += "<a href=\"" % htmlPath % "/" % htmlFileName % "\">" % htmlFileName % "</a></li>\n";
		}

		responseBody += "</ul></div>\n<div id=\"footer\"><p>" % html_escape(qApp->applicationDisplayName()) % QStringLiteral(" v") % html_escape(qApp->applicationVersion()) % "</p></div></body>\n</html>";
		sendHeader(QStringLiteral("Content-length"), QString::number(responseBody.size()));
		sendHeader(QStringLiteral("Content-MD5"), QString::fromUtf8(QCryptographicHash::hash(responseBody, QCryptographicHash::Md5).toHex()));

		if(HttpMethod::Get == m_requestMethod || HttpMethod::Post == m_requestMethod) {
			sendBody(responseBody);
		}
	}


	void RequestHandler::sendFile(const QString & localPath, const QString & mimeType) {
		const auto clientAddr = m_socket->peerAddress().toString();
		const auto clientPort = m_socket->peerPort();

		if(starts_with(QFileInfo(localPath).absolutePath(), QFileInfo(m_config.cgiBin()).absolutePath())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Refusing to serve file \"" << qPrintable(localPath) << "\" from inside cgi-bin\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
			sendError(HttpResponseCode::Forbidden);
			return;
		}

		QFile localFile(localPath);

		if(!localFile.exists()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: File not found - sending HTTP_NOT_FOUND\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
			sendError(HttpResponseCode::NotFound);
			return;
		}

		if(!localFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: File can't be found - sending HTTP_NOT_FOUND\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
			sendError(HttpResponseCode::NotFound);
			return;
		}

		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Serve);

		sendResponseCode(HttpResponseCode::Ok);
		sendDateHeader();
		sendHeaders(m_encoder->headers());
		sendHeader(QStringLiteral("Content-type"), mimeType);
		sendHeader(QStringLiteral("Content-length"), QString::number(localFile.size()));

		if(HttpMethod::Get == m_requestMethod || HttpMethod::Post == m_requestMethod) {
			sendBody(localFile);
		}

		localFile.close();
	}


	void RequestHandler::doCgi(const QString & localPath, const QString & mimeType) {
		const auto clientAddr = m_socket->peerAddress().toString();
		const auto clientPort = m_socket->peerPort();
		const auto docRoot = QFileInfo(m_config.documentRoot());

		// null means no CGI execution
		if(m_config.cgiBin().isNull()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Server not configured for CGI support - sending HTTP_NOT_FOUND\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
			sendError(HttpResponseCode::NotFound);
			return;
		}

		QString cgiCommandLine;
		QString cgiWorkingDir;
		QString envScriptFileName;

		if(starts_with(m_requestUriPath, {"/cgi-bin/"})) {
			cgiWorkingDir = m_config.cgiBin();
			cgiCommandLine = m_config.cgiBin();

			// use .back() when we can rely on Qt5.10 or later
			if('/' != cgiCommandLine.at(cgiCommandLine.size() - 1)) {
				cgiCommandLine.push_back('/');
			}

			// 9 == "/cgi-bin/".size()
			const auto begin = m_requestUriPath.cbegin() + 9;
			cgiCommandLine.append(QString::fromStdString({begin, m_requestUriPath.cend()}));
			envScriptFileName = cgiCommandLine;
		}
		else {
			cgiCommandLine = m_config.mimeTypeCgi(mimeType);

			if(cgiCommandLine.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing CGI processor for CGI script (\"" << m_requestUri << "\", MIME type " << qPrintable(mimeType) << ") not in cgi-bin\n";
				Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
				sendError(HttpResponseCode::Forbidden);
				return;
			}

			cgiCommandLine = QFileInfo(cgiCommandLine).absoluteFilePath();

			if(cgiCommandLine.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: CGI processor \"" << qPrintable(m_config.mimeTypeCgi(mimeType)) << "\" for CGI script (\"" << m_requestUri << "\", MIME type " << qPrintable(mimeType) << ") not found\n";
				Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
				sendError(HttpResponseCode::Forbidden);
				return;
			}

			cgiCommandLine += QStringLiteral(" \"") % localPath % '\"';
			auto localPathInfo = QFileInfo(localPath);
			cgiWorkingDir = localPathInfo.absolutePath();
			envScriptFileName = localPathInfo.absoluteFilePath();
		}

		// cgiCommandLine is now fully-resolved path to executable with script as path if necessary
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: CGI command line: " << qPrintable(cgiCommandLine) << "\n"
					 << std::flush;

		QStringList env = {QStringLiteral("GATEWAY_INTERFACE=CGI/1.1"),
								 QStringLiteral("REDIRECT_STATUS=1"),  // non-standard, but since 5.3 is required to make PHP happy
								 QStringLiteral("REMOTE_ADDR=") % clientAddr,
								 QStringLiteral("REMOTE_PORT=") % QString::number(clientPort),
								 QStringLiteral("REQUEST_METHOD=") % QString::fromStdString(m_requestMethodString),
								 QStringLiteral("REQUEST_URI=") % QString::fromStdString(m_requestUri),
								 QStringLiteral("SCRIPT_NAME=") % QString::fromStdString(m_requestUriPath),
								 QStringLiteral("SCRIPT_FILENAME=") % envScriptFileName,
								 QStringLiteral("SERVER_NAME=") % m_config.listenAddress(),
								 QStringLiteral("SERVER_ADDR=") % m_config.listenAddress(),
								 QStringLiteral("SERVER_PORT=") % QString::number(m_config.port()),
								 QStringLiteral("DOCUMENT_ROOT=") % docRoot.absoluteFilePath(),
								 QStringLiteral("SERVER_PROTOCOL=HTTP/") % QString::fromStdString(m_requestHttpVersion.data()),
								 QStringLiteral("SERVER_SOFTWARE=") % qApp->applicationName() % QStringLiteral(" v") % qApp->applicationVersion(),
								 QStringLiteral("SERVER_SIGNATURE=AnansiRequestHandler on ") % m_config.listenAddress() % QStringLiteral(" port ") % QString::number(m_config.port()),
								 QStringLiteral("SERVER_ADMIN=") % m_config.administratorEmail()};

		if(!m_requestUriQuery.empty()) {
			env.push_back(QStringLiteral("QUERY_STRING=") % QString::fromStdString(m_requestUriQuery.data()));
		}

		const auto contentTypeIter = m_requestHeaders.find("content-type");

		if(m_requestHeaders.cend() != contentTypeIter) {
			env.push_back(QStringLiteral("CONTENT_TYPE=") % QString::fromStdString(contentTypeIter->second));
			env.push_back(QStringLiteral("CONTENT_LENGTH=") % QString::number(m_requestBody.size()));
		}

		// put the HTTP headers into the CGI environment
		for(const auto & header : m_requestHeaders) {
			env.push_back(QStringLiteral("HTTP_") % QString::fromStdString(header.first).replace('-', '_').toUpper() % "=" % header.second.data());
		}

		QProcess cgiProcess;

		// ensure CGI process is closed on all exit paths
		Equit::ScopeGuard cgiProcessGuard = [&cgiProcess]() {
			cgiProcess.close();
		};

		cgiProcess.setEnvironment(env);
		cgiProcess.setWorkingDirectory(cgiWorkingDir);
		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::CGI);
		cgiProcess.start(cgiCommandLine, QIODevice::ReadWrite);

		if(!cgiProcess.waitForStarted(m_config.cgiTimeout())) {
			if(QProcess::Timedout == cgiProcess.error()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Timeout waiting for CGI process to start.\n";
				sendError(HttpResponseCode::RequestTimeout);
			}
			else {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Error starting CGI process: \"" << qPrintable(cgiProcess.errorString()) << "\".\n";
				sendError(HttpResponseCode::InternalServerError);
			}

			return;
		}

		if(!cgiProcess.waitForFinished(m_config.cgiTimeout())) {
			if(QProcess::Timedout == cgiProcess.error()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Timeout waiting for CGI process to complete.\n";
				sendError(HttpResponseCode::RequestTimeout);
			}
			else {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Error in CGI process: \"" << qPrintable(cgiProcess.errorString()) << "\".\n";
				sendError(HttpResponseCode::InternalServerError);
			}

			return;
		}

		cgiProcess.waitForReadyRead();

		if(0 != cgiProcess.exitCode()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: CGI process returned error status " << cgiProcess.exitCode() << "\n";
			std::cerr << qPrintable(cgiProcess.readAllStandardError()) << "\n";
		}

		std::string headerData;
		std::regex headerRx("^([a-zA-Z][a-zA-Z\\-]*) *: *(.+)$");

		while(true) {
			auto headerLine = readHeaderLine(cgiProcess);

			if(!headerLine) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid CGI output - invalid header\n";
			}

			if(headerLine->empty()) {
				// all headers read
				break;
			}

			if(!std::regex_match(*headerLine, headerRx)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid CGI output (invalid header \"" << *headerLine << "\")\n";
				sendError(HttpResponseCode::InternalServerError);
				return;
			}

			headerData.append(*headerLine);
			headerData.append(EOL);
		}

		sendResponseCode(HttpResponseCode::Ok);
		sendHeaders(m_encoder->headers());
		sendDateHeader();
		sendData(QByteArray::fromStdString(headerData));
		sendBody(cgiProcess);
	}


	ConnectionPolicy RequestHandler::determineConnectionPolicy() {
		QString clientAddress = m_socket->peerAddress().toString();
		uint16_t clientPort = m_socket->peerPort();
		Q_EMIT handlingRequestFrom(clientAddress, clientPort);
		ConnectionPolicy policy = m_config.ipAddressConnectionPolicy(clientAddress);
		Q_EMIT requestConnectionPolicyDetermined(clientAddress, clientPort, policy);

		switch(policy) {
			case ConnectionPolicy::Accept:
				Q_EMIT acceptedRequestFrom(clientAddress, clientPort);
				break;

			case ConnectionPolicy::None:
			case ConnectionPolicy::Reject:
				Q_EMIT rejectedRequestFrom(clientAddress, clientPort, tr("Policy for this IP address is Reject"));
				break;
		}

		return policy;
	}


	bool RequestHandler::readRequestHeaders() {
		std::regex headerRx("^([a-zA-Z][a-zA-Z\\-]*) *: *(.+)$");
		std::smatch captures;

		while(true) {
			auto headerLine = readHeaderLine(*m_socket);

			if(!headerLine) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid header)\n";
				return false;
			}

			if(headerLine->empty()) {
				// all headers read
				break;
			}

			if(!std::regex_match(*headerLine, captures, headerRx)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid header \"" << *headerLine << "\")\n";
				return false;
			}

			m_requestHeaders.emplace(to_lower(captures.str(1)), captures.str(2));
		}

		return true;
	}


	// empty optional if invalid; -1 if absent; >= 0 if present and valid
	std::optional<int> RequestHandler::parseRequestContentLength() {
		const auto contentLengthIt = m_requestHeaders.find("content-length");
		auto contentLength = -1L;

		if(contentLengthIt != m_requestHeaders.cend()) {
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
				return {};
			}
		}

		return contentLength;
	}


	bool RequestHandler::readRequestBody(int contentLength) {
		Q_ASSERT_X(-1 <= contentLength, __PRETTY_FUNCTION__, "invalid content length");
		std::array<char, ReadBufferSize> readBuffer;
		auto bytesRemaining = contentLength;
		int consecutiveTimeoutCount = 0;
		m_requestBody.clear();

		if(0 < contentLength && m_requestBody.capacity() < static_cast<std::string::size_type>(bytesRemaining)) {
			m_requestBody.reserve(static_cast<std::string::size_type>(bytesRemaining));  // +1 for null?
		}

		while((-1 == contentLength || 0 < bytesRemaining) && !m_socket->atEnd()) {
			auto bytesRead = m_socket->read(&readBuffer[0], readBuffer.size());

			if(-1 == bytesRead) {
				if(QAbstractSocket::SocketTimeoutError != m_socket->error()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error reading body data from socket (" << qPrintable(m_socket->errorString()) << ")\n";
					return {};
				}

				++consecutiveTimeoutCount;

				if(MaxReadErrorCount < consecutiveTimeoutCount) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: too many timeouts attempting to read request body\n";
					return {};
				}
			}
			else {
				m_requestBody.append(&readBuffer[0], static_cast<std::string::size_type>(bytesRead));
				consecutiveTimeoutCount = 0;
			}
		}

		if(0 < bytesRemaining) {
			// not enough body data
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket stopped providing data while still expecting " << bytesRemaining << " bytes (\"" << qPrintable(m_socket->errorString()) << "\")\n";
			return false;
		}

		if(!m_socket->atEnd() || (-1 != contentLength && 0 > bytesRemaining)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket provided more body data than expected (at least " << (-bytesRemaining) << " surplus bytes)\n";
		}

		return true;
	}


	std::optional<std::tuple<std::string, std::string, std::string>> RequestHandler::readHttpRequestLine() {
		auto requestLine = readHeaderLine(*m_socket);

		if(!requestLine) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (failed to read request line)\n";
			return {};
		}

		std::smatch captures;

		if(!std::regex_match(*requestLine, captures, std::regex("^(OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT) ([^ ]+) HTTP/([0-9](?:\\.[0-9]+)*)$"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request line \"" << *requestLine << "\"\n";
			return {};
		}

		return std::make_tuple(captures[1], captures[2], captures[3]);
	}


	/**
	 * \brief Point of entry for the thread.
	 *
	 * This is where the handler starts execution. This method simply sets up the
	 * socket object, reads and parses the request line from the socket, and passes
	 * the details on to the handleHTTPRequest() method.
	 */
	void RequestHandler::run() {
		Q_ASSERT_X(m_socket, __PRETTY_FUNCTION__, "null socket");

		// scope guard does all cleanup on all exit paths
		Equit::ScopeGuard cleanup = [this]() {
			m_socket->flush();
			disposeSocket();
		};

		if(ConnectionPolicy::Accept != determineConnectionPolicy()) {
			sendError(HttpResponseCode::Forbidden);
			return;
		}

		auto requestLine = readHttpRequestLine();

		if(!requestLine) {
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		// safe to deref optional without checking because RX in readRequestLine
		// guarantees method string is one that parseHttpMethod() will accept
		std::tie(m_requestMethodString, m_requestUri, m_requestHttpVersion) = *requestLine;
		m_requestMethod = *parseHttpMethod(m_requestMethodString);

		if(!readRequestHeaders()) {
			sendError(HttpResponseCode::BadRequest);
			return;
		}


		auto contentLength = parseRequestContentLength();

		if(!contentLength) {
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		if(!readRequestBody(*contentLength)) {
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		handleHttpRequest();
	}


	/// \brief Handle a parsed HTTP request.
	///
	/// \param httpVersion is the HTTP version of the request (usually 1.1).
	/// \param method is the HTTP method the request is using.
	/// \param uri is the URI of the resource requested.
	/// \param headers is the set of headers sent with the request.
	/// \param body is the message body sent with the request.
	///
	/// The default implementation handles HTTP 1.1 requests. Future or later
	/// versions of
	/// the protocol can be handled using subclasses.
	///
	/// \note At present, only requests using the GET method are accepted.
	void RequestHandler::handleHttpRequest() {
		if(!m_socket) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no socket\n";
			return;
		}

		// will accept anything up to HTTP/1.1 and process it as HTTP/1.1
		if("1.0" != m_requestHttpVersion && "1.1" != m_requestHttpVersion) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: HTTP version (HTTP/" << m_requestHttpVersion << ") is not supported\n";
			sendError(HttpResponseCode::HttpVersionNotSupported);
			return;
		}

		// for now we only support GET, HEAD and POST, which covers the REQUIRED HTTP/1.1 methods (GET, HEAD).
		// in future we may need to support all HTTP 1.1 methods: OPTIONS,GET,HEAD,POST,PUT,DELETE,TRACE,CONNECT
		if(HttpMethod::Get != m_requestMethod && HttpMethod::Head != m_requestMethod && HttpMethod::Post != m_requestMethod) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Request method " << enumeratorString(m_requestMethod) << " not supported\n";
			sendError(HttpResponseCode::NotImplemented);
			return;
		}

		std::regex rxUri("^([^?#]*)(?:\\?([^#]+))?(?:#(.*))?$");
		std::smatch captures;

		if(!std::regex_match(m_requestUri, captures, rxUri)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed parsing request URI \"" << m_requestUri << "\"\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		m_requestUriPath = percent_decode(captures[1].str());
		m_requestUriQuery = captures[2];

		// we should never receive a fragment, should we?
		m_requestUriFragment = captures[3];

		const auto & md5It = m_requestHeaders.find("content-md5");

		if(md5It != m_requestHeaders.cend()) {
			QCryptographicHash hash(QCryptographicHash::Md5);
			hash.addData(m_requestBody.data(), static_cast<int>(m_requestBody.size()));

			if(md5It->second != hash.result().toHex().constData()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: calculated MD5 of request body does not match Content-MD5 header\n";
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: calculated:" << hash.result().toHex().constData() << "; header:" << md5It->second << "\n";
				// I think this is the correct response for this error case
				sendError(HttpResponseCode::BadRequest);
			}

			return;
		}

		QFileInfo docRoot(m_config.documentRoot());
		QFileInfo resource(docRoot.absoluteFilePath() + "/" + QString::fromStdString(m_requestUriPath));  //uri.toLocalFile());
		QString resolvedResourcePath = resource.absoluteFilePath();

		// only serve request from inside doc root
		if(!starts_with(resolvedResourcePath, docRoot.absoluteFilePath())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: requested local resource is outside document root.\n";
			sendError(HttpResponseCode::NotFound);
			return;
		}

		const auto clientAddr = m_socket->peerAddress().toString();
		const auto clientPort = m_socket->peerPort();

		/// on leaving the method, ensure content encoder has finished its job
		Equit::ScopeGuard finishSendingBody = [this]() {
			if(m_encoder) {
				m_encoder->finishEncoding(*m_socket);
			}
		};

		determineResponseEncoding();

		switch(m_responseEncoding) {
			case ContentEncoding::Deflate:
				m_encoder = std::make_unique<DeflateContentEncoder>();
				break;

			case ContentEncoding::Gzip:
				m_encoder = std::make_unique<GzipContentEncoder>();
				break;

			case ContentEncoding::Identity:
				m_encoder = std::make_unique<IdentityContentEncoder>();
				break;
		}

		if(!m_encoder) {
			const auto acceptIt = m_requestHeaders.find("accept-encoding");
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to find a suitable content encoder (accept-encoding: " << (m_requestHeaders.cend() != acceptIt ? acceptIt->second : "<not specified>") << "\n";
			sendError(HttpResponseCode::NotAcceptable, tr("No supported, acceptable content-encoding could be determined."));
			return;
		}

		if(resource.isDir()) {
			sendDirectoryListing(resolvedResourcePath);
			m_stage = ResponseStage::Completed;
			return;
		}

		auto suffix = resource.suffix();

		if(suffix == resource.fileName()) {
			// e.g. ".bashrc" will have basename "" and suffix "bashrc", so fix this
			// so fit convention
			suffix = "";
		}

		// NEXTRELEASE support fcgi
		for(const auto & mimeType : m_config.mimeTypesForFileExtension(suffix)) {
			switch(m_config.mimeTypeAction(mimeType)) {
				case WebServerAction::Ignore:
					// do nothing - just try the next MIME type for the resource
					break;

				case WebServerAction::Serve:
					sendFile(resolvedResourcePath, mimeType);
					m_stage = ResponseStage::Completed;
					return;

				case WebServerAction::CGI:
					doCgi(resolvedResourcePath, mimeType);
					m_stage = ResponseStage::Completed;
					return;

				case WebServerAction::Forbid:
					Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
					sendError(HttpResponseCode::Forbidden);
					return;
			}
		}

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no action configured for resource \"" << m_requestUriPath << "\"\n";
		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestUri), WebServerAction::Forbid);
		sendError(HttpResponseCode::NotFound);
	}


}  // namespace Anansi
