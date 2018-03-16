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

/// \file requesthandler.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date Marh 2018
///
/// \brief Implementation of the RequestHandler class for Anansi.
///
/// \dep
/// - requesthandler.h
/// - <iostream>
/// - <algorithm>
/// - <cstdint>
/// - <string>
/// - <array>
/// - <vector>
/// - <optional>
/// - <regex>
/// - <QByteArray>
/// - <QStringBuilder>
/// - <QApplication>
/// - <QCryptographicHash>
/// - <QDir>
/// - <QFile>
/// - <QFileInfo>
/// - <QUrl>
/// - <QHostAddress>
/// - <QProcess>
/// - assert.h
/// - qtmetatypes.h
/// - configuration.h
/// - strings.h
/// - scopeguard.h
/// - mediatypeicons.h
/// - deflatecontentencoder.h
/// - gzipcontentencoder.h
/// - identitycontentencoder.h
///
/// \par Changes
/// - (2018-03) First release.

#include "requesthandler.h"

#include <iostream>
#include <algorithm>
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <optional>
#include <regex>

#include <QByteArray>
#include <QStringBuilder>
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QHostAddress>
#include <QProcess>

#include "assert.h"
#include "qtmetatypes.h"
#include "configuration.h"
#include "strings.h"
#include "scopeguard.h"
#include "mediatypeicons.h"
#include "deflatecontentencoder.h"
#include "gzipcontentencoder.h"
#include "identitycontentencoder.h"


namespace Anansi {


	using Equit::ScopeGuard;
	using Equit::percent_decode;
	using Equit::starts_with;
	using Equit::to_html_entities;
	using Equit::to_lower;
	using Equit::parse_int;


	static constexpr const int MaxReadErrorCount = 3;
	static constexpr const unsigned int ReadBufferSize = 1024;
	static const QByteArray EOL = QByteArrayLiteral("\r\n");


	static const std::unordered_map<std::string, ContentEncoding> SupportedEncodings = {
	  {"deflate", ContentEncoding::Deflate},
	  {"gzip", ContentEncoding::Gzip},
	  {"identity", ContentEncoding::Identity},
	};


	// resource file is loaded when first needed, to keep mem footprint a little lower
	// interesting(!) note: initialising using a lambda version of loadDirListingCss()
	// that is immediately invoked - e.g. dirListingCss = ([](){...})(); - works for
	// debug builds but not for release builds (the compiled-in resource
	// ":/stylesheets/directory-listing" is not found). optimisation issue?
	static std::string dirListingCss;


	static bool loadDirListingCss() {
		QFile staticResourceFile(QStringLiteral(":/stylesheets/directory-listing"));

		if(!staticResourceFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read built-in directory listing stylesheet (couldn't open resource file)\n";
			return false;
		}

		dirListingCss.reserve(static_cast<std::string::size_type>(staticResourceFile.size() + 1));

		while(!staticResourceFile.atEnd()) {
			dirListingCss.append(staticResourceFile.readAll().constData());
		}

		return true;
	};


	template<class StringType = std::string>
	static std::optional<HttpMethod> parseHttpMethod(const StringType & str) {
		if("OPTIONS" == str) {
			return HttpMethod::Options;
		}

		if("GET" == str) {
			return HttpMethod::Get;
		}

		if("HEAD" == str) {
			return HttpMethod::Head;
		}

		if("POST" == str) {
			return HttpMethod::Post;
		}

		if("PUT" == str) {
			return HttpMethod::Put;
		}

		if("DELETE" == str) {
			return HttpMethod::Delete;
		}

		if("TRACE" == str) {
			return HttpMethod::Trace;
		}

		if("CONNECT" == str) {
			return HttpMethod::Connect;
		}

		return {};
	}


	template<class BufferType = std::string>
	static std::optional<BufferType> readHeaderLine(QIODevice & in) {
		std::array<char, ReadBufferSize> readBuffer;
		BufferType line;
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

		// no need to check for trailing '\n', its presence is a necessary read loop exit condition
		if(2 > length || '\r' != line[static_cast<std::string::size_type>(length) - 2]) {
			return {};
		}

		// trim off trailing \r\n
		const auto end = line.cend();
		line.erase(end - 2, end);
		return line;
	}


	template<class StringType>
	StringType RequestHandler::responseStageString(ResponseStage stage) {
		switch(stage) {
			case ResponseStage::SendingResponse:
				return "Completed";

			case ResponseStage::SendingHeaders:
				return "Completed";

			case ResponseStage::SendingBody:
				return "SendingBody";

			case ResponseStage::Completed:
				return "Completed";
		}

		eqAssert(false, "unhandled ResponseStage value " << static_cast<int>(stage));
		return {};
	}

	RequestHandler::RequestHandler(std::unique_ptr<QTcpSocket> socket, const Configuration & config, QObject * parent)
	: QThread(parent),
	  m_socket(std::move(socket)),
	  m_config(config),
	  m_stage(ResponseStage::SendingResponse),
	  m_responseEncoding(ContentEncoding::Identity),
	  m_encoder(nullptr) {
		Q_ASSERT(m_socket);
		m_socket->moveToThread(this);
	}


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

		// q-values stored * 1000 for ease of comparison
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

		std::stable_sort(acceptEncodingEntries.begin(), acceptEncodingEntries.end(), [](const auto & firstEncoding, const auto & secondEncoding) {
			// > rather than < because we wan't reverse sort (higest qValue first)
			return firstEncoding.qValue > secondEncoding.qValue;
		});

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

				// at the first qValue of 0 we know all subsequent encodings are also
				// unacceptable because the list is sorted by descending qValue.
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


	bool RequestHandler::sendData(const QByteArray & data) {
		if(!m_socket->isWritable()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: tcp socket  is not writable\n";
			return false;
		}

		int64_t bytes;
		int remaining = data.size();
		const char * buffer = data.data();

		while(0 < remaining) {
			bytes = m_socket->write(buffer, remaining);

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

			buffer += bytes;
			remaining -= bytes;
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


	bool RequestHandler::sendResponseCode(HttpResponseCode code, const std::optional<QString> & title) {
		eqAssert(ResponseStage::SendingResponse == m_stage, "must be in SendingResponse stage to send the HTTP response header (stage is currently " << responseStageString<std::string>(m_stage) << ")");
		return sendData(QByteArrayLiteral("HTTP/1.1 ") % QByteArray::number(static_cast<unsigned int>(code)) % ' ' % (!title ? RequestHandler::defaultResponseReason(code).toUtf8() : title->toUtf8()) + EOL);
	}


	template<class StringType>
	inline bool RequestHandler::sendHeader(const StringType & header, const StringType & value) {
		return sendHeader(QByteArray::fromRawData(static_cast<const char *>(header.data()), static_cast<int>(header.size())), QByteArray::fromRawData(static_cast<const char *>(value.data()), static_cast<int>(value.size())));
	}


	template<>
	bool RequestHandler::sendHeader(const QByteArray & header, const QByteArray & value) {
		eqAssert(ResponseStage::SendingResponse == m_stage || ResponseStage::SendingHeaders == m_stage, "must be in SendingResponse or SendingHeaders stage to send a header (stage is currently " << responseStageString<std::string>(m_stage) << ")");
		m_stage = ResponseStage::SendingHeaders;
		return sendData(header % QByteArrayLiteral(": ") % value % EOL);
	}


	template<>
	inline bool RequestHandler::sendHeader(const QString & header, const QString & value) {
		return sendHeader(header.toUtf8(), value.toUtf8());
	}


	bool RequestHandler::sendDateHeader(const QDateTime & date) {
		return sendHeader(QByteArrayLiteral("Date"), date.toUTC().toString(QStringLiteral("ddd, d MMM yyyy hh:mm:ss")).toUtf8() + QByteArrayLiteral(" GMT"));
	}


	bool RequestHandler::sendBody(const QByteArray & body) {
		eqAssert(m_stage != ResponseStage::Completed, "cannot send body after request response has been fulfilled (stage is currently " << responseStageString<std::string>(m_stage) << ")");
		eqAssert(m_encoder, "can't send body until content-encoding has been determined");

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
		eqAssert(m_stage != ResponseStage::Completed, "cannot send body after request response has been fulfilled (stage is currently " << responseStageString<std::string>(m_stage) << ")");
		eqAssert(m_encoder, "can't send body until content-encoding has been determined");

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


	bool RequestHandler::sendError(HttpResponseCode code, QString msg, QString title) {
		eqAssert(ResponseStage::SendingResponse == m_stage, "cannot send a complete error response when header or body content has already been sent (stage is currently " << responseStageString<std::string>(m_stage) << ")");

		if(title.isEmpty()) {
			title = RequestHandler::defaultResponseReason(code);
		}

		if(!sendResponseCode(code, title)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of response line for error failed.\n";
			return false;
		}

		if(!sendDateHeader() || !sendHeader(QByteArrayLiteral("Content-type"), QByteArrayLiteral("text/html"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of header for error failed.\n";
			return false;
		}

		if(msg.isEmpty()) {
			msg = RequestHandler::defaultResponseMessage(code);
		}

		const auto htmlTitle = to_html_entities(title).toUtf8();

		if(!sendData(QByteArrayLiteral("\r\n<html><head><title>") % htmlTitle % QByteArrayLiteral("</title></head><body><h1>") % QByteArray::number(static_cast<unsigned int>(code)) % ' ' % htmlTitle % QByteArrayLiteral("</h1><p>") % to_html_entities(msg).toUtf8() % QByteArrayLiteral("</p></body></html>"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: sending of body content for error failed.\n";
			return false;
		}

		m_stage = ResponseStage::Completed;
		return true;
	}


	void RequestHandler::sendDirectoryListing(const QString & localPath) {
		const QString clientAddr = m_socket->peerAddress().toString();
		const uint16_t clientPort = m_socket->peerPort();

		if(!m_config.directoryListingsAllowed()) {
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
			sendError(HttpResponseCode::Forbidden);
			return;
		}

		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Serve);
		sendResponseCode(HttpResponseCode::Ok);
		sendDateHeader();
		sendHeader(QByteArrayLiteral("Content-type"), QByteArrayLiteral("text/html; charset=UTF-8"));
		sendHeaders(m_encoder->headers());
		QByteArray responseBody = QByteArrayLiteral("<html>\n<head><title>Directory listing for ");
		QByteArray htmlPath(0, '\0');

		// show path in title, and create entry linking to parent dir
		{
			auto uriPath = m_requestLine.uri;
			auto pathIt = uriPath.rbegin();
			const auto pathEnd = uriPath.crend();

			while(pathIt != pathEnd && '/' == *pathIt) {
				++pathIt;
			}

			if(dirListingCss.empty()) {
				loadDirListingCss();
			}

			// if pathIt == pathEnd, pathIt.base() == uriPath.begin()
			uriPath.erase(pathIt.base(), uriPath.cend());
			htmlPath = to_html_entities(QUrl::fromPercentEncoding(uriPath.data()).toUtf8());
			responseBody += htmlPath % QByteArrayLiteral("</title><style>") % QByteArray::fromStdString(dirListingCss) % QByteArrayLiteral("</style></head>\n<body>\n<div id=\"header\"><p>Directory listing for <em>") % htmlPath % QByteArrayLiteral("/</em></p></div>\n<div id=\"content\"><ul class=\"directory-listing\">");

			if("" != uriPath) {
				auto pos = uriPath.rfind('/');

				if(pos != decltype(uriPath)::npos) {
					uriPath.erase(pos);
				}

				responseBody += QByteArrayLiteral("<li><img src=\"") % mediaTypeIconUri(QStringLiteral("inode/directory")) % QByteArrayLiteral("\" />&nbsp;<em><a href=\"") % (uriPath.empty() ? QByteArrayLiteral("/") : QByteArray(uriPath.data(), static_cast<int>(uriPath.size()))) % "\">&lt;" % tr("parent") % QByteArrayLiteral("&gt;</a></em></li>\n");
			}
		}

		const auto addMediaTypeIconToResponseBody = [&responseBody, this](const auto & ext) {
			if(!ext.isEmpty()) {
				for(const auto & mediaType : m_config.fileExtensionMediaTypes(ext)) {
					const auto mediaTypeIcon = mediaTypeIconUri(mediaType);

					if(!mediaTypeIcon.isEmpty()) {
						responseBody += "<img src=\"" % mediaTypeIcon % "\" />&nbsp;";
						return;
					}
				}
			}

			responseBody += QByteArrayLiteral("<img src=\"") % mediaTypeIconUri(QStringLiteral("application/octet-stream")) % QByteArrayLiteral("\" />&nbsp;");
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
			const auto htmlFileName = to_html_entities(entry.fileName());
			responseBody += QByteArrayLiteral("<li");

			if(entry.isSymLink()) {
				// NEXTRELEASE if target is outside doc root, suppress output of link?
				// canonicalFilePath() (on linux, at least) returns entry's path untouched if symlink target is circular
				auto targetEntry = QFileInfo(entry.canonicalFilePath());
				responseBody += QByteArrayLiteral(" class=\"symlink\">");

				if(!targetEntry.exists()) {
					responseBody += QByteArrayLiteral("<img src=\"") % mediaTypeIconUri(QStringLiteral("application/octet-stream")) % QByteArrayLiteral("\" />&nbsp;");
				}
				else if(targetEntry.isDir()) {
					responseBody += QByteArrayLiteral("<img src=\"") % mediaTypeIconUri(QStringLiteral("inode/directory")) % QByteArrayLiteral("\" />&nbsp;");
				}
				else if(targetEntry.isFile()) {
					addMediaTypeIconToResponseBody(targetEntry.suffix());
				}
				else {
					responseBody += QByteArrayLiteral("<img src=\"") % mediaTypeIconUri(QStringLiteral("application/octet-stream")) % QByteArrayLiteral("\" />&nbsp;");
				}
			}
			else if(entry.isDir()) {
				responseBody += QByteArrayLiteral(" class=\"directory\"><img src=\"") % mediaTypeIconUri(QStringLiteral("inode/directory")) % QByteArrayLiteral("\" />&nbsp;");
			}
			else if(entry.isFile()) {
				responseBody += QByteArrayLiteral(" class=\"file\">");
				addMediaTypeIconToResponseBody(entry.suffix());
			}
			else {
				responseBody += QByteArrayLiteral("><img src=\"") % mediaTypeIconUri(QStringLiteral("application/octet-stream")) % QByteArrayLiteral("\" />&nbsp;");
			}

			responseBody += QByteArrayLiteral("<a href=\"") % htmlPath % '/' % htmlFileName % QByteArrayLiteral("\">") % htmlFileName % QByteArrayLiteral("</a></li>\n");
		}

		responseBody += QByteArrayLiteral("</ul></div>\n<div id=\"footer\"><p>") % to_html_entities(qApp->applicationDisplayName()) % QStringLiteral(" v") % to_html_entities(qApp->applicationVersion()) % "</p></div></body>\n</html>";
		sendHeader(QStringLiteral("Content-length"), QString::number(responseBody.size()));
		sendHeader(QStringLiteral("Content-MD5"), QString::fromUtf8(QCryptographicHash::hash(responseBody, QCryptographicHash::Md5).toHex()));

		if(HttpMethod::Get == m_requestMethod || HttpMethod::Post == m_requestMethod) {
			sendBody(responseBody);
		}
	}


	void RequestHandler::sendFile(const QString & localPath, const QString & mediaType) {
		const QString clientAddr = m_socket->peerAddress().toString();
		const uint16_t clientPort = m_socket->peerPort();

		if(starts_with(QFileInfo(localPath).absolutePath(), QFileInfo(m_config.cgiBin()).absolutePath())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Refusing to serve file \"" << qPrintable(localPath) << "\" from inside cgi-bin\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
			sendError(HttpResponseCode::Forbidden);
			return;
		}

		QFile localFile(localPath);

		if(!localFile.exists()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: File not found - sending HTTP_NOT_FOUND\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
			sendError(HttpResponseCode::NotFound);
			return;
		}

		if(!localFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: File can't be found - sending HTTP_NOT_FOUND\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
			sendError(HttpResponseCode::NotFound);
			return;
		}

		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Serve);

		sendResponseCode(HttpResponseCode::Ok);
		sendDateHeader();
		sendHeaders(m_encoder->headers());
		sendHeader(QStringLiteral("Content-type"), mediaType);
		sendHeader(QStringLiteral("Content-length"), QString::number(localFile.size()));

		if(HttpMethod::Get == m_requestMethod || HttpMethod::Post == m_requestMethod) {
			sendBody(localFile);
		}

		localFile.close();
	}


	void RequestHandler::doCgi(const QString & localPath, const QString & mediaType) {
		const QString clientAddr = m_socket->peerAddress().toString();
		const uint16_t clientPort = m_socket->peerPort();
		const auto docRoot = QFileInfo(m_config.documentRoot());

		// null means no CGI execution
		if(m_config.cgiBin().isNull()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Server not configured for CGI support - sending HTTP_NOT_FOUND\n";
			Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
			sendError(HttpResponseCode::NotFound);
			return;
		}

		QString cgiCommandLine;
		QString cgiWorkingDir;
		QString envScriptFileName;

		if(starts_with(m_requestUri.path, "/cgi-bin/")) {
			cgiWorkingDir = m_config.cgiBin();
			cgiCommandLine = m_config.cgiBin();

			// use .back() when we can rely on Qt5.10 or later
			if('/' != cgiCommandLine.at(cgiCommandLine.size() - 1)) {
				cgiCommandLine.push_back('/');
			}

			// 9 == "/cgi-bin/".size()
			const auto begin = m_requestUri.path.cbegin() + 9;
			cgiCommandLine.append(QString::fromStdString({begin, m_requestUri.path.cend()}));
			envScriptFileName = cgiCommandLine;
		}
		else {
			cgiCommandLine = m_config.mediaTypeCgi(mediaType);

			if(cgiCommandLine.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no CGI processor set for script \"" << m_requestLine.uri << "\" (media type: " << qPrintable(mediaType) << ")\n";
				Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
				sendError(HttpResponseCode::Forbidden);
				return;
			}

			cgiCommandLine = QFileInfo(cgiCommandLine).absoluteFilePath();

			if(cgiCommandLine.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: CGI processor \"" << qPrintable(m_config.mediaTypeCgi(mediaType)) << "\" for CGI script (\"" << m_requestLine.uri << "\", media type " << qPrintable(mediaType) << ") not found\n";
				Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
				sendError(HttpResponseCode::Forbidden);
				return;
			}

			cgiCommandLine += QStringLiteral(" \"") % localPath % '\"';
			auto localPathInfo = QFileInfo(localPath);
			cgiWorkingDir = localPathInfo.absolutePath();
			envScriptFileName = localPathInfo.absoluteFilePath();
		}

		// cgiCommandLine is now fully-resolved path to executable with script as path if necessary

		QStringList env = {QStringLiteral("GATEWAY_INTERFACE=CGI/1.1"),
								 QStringLiteral("REDIRECT_STATUS=1"),  // non-standard, but since 5.3 is required to make PHP happy
								 QStringLiteral("REMOTE_ADDR=") % clientAddr,
								 QStringLiteral("REMOTE_PORT=") % QString::number(clientPort),
								 QStringLiteral("REQUEST_METHOD=") % QString::fromStdString(m_requestLine.method),
								 QStringLiteral("REQUEST_URI=") % QString::fromStdString(m_requestLine.uri),
								 QStringLiteral("SCRIPT_NAME=") % QString::fromStdString(m_requestUri.path),
								 QStringLiteral("SCRIPT_FILENAME=") % envScriptFileName,
								 QStringLiteral("SERVER_NAME=") % m_config.listenAddress(),
								 QStringLiteral("SERVER_ADDR=") % m_config.listenAddress(),
								 QStringLiteral("SERVER_PORT=") % QString::number(m_config.port()),
								 QStringLiteral("DOCUMENT_ROOT=") % docRoot.absoluteFilePath(),
								 QStringLiteral("SERVER_PROTOCOL=HTTP/") % QString::fromStdString(m_requestLine.httpVersion),
								 QStringLiteral("SERVER_SOFTWARE=") % qApp->applicationName() % QStringLiteral(" v") % qApp->applicationVersion(),
								 QStringLiteral("SERVER_SIGNATURE=AnansiRequestHandler on ") % m_config.listenAddress() % QStringLiteral(" port ") % QString::number(m_config.port()),
								 QStringLiteral("SERVER_ADMIN=") % m_config.administratorEmail()};

		if(!m_requestUri.query.empty()) {
			env.push_back(QStringLiteral("QUERY_STRING=") % QString::fromStdString(m_requestUri.query));
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
		ScopeGuard cgiProcessGuard = [&cgiProcess]() {
			cgiProcess.close();
		};

		cgiProcess.setEnvironment(env);
		cgiProcess.setWorkingDirectory(cgiWorkingDir);
		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::CGI);
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


	ConnectionPolicy RequestHandler::determineConnectionPolicy() const {
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


	std::optional<int> RequestHandler::parseContentLengthValue(const std::string & contentLengthHeaderValue) {
		auto ret = parse_int(contentLengthHeaderValue);

		if(!ret) {
			return ret;
		}

		if(0 > *ret) {
			return {};
		}

		return ret;
	}


	bool RequestHandler::readRequestBody(std::optional<int> contentLength) {
		eqAssert(!contentLength || 0 < *contentLength, "invalid content length (" << (contentLength ? std::to_string(*contentLength) : "[empty]") << ")");
		std::array<char, ReadBufferSize> readBuffer;
		int consecutiveTimeoutCount = 0;
		m_requestBody.clear();

		if(contentLength && m_requestBody.capacity() < static_cast<std::string::size_type>(*contentLength)) {
			m_requestBody.reserve(static_cast<std::string::size_type>(*contentLength));
		}

		while((!contentLength || 0 < *contentLength) && !m_socket->atEnd()) {
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
				if(contentLength) {
					*contentLength -= bytesRead;
				}

				m_requestBody.append(&readBuffer[0], static_cast<std::string::size_type>(bytesRead));
				consecutiveTimeoutCount = 0;
			}
		}

		if(contentLength && 0 < *contentLength) {
			// not enough body data
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket stopped providing data while still expecting " << *contentLength << " bytes (\"" << qPrintable(m_socket->errorString()) << "\")\n";
			return false;
		}

		if(!m_socket->atEnd() || (contentLength && 0 > *contentLength)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: socket provided more body data than expected\n";
		}

		return true;
	}


	std::optional<RequestHandler::HttpRequestLine> RequestHandler::parseHttpRequestLine(const std::string & requestLine) {
		std::smatch captures;

		if(!std::regex_match(requestLine, captures, std::regex("^(OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT) ([^ ]+) HTTP/([0-9](?:\\.[0-9]+)*)$"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request line \"" << requestLine << "\"\n";
			return {};
		}

		return {{captures[1], captures[2], captures[3]}};
	}


	void RequestHandler::run() {
		eqAssert(m_socket, "socket must not be null");

		// scope guard does all cleanup on all exit paths
		ScopeGuard cleanup = [this]() {
			m_socket->flush();
			disposeSocket();
		};

		if(ConnectionPolicy::Accept != determineConnectionPolicy()) {
			sendError(HttpResponseCode::Forbidden);
			return;
		}

		if(auto requestLine = readHeaderLine(*m_socket); !requestLine) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (failed to read request line)\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}
		else if(auto parsedRequestLine = parseHttpRequestLine(*requestLine); !parsedRequestLine) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (failed to parse request line)\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}
		else {
			m_requestLine = std::move(*parsedRequestLine);
		}

		// safe to deref optional without checking because RX in parseHttpRequestLine()
		// guarantees method string is one that parseHttpMethod() will accept
		m_requestMethod = *parseHttpMethod(m_requestLine.method);

		if(!readRequestHeaders()) {
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		std::optional<int> contentLength;

		if(const auto contentLengthIt = m_requestHeaders.find("content-length"); contentLengthIt != m_requestHeaders.cend()) {
			contentLength = parseContentLengthValue(contentLengthIt->second);

			if(!contentLength) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid HTTP request (invalid content-length header)\n";
				sendError(HttpResponseCode::BadRequest);
				return;
			}
		}

		if(!readRequestBody(contentLength)) {
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		handleHttpRequest();
	}


	void RequestHandler::handleHttpRequest() {
		eqAssert(m_socket, "socket must not be null");

		// will accept anything up to HTTP/1.1 and process it as HTTP/1.1
		if("1.0" != m_requestLine.httpVersion && "1.1" != m_requestLine.httpVersion) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: HTTP version (HTTP/" << m_requestLine.httpVersion << ") is not supported\n";
			sendError(HttpResponseCode::HttpVersionNotSupported);
			return;
		}

		// covers the REQUIRED HTTP/1.1 methods (GET, HEAD).
		if(HttpMethod::Get != m_requestMethod && HttpMethod::Head != m_requestMethod && HttpMethod::Post != m_requestMethod) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: Request method " << enumeratorString(m_requestMethod) << " not supported\n";
			sendError(HttpResponseCode::NotImplemented);
			return;
		}

		std::regex rxUri("^([^?#]*)(?:\\?([^#]+))?(?:#(.*))?$");
		std::smatch captures;

		if(!std::regex_match(m_requestLine.uri, captures, rxUri)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed parsing request URI \"" << m_requestLine.uri << "\"\n";
			sendError(HttpResponseCode::BadRequest);
			return;
		}

		// we should never receive a fragment, should we?
		m_requestUri = {percent_decode(captures[1].str()), captures[2], captures[3]};
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
		QFileInfo resource(docRoot.absoluteFilePath() + "/" + QString::fromStdString(m_requestUri.path));  //uri.toLocalFile());
		QString resolvedResourcePath = resource.absoluteFilePath();

		// only serve request from inside doc root
		if(!starts_with(resolvedResourcePath, docRoot.absoluteFilePath())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: requested local resource is outside document root.\n";
			sendError(HttpResponseCode::NotFound);
			return;
		}

		const QString clientAddr = m_socket->peerAddress().toString();
		const uint16_t clientPort = m_socket->peerPort();

		ScopeGuard finishSendingBody = [this]() {
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
			// to fit convention
			suffix = "";
		}

		// NEXTRELEASE support fcgi
		for(const auto & mediaType : m_config.fileExtensionMediaTypes(suffix)) {
			switch(m_config.mediaTypeAction(mediaType)) {
				case WebServerAction::Ignore:
					// just try the next media type for the resource
					break;

				case WebServerAction::Serve:
					sendFile(resolvedResourcePath, mediaType);
					m_stage = ResponseStage::Completed;
					return;

				case WebServerAction::CGI:
					doCgi(resolvedResourcePath, mediaType);
					m_stage = ResponseStage::Completed;
					return;

				case WebServerAction::Forbid:
					Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
					sendError(HttpResponseCode::Forbidden);
					return;
			}
		}

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no action configured for resource \"" << m_requestUri.path << "\", falling back on Forbid (Not found)\n";
		Q_EMIT requestActionTaken(clientAddr, clientPort, QString::fromStdString(m_requestLine.uri), WebServerAction::Forbid);
		sendError(HttpResponseCode::NotFound);
	}


}  // namespace Anansi
