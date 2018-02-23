/// \file Configuration.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date 19th June, 2012
///
/// \brief Implementation of the Configuration class for EquitWebServer.
///
/// \par Changes
/// - (2012-06-19) fixed parsing of XML stream where allowdirectorylistings
///   element would be overlooked.
/// - (2012-06-19) file documentation created.

#include "configuration.h"

#include <iostream>

#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


namespace EquitWebServer {


	static bool isValidIpAddress(const QString & addr) {
		static QHostAddress h;
		h.setAddress(addr);
		return !h.isNull();
	}


	static bool parseBooleanText(const QString & boolean, bool def) {
		if(0 == boolean.compare(QStringLiteral("true"), Qt::CaseInsensitive)) {
			return true;
		}
		else if(0 == boolean.compare(QStringLiteral("false"), Qt::CaseInsensitive)) {
			return false;
		}

		return def;
	}


	static Configuration::ConnectionPolicy parseConnectionPolicyText(const QString & policy) {
		if(QStringLiteral("RejectConnection") == policy || QStringLiteral("Reject") == policy) {
			return Configuration::ConnectionPolicy::Reject;
		}
		else if(QStringLiteral("AcceptConnection") == policy || QStringLiteral("Accept") == policy) {
			return Configuration::ConnectionPolicy::Accept;
		}

		return Configuration::ConnectionPolicy::None;
	}


	static Configuration::WebServerAction parseActionText(const QString & action) {
		if(QStringLiteral("Forbid") == action) {
			return Configuration::WebServerAction::Forbid;
		}
		else if(QStringLiteral("Serve") == action) {
			return Configuration::WebServerAction::Serve;
		}
		else if(QStringLiteral("CGI") == action) {
			return Configuration::WebServerAction::CGI;
		}

		return Configuration::WebServerAction::Ignore;
	}


	static void readUnknownElementXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement());
		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: unknown element \"" << qPrintable(xml.name().toString()) << "\"\n";

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.isStartElement()) {
				readUnknownElementXml(xml);
			}
		}
	}


/* lower-case platform strings for use when preserving paths in config files across platforms */
#if defined(Q_OS_LINUX)
	static constexpr const char * const RuntimePlatformString = "linux";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/Public");
#elif defined(Q_OS_WIN32)
	static constexpr const char * const RuntimePlatformString = "win32";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/public_html");
#elif defined(Q_OS_MACX)
	static constexpr const char * const RuntimePlatformString = "osx";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/Sites");
#elif defined(Q_OS_FREEBSD)
	static constexpr const char * const RuntimePlatformString = "freebsd";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/public_html");
#elif defined(Q_OS_OS2)
	static constexpr const char * const RuntimePlatformString = "os2";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/public_html");
#elif defined(Q_OS_SOLARIS)
	static constexpr const char * const RuntimePlatformString = "solaris";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/public_html");
#elif defined(Q_OS_UNIX)
	static constexpr const char * const RuntimePlatformString = "unix";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/public_html");
#else
	static constexpr const char * const RuntimePlatformString = "undefined";
	static const QString InitialDocumentRoot = (QDir::homePath() + "/public_html");
#endif

	static constexpr const Configuration::WebServerAction InitialDefaultAction = Configuration::WebServerAction::Forbid;
	static constexpr const Configuration::ConnectionPolicy InitialDefaultConnectionPolicy = Configuration::ConnectionPolicy::Accept;
	static constexpr const int DefaultCgiTimeout = 30000;
	static constexpr const char * DefaultBindAddress = "127.0.0.1";
	static constexpr bool DefaultAllowDirLists = true;


	Configuration::Configuration(void)
	: m_defaultConnectionPolicy(InitialDefaultConnectionPolicy),
	  m_defaultAction(InitialDefaultAction),
	  m_cgiTimeout(DefaultCgiTimeout),
	  m_allowDirectoryListings(DefaultAllowDirLists) {
		setDefaults();
	}


	Configuration::Configuration(const QString & docRoot, const QString & listenAddress, int port)
	: m_defaultConnectionPolicy(InitialDefaultConnectionPolicy),
	  m_defaultAction(InitialDefaultAction),
	  m_cgiTimeout(DefaultCgiTimeout),
	  m_allowDirectoryListings(DefaultAllowDirLists) {
		setDefaults();
		setDocumentRoot(docRoot);
		setListenAddress(listenAddress);
		setPort(port);
	}


	std::optional<Configuration> Configuration::loadFrom(const QString & fileName) {
		Configuration config;

		if(!config.read(fileName)) {
			return {};
		}

		return {std::move(config)};
	}


	/// \warning If a call to read() fails, the Configuration object is in an undefined
	/// state.
	bool Configuration::read(const QString & fileName) {
		if(fileName.isEmpty()) {
			return false;
		}

		QFile xmlFile(fileName);

		if(!xmlFile.open(QIODevice::ReadOnly)) {
			return false;
		}

		m_documentRoot.clear();
		m_ipConnectionPolicy.clear();
		m_extensionMIMETypes.clear();
		m_mimeActions.clear();
		m_mimeCgi.clear();

		QXmlStreamReader xml(&xmlFile);

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isStartElement()) {
				if(xml.name() == QStringLiteral("webserver")) {
					readWebserverXml(xml);
				}
				else {
					xml.readElementText();
				}
			}
		}

		return true;
	}


	bool Configuration::readWebserverXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("webserver"));

		bool ret = true;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("documentroot")) {
				ret = readDocumentRootXml(xml);
			}
			else if(xml.name() == QStringLiteral("bindaddress")) {
				ret = readListenAddressXml(xml);
			}
			else if(xml.name() == QStringLiteral("bindport")) {
				ret = readListenPortXml(xml);
			}
			else if(xml.name() == QStringLiteral("defaultconnectionpolicy")) {
				ret = readDefaultConnectionPolicyXml(xml);
			}
			else if(xml.name() == QStringLiteral("defaultmimetype")) {
				ret = readDefaultMIMETypeXml(xml);
			}
			else if(xml.name() == QStringLiteral("defaultmimetypeaction")) {
				ret = readDefaultActionXml(xml);
			}
			else if(xml.name() == QStringLiteral("ipconnectionpolicylist")) {
				ret = readIPConnectionPoliciesXml(xml);
			}
			else if(xml.name() == QStringLiteral("extensionmimetypelist")) {
				ret = readFileExtensionMIMETypesXml(xml);
			}
			else if(xml.name() == QStringLiteral("mimetypeactionlist")) {
				ret = readMIMETypeActionsXml(xml);
			}
			else if(xml.name() == QStringLiteral("mimetypecgilist")) {
				ret = readMIMETypeCGIExecutablesXml(xml);
			}
			else if(xml.name() == QStringLiteral("allowdirectorylistings")) {
				ret = readAllowDirectoryListingsXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return ret;
	}


	bool Configuration::readDocumentRootXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("documentroot"));

		QXmlStreamAttributes attrs = xml.attributes();

		if(attrs.value(QStringLiteral("platform")).isEmpty()) {
			// for legacy compatability, the current platform will be used if it has not been
			// set already from the config file in cases where the config file documentroot
			// element does not have a "platform" attribute. if the current platform is provided
			// with a specific document root later in the config file, the specific one will overwrite
			// the assumed one used here. when writing back out, the platform attribute is always
			// written
			if(m_documentRoot.cend() == m_documentRoot.find(RuntimePlatformString)) {
				/* just ignore it if the platform docroot is already set */
				xml.readElementText();
				return true;
			}

			attrs.push_back({QStringLiteral("platform"), RuntimePlatformString});
		}

		setDocumentRoot(xml.readElementText(), attrs.value(QStringLiteral("platform")).toString());
		return true;
	}


	bool Configuration::readListenAddressXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("bindaddress"));

		if(!setListenAddress(xml.readElementText())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid listen address on line " << xml.lineNumber() << "\n";
			return false;
		}

		return true;
	}


	bool Configuration::readListenPortXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("bindport"));
		bool ok;
		auto port = xml.readElementText().toInt(&ok);

		if(!ok) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid integer string representation for port on line " << xml.lineNumber() << "\n";
			return false;
		}

		if(!setPort(port)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid port " << port << " on line " << xml.lineNumber() << "\n";
			return false;
		}

		return true;
	}


	bool Configuration::readDefaultConnectionPolicyXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("defaultconnectionpolicy"));

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("connectionpolicy")) {
				setDefaultConnectionPolicy(parseConnectionPolicyText(xml.readElementText()));
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readDefaultMIMETypeXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("defaultmimetype"));

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("mimetype")) {
				setDefaultMimeType((xml.readElementText()));
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readDefaultActionXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("defaultmimetypeaction"));

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("webserveraction")) {
				setDefaultAction(parseActionText(xml.readElementText()));
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readAllowDirectoryListingsXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("allowdirectorylistings"));
		setAllowDirectoryListing(parseBooleanText(xml.readElementText(), false));
		return true;
	}


	bool Configuration::readIPConnectionPoliciesXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("ipconnectionpolicylist"));

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("ipconnectionpolicy")) {
				readIPConnectionPolicyXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}
		return true;
	}


	bool Configuration::readIPConnectionPolicyXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("ipconnectionpolicy"));

		QString ipAddress, policy;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("ipaddress")) {
				ipAddress = xml.readElementText();
			}
			else if(xml.name() == "connectionpolicy") {
				policy = xml.readElementText();
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		setIpAddressPolicy(ipAddress, parseConnectionPolicyText(policy));
		return true;
	}


	bool Configuration::readFileExtensionMIMETypesXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("extensionmimetypelist"));

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("extensionmimetype")) {
				readFileExtensionMIMETypeXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readFileExtensionMIMETypeXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("extensionmimetype"));

		QString ext;
		QStringList mimes;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("extension")) {
				ext = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("mimetype")) {
				mimes << xml.readElementText();
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(0 < mimes.count()) {
			for(const QString & mime : mimes) {
				addFileExtensionMimeType(ext, mime);
			}
		}

		return true;
	}


	bool Configuration::readMIMETypeActionsXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("mimetypeactionlist"));

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("mimetypeaction")) {
				readMIMETypeActionXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMIMETypeActionXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("mimetypeaction"));

		QString mime, action;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("mimetype")) {
				mime = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("webserveraction")) {
				action = xml.readElementText();
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		setMimeTypeAction(mime, parseActionText(action));
		return true;
	}


	bool Configuration::readMIMETypeCGIExecutablesXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("mimetypecgilist"));

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace())
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("mimetypecgi")) {
				readMIMETypeCGIExecutableXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMIMETypeCGIExecutableXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("mimetypecgi"));

		QString mime, exe;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == QStringLiteral("mimetype")) {
				mime = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("cgiexecutable")) {
				exe = xml.readElementText();
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		setMimeTypeCgi(mime, exe);
		return true;
	}


	bool Configuration::save(const QString & fileName) const {
		if(fileName.isEmpty()) {
			return false;
		}

		QFile xmlFile(fileName);

		if(!xmlFile.open(QIODevice::WriteOnly)) {
			return false;
		}

		QXmlStreamWriter xml(&xmlFile);
		xml.setAutoFormatting(true);
		bool ret = writeStartXml(xml) && writeWebserverXml(xml) && writeEndXml(xml);
		xmlFile.close();
		return ret;
	}


	bool Configuration::writeStartXml(QXmlStreamWriter & xml) const {
		xml.writeStartDocument();
		return true;
	}


	bool Configuration::writeEndXml(QXmlStreamWriter & xml) const {
		xml.writeEndDocument();
		return true;
	}


	bool Configuration::writeWebserverXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("webserver"));
		writeDocumentRootXml(xml);
		writeListenAddressXml(xml);
		writeListenPortXml(xml);
		writeDefaultConnectionPolicyXml(xml);
		writeDefaultMIMETypeXml(xml);
		writeDefaultActionXml(xml);
		writeAllowDirectoryListingsXml(xml);
		writeIpConnectionPoliciesXml(xml);
		writeFileExtensionMIMETypesXml(xml);
		writeMimeTypeActionsXml(xml);
		writeMimeTypeCGIExecutablesXml(xml);
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDocumentRootXml(QXmlStreamWriter & xml) const {
		for(const auto & platformDocRoot : m_documentRoot) {
			xml.writeStartElement(QStringLiteral("documentroot"));
			xml.writeAttribute("platform", platformDocRoot.first);
			xml.writeCharacters(platformDocRoot.second);
			xml.writeEndElement();
		}

		return true;
	}


	bool Configuration::writeListenAddressXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("bindaddress"));
		xml.writeCharacters(m_listenIP);
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeListenPortXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("bindport"));
		xml.writeCharacters(QString::number(m_listenPort));
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDefaultConnectionPolicyXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultconnectionpolicy"));
		xml.writeStartElement(QStringLiteral("connectionpolicy"));

		switch(m_defaultConnectionPolicy) {
			case ConnectionPolicy::None:
				xml.writeCharacters(QStringLiteral("None"));
				break;

			case ConnectionPolicy::Reject:
				xml.writeCharacters(QStringLiteral("Reject"));
				break;

			case ConnectionPolicy::Accept:
				xml.writeCharacters(QStringLiteral("Accept"));
				break;
		}

		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDefaultMIMETypeXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultmimetype"));
		xml.writeStartElement(QStringLiteral("mimetype"));
		xml.writeCharacters(m_defaultMIMEType);
		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeAllowDirectoryListingsXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("allowdirectorylistings"));
		xml.writeCharacters(m_allowDirectoryListings ? "true" : "false");
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeIpConnectionPoliciesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("ipconnectionpolicylist"));

		for(const auto & ip : m_ipConnectionPolicy) {
			xml.writeStartElement(QStringLiteral("ipconnectionpolicy"));
			xml.writeStartElement(QStringLiteral("ipaddress"));
			xml.writeCharacters(ip.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("connectionpolicy"));

			switch(ip.second) {
				case ConnectionPolicy::None:
					xml.writeCharacters(QStringLiteral("None"));
					break;

				case ConnectionPolicy::Reject:
					xml.writeCharacters(QStringLiteral("Reject"));
					break;

				case ConnectionPolicy::Accept:
					xml.writeCharacters(QStringLiteral("Accept"));
					break;
			}

			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeFileExtensionMIMETypesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("extensionmimetypelist"));

		//		for(const auto & ext : m_extensionMIMETypes.keys()) {
		for(const auto & entry : m_extensionMIMETypes) {
			xml.writeStartElement(QStringLiteral("extensionmimetype"));
			xml.writeStartElement(QStringLiteral("extension"));
			xml.writeCharacters(entry.first);
			xml.writeEndElement();

			//			for(const auto & mime : m_extensionMIMETypes[ext]) {
			for(const auto & mime : entry.second) {
				xml.writeStartElement(QStringLiteral("mimetype"));
				xml.writeCharacters(mime);
				xml.writeEndElement();
			}

			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeMimeTypeActionsXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("mimetypeactionlist"));

		for(const auto & mime : m_mimeActions) {
			xml.writeStartElement(QStringLiteral("mimetypeaction"));
			xml.writeStartElement(QStringLiteral("mimetype"));
			xml.writeCharacters(mime.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("webserveraction"));

			switch(mime.second) {
				case WebServerAction::Ignore:
					xml.writeCharacters(QStringLiteral("Ignore"));
					break;

				case WebServerAction::Serve:
					xml.writeCharacters(QStringLiteral("Serve"));
					break;

				case WebServerAction::CGI:
					xml.writeCharacters(QStringLiteral("CGI"));
					break;

				case WebServerAction::Forbid:
					xml.writeCharacters(QStringLiteral("Forbid"));
					break;
			}

			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeMimeTypeCGIExecutablesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("mimetypecgilist"));

		for(const auto & mime : m_mimeCgi) {
			xml.writeStartElement(QStringLiteral("mimetypecgi"));
			xml.writeStartElement(QStringLiteral("mimetype"));
			xml.writeCharacters(mime.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("cgiexecutable"));
			xml.writeCharacters(mime.second);
			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDefaultActionXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultmimetypeaction"));
		xml.writeStartElement(QStringLiteral("webserveraction"));

		switch(m_defaultAction) {
			case WebServerAction::Ignore:
				xml.writeCharacters(QStringLiteral("Ignore"));
				break;

			case WebServerAction::Serve:
				xml.writeCharacters(QStringLiteral("Serve"));
				break;

			case WebServerAction::CGI:
				xml.writeCharacters(QStringLiteral("CGI"));
				break;

			case WebServerAction::Forbid:
				xml.writeCharacters(QStringLiteral("Forbid"));
				break;
		}

		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	void Configuration::setInvalid(void) {
		setInvalidDocumentRoot();
		setInvalidListenAddress();
		setInvalidListenPort();
	}


	void Configuration::setInvalidDocumentRoot(const QString & platform) {
		auto docRootIt = m_documentRoot.find(platform);

		if(m_documentRoot.cend() != docRootIt) {
			docRootIt->second = QString::null;
		}
		else {
			m_documentRoot.insert({RuntimePlatformString, QString::null});
		}
	}


	void Configuration::setInvalidListenAddress(void) {
		m_listenIP = QString::null;
	}


	void Configuration::setInvalidListenPort(void) {
		m_listenPort = -1;
	}


	void Configuration::setDefaults(void) {
		m_documentRoot.insert({RuntimePlatformString, InitialDocumentRoot});
		m_listenIP = DefaultBindAddress;
		m_listenPort = DefaultPort;
		m_cgiTimeout = DefaultCgiTimeout;
		m_allowDirectoryListings = DefaultAllowDirLists;
		m_extensionMIMETypes.clear();
		m_mimeActions.clear();
		m_mimeCgi.clear();
		clearAllIpAddressPolicies();
		setDefaultConnectionPolicy(InitialDefaultConnectionPolicy);

		addFileExtensionMimeType(QStringLiteral("html"), QStringLiteral("text/html"));
		addFileExtensionMimeType(QStringLiteral("htm"), QStringLiteral("text/html"));
		addFileExtensionMimeType(QStringLiteral("shtml"), QStringLiteral("text/html"));

		addFileExtensionMimeType(QStringLiteral("css"), QStringLiteral("text/css"));

		addFileExtensionMimeType(QStringLiteral("pdf"), QStringLiteral("application/pdf"));

		addFileExtensionMimeType(QStringLiteral("js"), QStringLiteral("application/x-javascript"));

		addFileExtensionMimeType(QStringLiteral("ico"), QStringLiteral("image/x-ico"));
		addFileExtensionMimeType(QStringLiteral("png"), QStringLiteral("image/png"));
		addFileExtensionMimeType(QStringLiteral("jpg"), QStringLiteral("image/jpeg"));
		addFileExtensionMimeType(QStringLiteral("jpeg"), QStringLiteral("image/jpeg"));
		addFileExtensionMimeType(QStringLiteral("gif"), QStringLiteral("image/gif"));
		addFileExtensionMimeType(QStringLiteral("bmp"), QStringLiteral("image/x-bmp"));

		setMimeTypeAction(QStringLiteral("text/html"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("text/css"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("application/pdf"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("application/x-javascript"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("image/png"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("image/jpeg"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("image/gif"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("image/x-ico"), WebServerAction::Serve);
		setMimeTypeAction(QStringLiteral("image/x-bmp"), WebServerAction::Serve);

		setDefaultMimeType(QStringLiteral("application/octet-stream"));
		setDefaultAction(InitialDefaultAction);
	}


	const QString & Configuration::listenAddress(void) const {
		return m_listenIP;
	}


	bool Configuration::setListenAddress(const QString & listenAddress) {
		if(isValidIpAddress(listenAddress)) {
			m_listenIP = listenAddress;
			return true;
		}

		return false;
	}


	int Configuration::port(void) const {
		return m_listenPort;
	}


	bool Configuration::setPort(int port) {
		if(port > 0 && port < 65536) {
			m_listenPort = port;
			return true;
		}

		return false;
	}


	const QString Configuration::documentRoot(const QString & platform) const {
		auto docRootIt = m_documentRoot.find(platform);

		if(m_documentRoot.cend() != docRootIt) {
			return docRootIt->second;
		}

		return m_documentRoot.at(RuntimePlatformString);
	}


	bool Configuration::setDocumentRoot(const QString & docRoot, const QString & platform) {
		if(platform.isEmpty()) {
			m_documentRoot.insert_or_assign(RuntimePlatformString, docRoot);
		}
		else {
			m_documentRoot.insert_or_assign(platform, docRoot);
		}

		return true;
	}


	std::vector<QString> Configuration::registeredIpAddressList(void) const {
		std::vector<QString> ret;

		std::transform(m_ipConnectionPolicy.cbegin(), m_ipConnectionPolicy.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	std::vector<QString> Configuration::registeredFileExtensions(void) const {
		std::vector<QString> ret;

		std::transform(m_extensionMIMETypes.cbegin(), m_extensionMIMETypes.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	/// \brief Gets a list of MIME types with registered actions.
	///
	/// \note The returned list will not include any MIME types associated
	/// with file extensions that do not have specific registered actions.
	///
	/// \return A list of MIME types that have specific registered actions.
	std::vector<QString> Configuration::registeredMimeTypes(void) const {
		std::vector<QString> ret;

		std::transform(m_mimeActions.cbegin(), m_mimeActions.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	bool Configuration::fileExtensionIsRegistered(const QString & ext) const {
		return m_extensionMIMETypes.cend() != m_extensionMIMETypes.find(ext);
	}


	bool Configuration::mimeTypeIsRegistered(const QString & mimeType) const {
		return m_mimeActions.cend() != m_mimeActions.find(mimeType);
	}


	bool Configuration::fileExtensionHasMimeType(const QString & ext, const QString & mime) const {
		const auto extIt = m_extensionMIMETypes.find(ext);

		if(m_extensionMIMETypes.cend() == extIt) {
			return false;
		}

		const auto & mimeTypes = extIt->second;
		const auto & end = mimeTypes.cend();
		return std::find(mimeTypes.cbegin(), end, mime) != end;
	}


	bool Configuration::mimeTypeHasAction(const QString & mime) const {
		const auto & end = m_mimeActions.cend();
		const auto mimeIt = m_mimeActions.find(mime);
		return end != mimeIt;
	}


	bool Configuration::changeFileExtensionMimeType(const QString & ext, const QString & fromMime, const QString & toMime) {
		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no extension\n";
			return false;
		}

		if(fromMime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no MIME type to change\n";
			return false;
		}

		if(toMime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no new MIME type\n";
			return false;
		}

		if(fromMime == toMime) {
			return true;
		}

		const auto & mimeTypesIt = m_extensionMIMETypes.find(ext);

		if(m_extensionMIMETypes.cend() == mimeTypesIt) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extension \"" << qPrintable(ext) << "\" is not registered\n";
			return false;
		}

		auto & mimeTypes = mimeTypesIt->second;
		const auto & begin = mimeTypes.cbegin();
		const auto & end = mimeTypes.cend();
		const auto mimeIt = std::find(begin, end, fromMime);

		if(end != mimeIt) {
			mimeTypes[static_cast<std::size_t>(std::distance(begin, mimeIt))] = toMime;
			return true;
		}

		std::cerr << "\"" << qPrintable(fromMime) << "\" not registered for \"" << qPrintable(ext) << "\"\n";
		return false;
	}


	/// \brief Adds a MIME type for a file extension.
	///
	/// \param ext is the file extension WITHOUT the leading '.'
	/// \param mime is the MIME type.
	///
	/// The only validation carried out is to ensure that neither the extension
	/// nor the MIME type is empty.
	///
	/// \return \c true if a new association was made between the extension and
	/// the MIME type, \c false otherwise. Note that \c false will be returned
	/// if the MIME type is already associated with the extension.
	bool Configuration::addFileExtensionMimeType(const QString & ext, const QString & mime) {
		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no extension\n";
			return false;
		}

		if(mime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no MIME type\n";
			return false;
		}

		const auto & mimeTypesIt = m_extensionMIMETypes.find(ext);

		if(m_extensionMIMETypes.cend() == mimeTypesIt) {
			m_extensionMIMETypes.emplace(ext, MimeTypeList({mime}));
			return true;
		}
		else {
			auto & mimeTypes = mimeTypesIt->second;
			const auto & end = mimeTypes.cend();
			const auto mimeIt = std::find(mimeTypes.cbegin(), end, mime);

			if(end == mimeIt) {
				mimeTypes.push_back(mime);
				return true;
			}
		}

		std::cerr << "\"" << qPrintable(mime) << "\" already registered for \"" << qPrintable(ext) << "\"\n";
		return false;
	}


	void Configuration::removeFileExtensionMimeType(const QString & ext, const QString & mime) {
		if(ext.isEmpty()) {
			return;
		}

		auto mimeTypesIt = m_extensionMIMETypes.find(ext);

		if(m_extensionMIMETypes.end() == mimeTypesIt) {
			return;
		}

		if(mime.isEmpty()) {
			m_extensionMIMETypes.erase(mimeTypesIt);
		}
		else {
			auto & mimeTypes = mimeTypesIt->second;
			const auto & end = mimeTypes.cend();
			auto mimeIt = std::find(mimeTypes.cbegin(), end, mime);

			if(mimeIt != end) {
				std::cerr << "erasing \"" << qPrintable(*mimeIt) << "\"\n";
				mimeTypes.erase(mimeIt);
			}
		}
	}


	bool Configuration::changeFileExtension(const QString & oldExt, const QString & newExt) {
		if(oldExt.isEmpty()) {
			return false;
		}

		if(newExt.isEmpty()) {
			return false;
		}

		if(oldExt == newExt) {
			return false;
		}

		const auto end = m_extensionMIMETypes.cend();
		auto extIt = m_extensionMIMETypes.find(newExt);

		if(extIt != end) {
			// new extension already exists
			return false;
		}

		extIt = m_extensionMIMETypes.find(oldExt);

		if(extIt == end) {
			// old extension does not exist
			return false;
		}

		m_extensionMIMETypes.emplace(newExt, extIt->second);
		m_extensionMIMETypes.erase(extIt);
		return true;
	}


	int Configuration::fileExtensionMimeTypeCount(const QString & ext) const {
		if(ext.isEmpty()) {
			return 0;
		}

		const auto mimeTypesIt = m_extensionMIMETypes.find(ext);

		if(m_extensionMIMETypes.cend() == mimeTypesIt) {
			return 0;
		}

		return static_cast<int>(mimeTypesIt->second.size());
	}


	Configuration::MimeTypeList Configuration::mimeTypesForFileExtension(const QString & ext) const {
		if(ext.isEmpty()) {
			return {};
		}

		const auto mimeTypesIt = m_extensionMIMETypes.find(ext);

		if(m_extensionMIMETypes.cend() != mimeTypesIt) {
			return mimeTypesIt->second;
		}

		/* if no defalt MIME type, return an empty vector */
		if(m_defaultMIMEType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: there is no default MIME type specified.\n";
			return {};
		}

		return {m_defaultMIMEType};
	}


	void Configuration::clearAllFileExtensions(void) {
		m_extensionMIMETypes.clear();
	}


	/// \brief Gets the action configured for a MIME type.
	///
	/// \param mime is the MIME type.
	///
	/// \note If the MIME type provided is empty, the action will always be Forbid.
	/// This is because an empty MIME type is only given for a file extension when
	/// the server is configured not to provide a default MIME type, in other words
	/// when the server is configured not to serve files of types it does not
	/// recognise. To serve files even when the server does not recognise the
	/// extension, set a default MIME type, which will guarantee that all extensions
	/// will resolve to a MIME type.
	///
	/// \return The action associated with the MIME type, or the default action
	/// if no specific action has been defined for the MIME type.
	Configuration::WebServerAction Configuration::mimeTypeAction(const QString & mime) const {
		QString myMime = mime.trimmed();

		if(mime.isEmpty()) {
			return WebServerAction::Forbid;
		}

		auto mimeActionIt = m_mimeActions.find(myMime);

		if(m_mimeActions.cend() != mimeActionIt) {
			return mimeActionIt->second;
		}

		return m_defaultAction;
	}


	bool Configuration::setMimeTypeAction(const QString & mime, const WebServerAction & action) {
		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			return false;
		}

		m_mimeActions.insert_or_assign(myMime, action);
		return true;
	}


	void Configuration::unsetMimeTypeAction(const QString & mime) {
		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			return;
		}

		auto mimeTypeIt = m_mimeActions.find(myMime);

		if(m_mimeActions.cend() != mimeTypeIt) {
			m_mimeActions.erase(mimeTypeIt);
		}
	}


	void Configuration::clearAllMimeTypeActions(void) {
		m_mimeActions.clear();
	}


	/// \brief Gets the default action.
	///
	/// \see setDefaultAction()
	///
	/// \return The default action.
	Configuration::WebServerAction Configuration::defaultAction(void) const {
		return m_defaultAction;
	}


	/// \brief Sets the default action.
	///
	/// \param action is the default action to use.
	///
	/// The default action is given when a MIME type does not have a specific action
	/// attached to it.
	void Configuration::setDefaultAction(const WebServerAction & action) {
		m_defaultAction = action;
	}


	/// \brief Gets the default MIME type.
	///
	/// \see setDefaultMimeType(), unsetDefaultMIMEType();
	///
	/// \return The default MIME type, or an empty string if no default MIME type
	/// is set.
	QString Configuration::defaultMimeType(void) const {
		return m_defaultMIMEType;
	}


	/// \brief Sets the default MIME type.
	///
	/// \param mime is the MIME type to use as the default.
	///
	/// \see getDefaultMimeType(), unsetDefaultMIMEType();
	///
	/// The default MIME type is used when a resource extension cannot be translated
	/// into a MIME type. If it is set to an empty string, no default MIME type will
	/// be used, and resources whose extension is not recognised will not be served.
	void Configuration::setDefaultMimeType(const QString & mime) {
		m_defaultMIMEType = mime.trimmed().toLower();
	}


	/// \brief Unsets the default MIME type.
	///
	/// \see getDefaultMimeType(), setDefaultMIMEType();
	///
	/// This method ensures that resources with unknown MIME types are not served.
	void Configuration::unsetDefaultMimeType(void) {
		setDefaultMimeType(QString::null);
	}


	QString Configuration::cgiBin(void) const {
		return m_cgiBin;
	}


	void Configuration::setCgiBin(const QString & bin) {
		/// TODO preprocess the path to ensure it's safe - i.e. no '..' and not absolute
		m_cgiBin = bin;
	}


	/// \brief Adds a CGI handler for a MIME type.
	///
	/// \param mime is the MIME type for which to add a CGI handler.
	/// \param cgiExe is the executable to use for CGI execution.
	///
	/// Note that this method does not guarantee that a MIME type will be handled
	/// by CGI. The MIME type will only be handled by CGI if the action for that
	/// MIME type is set to \c CGI in setMIMETypeAction().
	///
	/// The execution will always respect the setting for CGIBin. Only executables
	/// found in the directory specified in CGIBin will be used. If the executable
	/// provided to this method is not in that directory, CGI execution will fail at
	/// runtime.
	QString Configuration::mimeTypeCgi(const QString & mime) const {
		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			return QString();
		}

		auto mimeTypeIt = m_mimeCgi.find(myMime);

		if(m_mimeCgi.cend() == mimeTypeIt) {
			return {};
		}

		return mimeTypeIt->second;
	}


	void Configuration::setMimeTypeCgi(const QString & mime, const QString & cgiExe) {
		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			return;
		}

		auto mimeTypeIt = m_mimeCgi.find(myMime);
		QString myCgi = cgiExe.trimmed();

		if(myCgi.isEmpty()) {
			if(m_mimeCgi.cend() != mimeTypeIt) {
				m_mimeCgi.erase(myMime);
			}
		}
		else {
			if(m_mimeCgi.cend() != mimeTypeIt) {
				mimeTypeIt->second = myCgi;
			}
			else {
				m_mimeCgi.insert({myMime, myCgi});
			}
		}
	}


	void Configuration::unsetMimeTypeCgi(const QString & mime) {
		setMimeTypeCgi(mime, QString::null);
	}


	int Configuration::cgiTimeout(void) const {
		return m_cgiTimeout;
	}


	bool Configuration::setCgiTimeout(int msec) {
		if(msec > 0) {
			m_cgiTimeout = msec;
			return true;
		}

		return false;
	}


	QString Configuration::adminEmail(void) const {
		return m_adminEmail;
	}


	void Configuration::setAdminEmail(const QString & admin) {
		m_adminEmail = admin;
	}


	Configuration::ConnectionPolicy Configuration::defaultConnectionPolicy(void) const {
		return m_defaultConnectionPolicy;
	}


	void Configuration::setDefaultConnectionPolicy(Configuration::ConnectionPolicy p) {
		m_defaultConnectionPolicy = p;
	}


	Configuration::ConnectionPolicy Configuration::ipAddressPolicy(const QString & addr) const {
		if(!isValidIpAddress(addr)) {
			return ConnectionPolicy::None;
		}

		auto policyIt = m_ipConnectionPolicy.find(addr);

		if(m_ipConnectionPolicy.cend() != policyIt) {
			return policyIt->second;
		}

		return defaultConnectionPolicy();
	}


	void Configuration::clearAllIpAddressPolicies(void) {
		m_ipConnectionPolicy.clear();
	}


	bool Configuration::setIpAddressPolicy(const QString & addr, ConnectionPolicy policy) {
		if(!isValidIpAddress(addr)) {
			return false;
		}

		m_ipConnectionPolicy.insert_or_assign(addr, policy);
		return true;
	}


	bool Configuration::clearIpAddressPolicy(const QString & addr) {
		if(!isValidIpAddress(addr)) {
			return false;
		}

		auto policyIt = m_ipConnectionPolicy.find(addr);

		if(m_ipConnectionPolicy.cend() != policyIt) {
			m_ipConnectionPolicy.erase(policyIt);
		}

		return true;
	}


	bool Configuration::isDirectoryListingAllowed(void) const {
		return m_allowDirectoryListings;
	}


	void Configuration::setAllowDirectoryListing(bool allow) {
		m_allowDirectoryListings = allow;
	}


#if !defined(NDEBUG)
	void Configuration::dumpFileAssociationMimeTypes() {
		for(const auto & ext : m_extensionMIMETypes) {
			std::cout << qPrintable(ext.first) << ":\n";

			for(const auto & mimeType : ext.second) {
				std::cout << "   " << qPrintable(mimeType) << "\n";
			}

			std::cout << "\n";
		}

		std::cout << std::flush;
	}


	void Configuration::dumpFileAssociationMimeTypes(const QString & ext) {
		std::cout << qPrintable(ext) << ":\n";
		const auto mimeTypesIt = m_extensionMIMETypes.find(ext);

		if(m_extensionMIMETypes.cend() == mimeTypesIt) {
			std::cout << "   [not found]\n";
		}
		else {
			for(const auto & mimeType : mimeTypesIt->second) {
				std::cout << "   " << qPrintable(mimeType) << "\n";
			}
		}

		std::cout << std::flush;
	}
#endif

}  // namespace EquitWebServer
