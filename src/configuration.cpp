/** \file Configuration.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the Configuration class for EquitWebServer.
  *
  * \todo
  * - decide on application license
  *
  * \par Changes
  * - (2012-06-19) fixed parsing of XML stream where allowdirectorylistings
  *   element would be overlooked.
  * - (2012-06-19) file documentation created.
  */

#include "configuration.h"

#include <iostream>

#include <QDebug>
#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


/*
** lower-case platform strings for use when attempting to preserve paths in config files across platforms
*/
#if defined(Q_OS_LINUX)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "linux"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/Public")
#elif defined(Q_OS_WIN32)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "win32"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_MACX)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "osx"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/Sites")
#elif defined(Q_OS_FREEBSD)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "freebsd"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_OS2)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "os2"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_SOLARIS)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "solaris"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_UNIX)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "unix"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#else
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "undefined"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#endif


namespace EquitWebServer {


	static constexpr const Configuration::WebServerAction InitialDefaultAction = Configuration::Forbid;
	static constexpr const Configuration::ConnectionPolicy InitialDefaultConnectionPolicy = Configuration::AcceptConnection;
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


	bool Configuration::load(const QString & fileName) {
		if(fileName.isEmpty()) {
			return false;
		}

		QFile xmlFile(fileName);

		if(!xmlFile.open(QIODevice::ReadOnly)) {
			return false;
		}

		//bpWebServer::Configuration loading;
		QXmlStreamReader xml(&xmlFile);

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isStartElement()) {
				if(xml.name() == "webserver") {
					parseWebserverXml(xml);
				}
				else {
					xml.readElementText();
				}
			}
		}

		return true;
	}


	bool Configuration::parseWebserverXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "webserver");

		bool ret = true;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: TokenType " << xml.tokenType() << "  ::  StartElement TokenType: " << QXmlStreamReader::StartElement << "\n";
			//		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: parsing XML element \"" << qPrintable(xml.name().toString()) << "\"\n";

			if(xml.name() == "documentroot") {
				ret = parseDocumentRootXml(xml);
			}
			else if(xml.name() == "bindaddress") {
				ret = parseListenAddressXml(xml);
			}
			else if(xml.name() == "bindport") {
				ret = parseListenPortXml(xml);
			}
			else if(xml.name() == "defaultconnectionpolicy") {
				ret = parseDefaultConnectionPolicyXml(xml);
			}
			else if(xml.name() == "defaultmimetype") {
				ret = parseDefaultMIMETypeXml(xml);
			}
			else if(xml.name() == "defaultmimetypeaction") {
				ret = parseDefaultActionXml(xml);
			}
			else if(xml.name() == "ipconnectionpolicylist") {
				ret = parseIPConnectionPoliciesXml(xml);
			}
			else if(xml.name() == "extensionmimetypelist") {
				ret = parseFileExtensionMIMETypesXml(xml);
			}
			else if(xml.name() == "mimetypeactionlist") {
				ret = parseMIMETypeActionsXml(xml);
			}
			else if(xml.name() == "mimetypecgilist") {
				ret = parseMIMETypeCGIExecutablesXml(xml);
			}
			else if(xml.name() == "allowdirectorylistings") {
				ret = parseAllowDirectoryListingsXml(xml);
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return ret;
	}


	void Configuration::parseUnknownElementXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement());
		std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: unknown element \"" << xml.name().toString().toUtf8().constData() << "\"\n";

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace())
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";

				/* ignore extraneous characters */
				continue;
			}

			if(xml.isStartElement()) {
				parseUnknownElementXml(xml);
			}
		}
	}


	bool Configuration::parseDocumentRootXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "documentroot");

		QXmlStreamAttributes attrs = xml.attributes();

		if(attrs.value("platform").isEmpty()) {
			// for legacy compatability, the current platform will be used if it has not been
			// set already from the config file in cases where the config file documentroot
			// element does not have a "platform" attribute. if the current platform is provided
			// with a specific document root later in the config file, the specific one will overwrite
			// the assumed one used here. when writing back out, the platform attribute is always
			// written
			if(m_documentRoot.contains(EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING)) {
				/* just ignore it if the platform docroot is already set */
				xml.readElementText();
				return true;
			}

			attrs.append("platform", EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING);
		}

		setDocumentRoot(xml.readElementText(), attrs.value("platform").toString());
		return true;
	}


	bool Configuration::parseListenAddressXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "bindaddress");

		setListenAddress(xml.readElementText());
		return true;
	}


	bool Configuration::parseListenPortXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "bindport");

		setPort(xml.readElementText().toInt());
		return true;
	}


	bool Configuration::parseDefaultConnectionPolicyXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "defaultconnectionpolicy");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "connectionpolicy") {
				setDefaultConnectionPolicy(parseConnectionPolicyText(xml.readElementText()));
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return true;
	}


	Configuration::ConnectionPolicy Configuration::parseConnectionPolicyText(const QString & policy) {
		if(policy == "RejectConnection") {
			return RejectConnection;
		}
		else if(policy == "AcceptConnection") {
			return AcceptConnection;
		}

		return NoConnectionPolicy;
	}


	bool Configuration::parseDefaultMIMETypeXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "defaultmimetype");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "mimetype") {
				setDefaultMIMEType((xml.readElementText()));
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::parseDefaultActionXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "defaultmimetypeaction");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "webserveraction") {
				setDefaultAction(parseActionText(xml.readElementText()));
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return true;
	}


	Configuration::WebServerAction Configuration::parseActionText(const QString & action) {
		if(action == "Forbid") {
			return WebServerAction::Forbid;
		}
		else if(action == "Serve") {
			return WebServerAction::Serve;
		}
		else if(action == "CGI") {
			return WebServerAction::CGI;
		}

		return WebServerAction::Ignore;
	}


	bool Configuration::parseBooleanText(const QString & boolean, bool def = false) {
		const auto myBoolean = boolean.trimmed().toLower();

		if(myBoolean == "true") {
			return true;
		}
		else if(myBoolean == "false") {
			return false;
		}

		return def;
	}


	bool Configuration::parseAllowDirectoryListingsXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "allowdirectorylistings");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "allowdirectorylistings") {
				setAllowDirectoryListing(parseBooleanText(xml.readElementText(), false));
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::parseIPConnectionPoliciesXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "ipconnectionpolicylist");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "ipconnectionpolicy") {
				parseIPConnectionPolicyXml(xml);
			}
			else {
				parseUnknownElementXml(xml);
			}
		}
		return true;
	}


	bool Configuration::parseIPConnectionPolicyXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "ipconnectionpolicy");

		QString ipAddress, policy;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "ipaddress") {
				ipAddress = xml.readElementText();
			}
			else if(xml.name() == "connectionpolicy") {
				policy = xml.readElementText();
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		setIpAddressPolicy(ipAddress, parseConnectionPolicyText(policy));
		return true;
	}


	bool Configuration::parseFileExtensionMIMETypesXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "extensionmimetypelist");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "extensionmimetype") {
				parseFileExtensionMIMETypeXml(xml);
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::parseFileExtensionMIMETypeXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "extensionmimetype");

		QString ext;
		QStringList mimes;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "extension") {
				ext = xml.readElementText();
			}
			else if(xml.name() == "mimetype") {
				mimes << xml.readElementText();
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		if(0 < mimes.count()) {
			for(const QString & mime : mimes) {
				addFileExtensionMIMEType(ext, mime);
			}
		}

		return true;
	}


	bool Configuration::parseMIMETypeActionsXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypeactionlist");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "mimetypeaction") {
				parseMIMETypeActionXml(xml);
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::parseMIMETypeActionXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypeaction");

		QString mime, action;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "mimetype") {
				mime = xml.readElementText();
			}
			else if(xml.name() == "webserveraction") {
				action = xml.readElementText();
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		setMimeTypeAction(mime, parseActionText(action));
		return true;
	}


	bool Configuration::parseMIMETypeCGIExecutablesXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypecgilist");

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace())
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "mimetypecgi") {
				parseMIMETypeCGIExecutableXml(xml);
			}
			else {
				parseUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::parseMIMETypeCGIExecutableXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypecgi");

		QString mime, exe;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace())
					std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";

				/* ignore extraneous characters */
				continue;
			}

			if(xml.name() == "mimetype") {
				mime = xml.readElementText();
			}
			else if(xml.name() == "cgiexecutable") {
				exe = xml.readElementText();
			}
			else {
				parseUnknownElementXml(xml);
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
		bool ret = startXml(xml) && writeXml(xml) && endXml(xml);
		xmlFile.close();
		return ret;
	}


	bool Configuration::startXml(QXmlStreamWriter & xml) const {
		xml.writeStartDocument();
		xml.writeStartElement(QStringLiteral("webserver"));
		return true;
	}


	bool Configuration::endXml(QXmlStreamWriter & xml) const {
		xml.writeEndElement();
		xml.writeEndDocument();
		return true;
	}


	bool Configuration::writeXml(QXmlStreamWriter & xml) const {
		documentRootXml(xml);
		listenAddressXml(xml);
		listenPortXml(xml);
		defaultConnectionPolicyXml(xml);
		defaultMIMETypeXml(xml);
		defaultActionXml(xml);
		allowDirectoryListingsXml(xml);
		ipConnectionPoliciesXml(xml);
		fileExtensionMIMETypesXml(xml);
		mimeTypeActionsXml(xml);
		mimeTypeCGIExecutablesXml(xml);
		return true;
	}


	bool Configuration::documentRootXml(QXmlStreamWriter & xml) const {
		for(const auto & platform : m_documentRoot.keys()) {
			xml.writeStartElement(QStringLiteral("documentroot"));
			xml.writeAttribute("platform", platform);
			xml.writeCharacters(m_documentRoot[platform]);
			xml.writeEndElement();
		}

		return true;
	}


	bool Configuration::listenAddressXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("bindaddress"));
		xml.writeCharacters(m_listenIP);
		xml.writeEndElement();
		return true;
	}


	bool Configuration::listenPortXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("bindport"));
		xml.writeCharacters(QString::number(m_listenPort));
		xml.writeEndElement();
		return true;
	}


	bool Configuration::defaultConnectionPolicyXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultconnectionpolicy"));
		xml.writeStartElement(QStringLiteral("connectionpolicy"));

		switch(m_defaultConnectionPolicy) {
			case NoConnectionPolicy:
				xml.writeCharacters(QStringLiteral("NoConnectionPolicy"));
				break;

			case RejectConnection:
				xml.writeCharacters(QStringLiteral("RejectConnection"));
				break;

			case AcceptConnection:
				xml.writeCharacters(QStringLiteral("AcceptConnection"));
				break;
		}

		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	bool Configuration::defaultMIMETypeXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultmimetype"));
		xml.writeStartElement(QStringLiteral("mimetype"));
		xml.writeCharacters(m_defaultMIMEType);
		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	bool Configuration::allowDirectoryListingsXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("allowdirectorylistings"));
		xml.writeCharacters(m_allowDirectoryListings ? "true" : "false");
		xml.writeEndElement();
		return true;
	}


	bool Configuration::ipConnectionPoliciesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("ipconnectionpolicylist"));

		for(const auto & ip : m_ipConnectionPolicy) {
			xml.writeStartElement(QStringLiteral("ipconnectionpolicy"));
			xml.writeStartElement(QStringLiteral("ipaddress"));
			xml.writeCharacters(ip.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("connectionpolicy"));

			switch(ip.second) {
				case NoConnectionPolicy:
					xml.writeCharacters(QStringLiteral("NoConnectionPolicy"));
					break;

				case RejectConnection:
					xml.writeCharacters(QStringLiteral("RejectConnection"));
					break;

				case AcceptConnection:
					xml.writeCharacters(QStringLiteral("AcceptConnection"));
					break;
			}

			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::fileExtensionMIMETypesXml(QXmlStreamWriter & xml) const {
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


	bool Configuration::mimeTypeActionsXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("mimetypeactionlist"));

		for(const auto & mime : m_mimeActions) {
			xml.writeStartElement(QStringLiteral("mimetypeaction"));
			xml.writeStartElement(QStringLiteral("mimetype"));
			xml.writeCharacters(mime.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("webserveraction"));

			switch(mime.second) {
				case Ignore:
					xml.writeCharacters(QStringLiteral("Ignore"));
					break;

				case Serve:
					xml.writeCharacters(QStringLiteral("Serve"));
					break;

				case CGI:
					xml.writeCharacters(QStringLiteral("CGI"));
					break;

				case Forbid:
					xml.writeCharacters(QStringLiteral("Forbid"));
					break;
			}

			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::mimeTypeCGIExecutablesXml(QXmlStreamWriter & xml) const {
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


	bool Configuration::defaultActionXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultmimetypeaction"));
		xml.writeStartElement(QStringLiteral("webserveraction"));

		switch(m_defaultAction) {
			case Ignore:
				xml.writeCharacters(QStringLiteral("Ignore"));
				break;

			case Serve:
				xml.writeCharacters(QStringLiteral("Serve"));
				break;

			case CGI:
				xml.writeCharacters(QStringLiteral("CGI"));
				break;

			case Forbid:
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
		if(m_documentRoot.contains(platform)) {
			m_documentRoot[platform] = QString::null;
		}
		else {
			m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING] = QString::null;
		}
	}


	void Configuration::setInvalidListenAddress(void) {
		m_listenIP = QString::null;
	}


	void Configuration::setInvalidListenPort(void) {
		m_listenPort = -1;
	}


	void Configuration::setDefaults(void) {
		m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING] = EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT;
		m_listenIP = DefaultBindAddress;
		m_listenPort = DefaultPort;
		m_cgiTimeout = DefaultCgiTimeout;
		m_allowDirectoryListings = DefaultAllowDirLists;
		m_extensionMIMETypes.clear();
		m_mimeActions.clear();
		m_mimeCgi.clear();
		clearAllIpAddressPolicies();
		setDefaultConnectionPolicy(InitialDefaultConnectionPolicy);

		addFileExtensionMIMEType("html", "text/html");
		addFileExtensionMIMEType("htm", "text/html");
		addFileExtensionMIMEType("shtml", "text/html");

		addFileExtensionMIMEType("css", "text/css");

		addFileExtensionMIMEType("pdf", "application/pdf");

		addFileExtensionMIMEType("js", "application/x-javascript");

		addFileExtensionMIMEType("ico", "image/x-ico");
		addFileExtensionMIMEType("png", "image/png");
		addFileExtensionMIMEType("jpg", "image/jpeg");
		addFileExtensionMIMEType("jpeg", "image/jpeg");
		addFileExtensionMIMEType("gif", "image/gif");
		addFileExtensionMIMEType("bmp", "image/x-bmp");

		setMimeTypeAction("text/html", WebServerAction::Serve);
		setMimeTypeAction("text/css", WebServerAction::Serve);
		setMimeTypeAction("application/pdf", WebServerAction::Serve);
		setMimeTypeAction("application/x-javascript", WebServerAction::Serve);
		setMimeTypeAction("image/png", WebServerAction::Serve);
		setMimeTypeAction("image/jpeg", WebServerAction::Serve);
		setMimeTypeAction("image/gif", WebServerAction::Serve);
		setMimeTypeAction("image/x-ico", WebServerAction::Serve);
		setMimeTypeAction("image/x-bmp", WebServerAction::Serve);

		setDefaultMIMEType("application/octet-stream");
		setDefaultAction(InitialDefaultAction);
	}


	bool Configuration::isValidIPAddress(const QString & addr) {
		static QHostAddress h;
		h.setAddress(addr);
		return !h.isNull();
	}


	const QString & Configuration::listenAddress(void) const {
		return m_listenIP;
	}


	bool Configuration::setListenAddress(const QString & listenAddress) {
		if(isValidIPAddress(listenAddress)) {
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
		if(m_documentRoot.contains(platform)) {
			return m_documentRoot[platform];
		}

		return m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING];
	}


	bool Configuration::setDocumentRoot(const QString & docRoot, const QString & platform) {
		if(platform.isEmpty()) {
			m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING] = docRoot;
		}
		else {
			m_documentRoot[platform] = docRoot;
		}

		return true;
	}


	QStringList Configuration::registeredIPAddressList(void) const {
		QStringList ret;

		std::transform(m_ipConnectionPolicy.cbegin(), m_ipConnectionPolicy.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	QStringList Configuration::registeredFileExtensions(void) const {
		QStringList ret;

		std::transform(m_extensionMIMETypes.cbegin(), m_extensionMIMETypes.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	QStringList Configuration::registeredMIMETypes(void) const {
		QStringList ret;

		std::transform(m_mimeActions.cbegin(), m_mimeActions.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	bool Configuration::addFileExtensionMIMEType(const QString & ext, const QString & mime) {
		QString realExt = ext.trimmed().toLower();
		QString myMime = mime.trimmed();

		if(realExt.isEmpty() || myMime.isEmpty()) {
			qDebug() << "bpWebServer::bpWebServer::Configuration::addFileExtensionMIMEType() - no extension or no MIME type";
			return false;
		}

		const auto & mimeTypesIt = m_extensionMIMETypes.find(realExt);

		if(m_extensionMIMETypes.cend() == mimeTypesIt) {
			m_extensionMIMETypes.emplace(realExt, MimeTypeList({myMime}));
			//			m_extensionMIMETypes[realExt].push_back(realMime);
			return true;
		}
		else {
			auto & mimeTypes = mimeTypesIt->second;
			const auto & end = mimeTypes.cend();
			auto mimeIt = std::find(mimeTypes.cbegin(), end, myMime);

			if(end == mimeIt) {
				mimeTypes.push_back(myMime);
				return true;
			}
		}

		return false;
	}


	void Configuration::removeFileExtensionMIMEType(const QString & ext, const QString & mime) {
		QString myExt = ext.trimmed().toLower();

		if(myExt.isEmpty()) {
			return;
		}

		auto mimeTypesIt = m_extensionMIMETypes.find(myExt);

		if(m_extensionMIMETypes.end() == mimeTypesIt) {
			return;
		}

		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			m_extensionMIMETypes.erase(mimeTypesIt);
		}
		else {
			auto & mimeTypes = mimeTypesIt->second;
			const auto & end = mimeTypes.cend();
			auto mimeIt = std::find(mimeTypes.cbegin(), end, myMime);

			if(mimeIt != end) {
				mimeTypes.erase(mimeIt);
			}
		}
	}


	Configuration::MimeTypeList Configuration::mimeTypesForFileExtension(const QString & ext) const {
		QString myExt = ext.trimmed().toLower();

		if(myExt.isEmpty()) {
			return {};
		}

		const auto mimeTypesIt = m_extensionMIMETypes.find(myExt);

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


	Configuration::WebServerAction Configuration::mimeTypeAction(const QString & mime) const {
		QString myMime = mime.trimmed();

		if(mime.isEmpty()) {
			return Forbid;
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


	Configuration::WebServerAction Configuration::defaultAction(void) const {
		return m_defaultAction;
	}


	void Configuration::setDefaultAction(const WebServerAction & action) {
		m_defaultAction = action;
	}


	QString Configuration::defaultMIMEType(void) const {
		return m_defaultMIMEType;
	}


	void Configuration::setDefaultMIMEType(const QString & mime) {
		m_defaultMIMEType = mime.trimmed().toLower();
	}


	void Configuration::unsetDefaultMIMEType(void) {
		setDefaultMIMEType(QString::null);
	}


	QString Configuration::cgiBin(void) const {
		return m_cgiBin;
	}


	void Configuration::setCgiBin(const QString & bin) {
		/// TODO preprocess the path to ensure it's safe - i.e. no '..' and not absolute
		m_cgiBin = bin;
	}


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


	void Configuration::unsetMIMETypeCGI(const QString & mime) {
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
		if(!isValidIPAddress(addr)) {
			return NoConnectionPolicy;
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
		if(!isValidIPAddress(addr)) {
			return false;
		}

		m_ipConnectionPolicy.insert_or_assign(addr, policy);
		return true;
	}


	bool Configuration::clearIpAddressPolicy(const QString & addr) {
		if(!isValidIPAddress(addr)) {
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


}  // namespace EquitWebServer
