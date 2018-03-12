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

/// \file types.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Application-wide types for Anansi..
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_TYPES_H
#define ANANSI_TYPES_H

#include <cassert>
#include <string>
#include <unordered_map>

#include <QString>

namespace Anansi {

	enum class WebServerAction {
		Ignore = 0, /* ignore the resource and try the action for the next mime type for a resource extension */
		Serve,		/* serve the content of the resource as-is (i.e. dump its contents to the socket) */
		CGI,			/* attempt to execute the file through CGI */
		Forbid,		/* forbid access to the resource */
	};

	enum class ConnectionPolicy {
		None = 0,
		Reject,
		Accept,
	};

	enum class DirectoryListingSortOrder {
		AscendingDirectoriesFirst = 0,
		AscendingFilesFirst,
		Ascending,
		DescendingDirectoriesFirst,
		DescendingFilesFirst,
		Descending,
	};

	enum class ContentEncoding {
		Identity = 0,
		Deflate,
		Gzip,
	};

	enum class HttpMethod {
		Options,
		Get,
		Head,
		Post,
		Put,
		Delete,
		Trace,
		Connect,
	};

	enum class HttpResponseCode {
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

	template<class StringType = std::string>
	StringType enumeratorString(HttpMethod enumerator) {
		switch(enumerator) {
			case HttpMethod::Options:
				return "Options";

			case HttpMethod::Get:
				return "Get";

			case HttpMethod::Head:
				return "Head";

			case HttpMethod::Post:
				return "Post";

			case HttpMethod::Put:
				return "Put";

			case HttpMethod::Delete:
				return "Delete";

			case HttpMethod::Trace:
				return "Trace";

			case HttpMethod::Connect:
				return "Connect";
		}

		assert(false && "unhandled enumerator value");
		return {};
	}

	template<class StringType = std::string>
	StringType enumeratorString(WebServerAction enumerator) {
		switch(enumerator) {
			case WebServerAction::Ignore:
				return "Ignore";

			case WebServerAction::Serve:
				return "Serve";

			case WebServerAction::CGI:
				return "CGI";

			case WebServerAction::Forbid:
				return "Forbid";
		}

		assert(false && "unhandled enumerator value");
		return {};
	}

	template<class StringType = std::string>
	StringType enumeratorString(ConnectionPolicy enumerator) {
		switch(enumerator) {
			case ConnectionPolicy::None:
				return "None";

			case ConnectionPolicy::Reject:
				return "Reject";

			case ConnectionPolicy::Accept:
				return "Accept";
		}

		assert(false && "unhandled enumerator value");
		return {};
	}

	// NEXTRELEASE headers with the same name are valid, so this should either be a flat map
	// or the value should be updated when parsing/creating a header with a name already
	// present (see RFC2616 sec 4.2)
	using HttpHeaders = std::unordered_map<std::string, std::string>;

}  // namespace Anansi

#endif  // ANANSI_TYPES_H
