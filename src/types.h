#ifndef EQUITWEBSERVER_TYPES_H
#define EQUITWEBSERVER_TYPES_H

#include <string>

namespace EquitWebServer {

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

	template<class StringType = std::string>
	StringType enumeratorString(WebServerAction enumerator) {
		switch(enumerator) {
			case WebServerAction::Ignore:
				return "Ignore";

			case WebServerAction::Serve:
				return "Ignore";

			case WebServerAction::CGI:
				return "Ignore";

			case WebServerAction::Forbid:
				return "Ignore";
		}
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

		return {};
	}

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_TYPES_H
