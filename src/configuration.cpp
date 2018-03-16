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

/// \file configuration.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the Configuration class for Anansi.
///
/// \dep
/// - configuration.h
/// - <optional>
/// - <iostream>
/// - <QtGlobal>
/// - <QFile>
/// - <QDir>
/// - <QHostAddress>
/// - <QXmlStreamWriter>
/// - <QXmlStreamReader>
/// - <QXmlStreamAttributes>
/// - assert.h
///
/// \par Changes
/// - (2018-03) First release.

#include "configuration.h"

#include <optional>
#include <iostream>

#include <QtGlobal>
#include <QFile>
#include <QDir>
#include <QStringBuilder>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>

#include "assert.h"


namespace Anansi {


/* lower-case platform strings for use when preserving paths in config files across platforms */
#if defined(Q_OS_LINUX)
	static const QString RuntimePlatformString = QStringLiteral("linux");
	static const QString DefaultDocumentRoot = QDir::homePath() % QStringLiteral("/Public");
#elif defined(Q_OS_WIN32)
	static const QString RuntimePlatformString = QStringLiteral("win32");
	static const QString InitialDocumentRoot = QDir::homePath() % QStringLiteral("/public_html");
#elif defined(Q_OS_MACX)
	static const QString RuntimePlatformString = QStringLiteral("osx");
	static const QString InitialDocumentRoot = QDir::homePath() % QStringLiteral("/Sites");
#elif defined(Q_OS_FREEBSD)
	static const QString RuntimePlatformString = QStringLiteral("freebsd");
	static const QString InitialDocumentRoot = QDir::homePath() % QStringLiteral("/public_html");
#elif defined(Q_OS_OS2)
	static const QString RuntimePlatformString = QStringLiteral("os2");
	static const QString InitialDocumentRoot = QDir::homePath() % QStringLiteral("/public_html");
#elif defined(Q_OS_SOLARIS)
	static const QString RuntimePlatformString = QStringLiteral("solaris");
	static const QString InitialDocumentRoot = QDir::homePath() % QStringLiteral("/public_html");
#elif defined(Q_OS_UNIX)
	static const QString RuntimePlatformString = QStringLiteral("unix");
	static const QString InitialDocumentRoot = QDir::homePath() % QStringLiteral("/public_html");
#else
	static const QString RuntimePlatformString = QStringLiteral("undefined");
	static const QString InitialDocumentRoot = QDir::homePath() % QStringLiteral("/public_html");
#endif


	static constexpr const ConnectionPolicy BuiltInDefaultConnectionPolicy = ConnectionPolicy::Accept;
	static const QString BuiltInDefaultMediaType = QStringLiteral("application/octet-stream");
	static constexpr const WebServerAction BuiltInDefaultAction = WebServerAction::Forbid;
	static constexpr const int DefaultCgiTimeout = 30000;
	static const QString DefaultBindAddress = QStringLiteral("127.0.0.1");
	static constexpr bool DefaultAllowDirLists = true;
	static constexpr const DirectoryListingSortOrder DefaultDirListSortOrder = DirectoryListingSortOrder::AscendingDirectoriesFirst;
	static constexpr bool DefaultAllowServeFromCgiBin = false;
	static constexpr bool DefaultShowHiddenFiles = false;


	static bool isValidIpAddress(const QString & addr) {
		return !QHostAddress(addr).isNull();
	}


	static std::optional<bool> parseBooleanText(const QString & boolean, const std::optional<bool> & def = {}) {
		if(0 == boolean.compare(QStringLiteral("true"), Qt::CaseInsensitive)) {
			return true;
		}
		else if(0 == boolean.compare(QStringLiteral("false"), Qt::CaseInsensitive)) {
			return false;
		}

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid boolean string, returning default \"" << (!def ? "[empty]" : (*def ? "true" : "false")) << "\n";
		return def;
	}


	template<class StringType>
	static std::optional<ConnectionPolicy> parseConnectionPolicyText(const StringType & policy) {
		if(StringType("RejectConnection") == policy || StringType("Reject") == policy) {
			return ConnectionPolicy::Reject;
		}
		else if(StringType("AcceptConnection") == policy || StringType("Accept") == policy) {
			return ConnectionPolicy::Accept;
		}

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid connection policy string\n";
		return {};
	}


	template<class StringType>
	static std::optional<WebServerAction> parseActionText(const StringType & action) {
		if(StringType("Forbid") == action) {
			return WebServerAction::Forbid;
		}

		if(StringType("Serve") == action) {
			return WebServerAction::Serve;
		}

		if(StringType("CGI") == action) {
			return WebServerAction::CGI;
		}

		if(StringType("Ignore") == action) {
			return WebServerAction::Ignore;
		}

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid web server action string\n";
		return {};
	}


	template<class StringType>
	static std::optional<DirectoryListingSortOrder> parseDirectoryListingSortOrder(const StringType & order) {
		if(StringType("AscendingDirectoriesFirst") == order) {
			return DirectoryListingSortOrder::AscendingDirectoriesFirst;
		}

		if(StringType("AscendingFilesFirst") == order) {
			return DirectoryListingSortOrder::AscendingFilesFirst;
		}

		if(StringType("Ascending") == order) {
			return DirectoryListingSortOrder::Ascending;
		}

		if(StringType("DescendingDirectoriesFirst") == order) {
			return DirectoryListingSortOrder::DescendingDirectoriesFirst;
		}

		if(StringType("DescendingFilesFirst") == order) {
			return DirectoryListingSortOrder::DescendingFilesFirst;
		}

		if(StringType("Descending") == order) {
			return DirectoryListingSortOrder::Descending;
		}

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid directory listing sort order string\n";
		return {};
	}


	static void readUnknownElementXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement(), "expecting start element in configuration at line " << xml.lineNumber());
		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: reading and ignoring unknown element \"" << qPrintable(xml.name().toString()) << "\"\n";

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.isStartElement()) {
				readUnknownElementXml(xml);
			}
		}
	}


	Configuration::Configuration() {
		// this call is why there is no member initialisation above
		setDefaults();
	}


	Configuration::Configuration(const QString & docRoot, const QString & listenAddress, int port)
	: Configuration() {
		setDocumentRoot(docRoot);
		setListenAddress(listenAddress);
		setPort(port);
	}


	std::optional<Configuration> Configuration::loadFrom(const QString & fileName) {
		eqAssert(!fileName.isEmpty(), "filename of configuration to load must not be empty");
		QFile xmlFile(fileName);

		if(!xmlFile.open(QIODevice::ReadOnly)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to open file \"" << qPrintable(fileName) << "\"\n";
			return {};
		}

		QXmlStreamReader xml(&xmlFile);
		Configuration config;
		config.m_documentRoot.clear();
		config.m_cgiBin.clear();
		config.m_ipConnectionPolicies.clear();
		config.m_extensionMediaTypes.clear();
		config.m_mediaTypeActions.clear();
		config.m_mediaTypeCgiExecutables.clear();

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isStartElement()) {
				if(xml.name() == QStringLiteral("webserver")) {
					config.readWebserverXml(xml);
				}
				else {
					xml.readElementText();
				}
			}
		}

		return {std::move(config)};
	}


	bool Configuration::readWebserverXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("webserver"), "expecting start element \"webserver\" in configuration at line " << xml.lineNumber());
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

				// ignore extraneous characters
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
			else if(xml.name() == QStringLiteral("cgibin")) {
				ret = readCgiBinXml(xml);
			}
			else if(xml.name() == QStringLiteral("servefromcgibin")) {
				ret = readAllowServingFilesFromCgiBin(xml);
			}
			else if(xml.name() == QStringLiteral("adminemail")) {
				ret = readAdministratorEmailXml(xml);
			}
			else if(xml.name() == QStringLiteral("defaultconnectionpolicy")) {
				ret = readDefaultConnectionPolicyXml(xml);
			}
			else if(xml.name() == QStringLiteral("defaultmediatype") || xml.name() == QStringLiteral("defaultmimetype")) {
				ret = readDefaultMediaTypeXml(xml);
			}
			else if(xml.name() == QStringLiteral("defaultmediatypeaction") || xml.name() == QStringLiteral("defaultmimetypeaction")) {
				ret = readDefaultActionXml(xml);
			}
			else if(xml.name() == QStringLiteral("ipconnectionpolicylist")) {
				ret = readIpConnectionPoliciesXml(xml);
			}
			else if(xml.name() == QStringLiteral("extensionmediatypelist") || xml.name() == QStringLiteral("extensionmimetypelist")) {
				ret = readFileExtensionMediaTypesXml(xml);
			}
			else if(xml.name() == QStringLiteral("mediatypeactionlist") || xml.name() == QStringLiteral("mimetypeactionlist")) {
				ret = readMediaTypeActionsXml(xml);
			}
			else if(xml.name() == QStringLiteral("mediatypecgilist") || xml.name() == QStringLiteral("mimetypecgilist")) {
				ret = readMediaTypeCgiExecutablesXml(xml);
			}
			else if(xml.name() == QStringLiteral("allowdirectorylistings")) {
				ret = readAllowDirectoryListingsXml(xml);
			}
			else if(xml.name() == QStringLiteral("showhiddenfiles")) {
				ret = readShowHiddenFilesInDirectoryListingsXml(xml);
			}
			else if(xml.name() == QStringLiteral("directorylistingsortorder")) {
				ret = readDirectoryListingSortOrderXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return ret;
	}


	bool Configuration::readDocumentRootXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("documentroot"), "expecting start element \"documentroot\" in configuration at line " << xml.lineNumber());
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
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("bindaddress"), "expecting start element \"bindaddress\" in configuration at line " << xml.lineNumber());

		if(!setListenAddress(xml.readElementText())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid listen address on line " << xml.lineNumber() << "\n";
			return false;
		}

		return true;
	}


	bool Configuration::readListenPortXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("bindport"), "expecting start element \"bindport\" in configuration at line " << xml.lineNumber());
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


	bool Configuration::readCgiBinXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("cgibin"), "expecting start element \"cgibin\" in configuration at line " << xml.lineNumber());
		QXmlStreamAttributes attrs = xml.attributes();

		if(!attrs.hasAttribute(QStringLiteral("platform"))) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing required attribute \"platform\" for \"cgibin\" element\n";
			return false;
		}

		auto cgiBinPath = xml.readElementText();

		if(!setCgiBin(cgiBinPath, attrs.value(QStringLiteral("platform")).toString())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: found invalid CGI bin path \"" << qPrintable(cgiBinPath) << "\" in config file\n";
		}

		return true;
	}


	bool Configuration::readAllowServingFilesFromCgiBin(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("servefromcgibin"), "expecting start element \"servefromcgibin\" in configuration at line " << xml.lineNumber());
		auto allow = parseBooleanText(xml.readElementText());

		if(!allow) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid \"servefromcgibin\" element content in XML stream at line " << xml.lineNumber() << " (expecting \"true\" or \"false\")\n";
			return false;
		}

		setAllowServingFilesFromCgiBin(*allow);
		return true;
	}


	bool Configuration::readAdministratorEmailXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("adminemail"), "expecting start element \"adminemail\" in configuration at line " << xml.lineNumber());
		setAdministratorEmail(xml.readElementText());
		return true;
	}


	bool Configuration::readDefaultConnectionPolicyXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("defaultconnectionpolicy"), "expecting start element \"defaultconnectionpolicy\" in configuration at line " << xml.lineNumber());
		std::optional<ConnectionPolicy> policy;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("connectionpolicy")) {
				if(policy) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extra \"connectionpolicy\" element in \"defaultconnectionpolicy\" element in configuration at line " << xml.lineNumber() << "\n";
					return false;
				}

				policy = parseConnectionPolicyText(xml.readElementText());

				if(!policy) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid default connection policy in configuration at line " << xml.lineNumber() << "\n";
					return false;
				}
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(!policy) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"connectionpolicy\" element in \"defaultconnectionpolicy\" element in configuration at line " << xml.lineNumber() << "\n";
			return false;
		}

		setDefaultConnectionPolicy(*policy);
		return true;
	}


	bool Configuration::readDefaultMediaTypeXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("defaultmediatype") || xml.name() == QStringLiteral("defaultmimetype")), "expecting start element \"defaultmediatype\" in configuration at line " << xml.lineNumber());

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("mediatype") || xml.name() == QStringLiteral("mimetype")) {
				setDefaultMediaType((xml.readElementText()));
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readDefaultActionXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("defaultmediatypeaction") || xml.name() == QStringLiteral("defaultmimetypeaction")), "expecting start element \"" << qPrintable(xml.name().toString()) << "\" in XML stream");
		std::optional<WebServerAction> action;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("webserveraction")) {
				if(action) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extra \"webserveraction\" element for \"defaultmediatypeaction\" in XML stream at line " << xml.lineNumber() << "\n";
					return false;
				}

				action = parseActionText(xml.readElementText());

				if(!action) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid web server action text in XML stream at line " << xml.lineNumber() << "\n";
					return false;
				}
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(!action) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"webserveraction\" element for \"defaultmediatypeaction\" in XML stream at line " << xml.lineNumber() << "\n";
			return false;
		}

		return true;
	}


	bool Configuration::readAllowDirectoryListingsXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("allowdirectorylistings"), "expecting start element \"allowdirectorylistings\" in configuration at line " << xml.lineNumber());
		auto allow = parseBooleanText(xml.readElementText());

		if(!allow) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid \"allowdirectorylistings\" element content in XML stream at line " << xml.lineNumber() << " (expecting \"true\" or \"false\")\n";
			return false;
		}

		setDirectoryListingsAllowed(*allow);
		return true;
	}


	bool Configuration::readShowHiddenFilesInDirectoryListingsXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("showhiddenfiles"), "expecting start element \"showhiddenfiles\" in configuration at line " << xml.lineNumber());
		auto show = parseBooleanText(xml.readElementText());

		if(!show) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid \"showhiddenfiles\" element content in XML stream at line " << xml.lineNumber() << " (expecting \"true\" or \"false\")\n";
			return false;
		}

		setShowHiddenFilesInDirectoryListings(*show);
		return true;
	}


	bool Configuration::readDirectoryListingSortOrderXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("directorylistingsortorder"), "expecting start element \"directorylistingsortorder\" in configuration at line " << xml.lineNumber());
		auto sortOrder = parseDirectoryListingSortOrder(xml.readElementText());

		if(!sortOrder) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid \"directorylistingsortorder\" element content in XML stream at line " << xml.lineNumber() << "\n";
			return false;
		}

		setDirectoryListingSortOrder(*sortOrder);
		return true;
	}


	bool Configuration::readIpConnectionPoliciesXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("ipconnectionpolicylist"), "expecting start element \"ipconnectionpolicylist\" in configuration at line " << xml.lineNumber());

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("ipconnectionpolicy")) {
				readIpConnectionPolicyXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}
		return true;
	}


	bool Configuration::readIpConnectionPolicyXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("ipconnectionpolicy"), "invalid XML state: expected start element \"ipconnectionpolicy\"");
		QString addr;
		std::optional<ConnectionPolicy> policy;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("ipaddress")) {
				addr = xml.readElementText();
			}
			else if(xml.name() == "connectionpolicy") {
				if(policy) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extra \"connectionpolicy\" element in \"ipconnectionpolicy\" element in configuration at line " << xml.lineNumber() << "\n";
					return false;
				}

				policy = parseConnectionPolicyText(xml.readElementText());

				if(!policy) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid \"connectionpolicy\" element content in configuration at line " << xml.lineNumber() << "\n";
					return false;
				}
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(!policy) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"connectionpolicy\" element for \"ipconnectionpolicy\" in configuration at line " << xml.lineNumber() << "\n";
			return false;
		}

		if(addr.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing or empty \"ipaddress\" element for \"ipconnectionpolicy\" element in configuration at line " << xml.lineNumber() << "\n";
			return false;
		}

		setIpAddressConnectionPolicy(addr, *policy);
		return true;
	}


	bool Configuration::readFileExtensionMediaTypesXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("extensionmediatypelist") || xml.name() == QStringLiteral("extensionmimetypelist")), "expecting start element \"extensionmediatypelist\" in configuration at line " << xml.lineNumber());

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("extensionmediatype") || xml.name() == QStringLiteral("extensionmimetype")) {
				readFileExtensionMediaTypeXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readFileExtensionMediaTypeXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("extensionmediatype") || xml.name() == QStringLiteral("extensionmimetype")), "expecting start element \"extensionmediatype\" in configuration at line " << xml.lineNumber());
		QString ext;
		std::vector<QString> mediaTypes;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("extension")) {
				ext = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("mediatype") || xml.name() == QStringLiteral("mimetype")) {
				mediaTypes.push_back(xml.readElementText());
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing or empty \"extension\" element for \"extensionmediatype\" element in configuration at line " << xml.lineNumber() << "\n";
			return false;
		}

		for(const QString & mediaType : mediaTypes) {
			addFileExtensionMediaType(ext, mediaType);
		}

		return true;
	}


	bool Configuration::readMediaTypeActionsXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("mediatypeactionlist") || xml.name() == QStringLiteral("mimetypeactionlist")), "expecting start element \"mediatypeactionlist\" in configuration at line " << xml.lineNumber());

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("mediatypeaction") || xml.name() == QStringLiteral("mimetypeaction")) {
				readMediaTypeActionXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMediaTypeActionXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("mediatypeaction") || xml.name() == QStringLiteral("mimetypeaction")), "expecting start element \"mediatypeaction\" in configuration at line " << xml.lineNumber());
		QString mediaType;
		std::optional<WebServerAction> action;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("mediatype") || xml.name() == QStringLiteral("mimetype")) {
				mediaType = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("webserveraction")) {
				if(action) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extra \"webserveraction\" element found for \"mediatypeaction\" at line " << xml.lineNumber() << "\n";
					return false;
				}

				action = parseActionText(xml.readElementText());

				if(!action) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid web server action text in XML stream at line " << xml.lineNumber() << "\n";
					return false;
				}
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(!action) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"webserveraction\" element for \"mediatypeaction\" at line " << xml.lineNumber() << "\n";
			return false;
		}

		if(mediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"mediatype\" element for \"mediatypeaction\" at line " << xml.lineNumber() << "\n";
			return false;
		}

		setMediaTypeAction(mediaType, *action);
		return true;
	}


	bool Configuration::readMediaTypeCgiExecutablesXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("mediatypecgilist") || xml.name() == QStringLiteral("mimetypecgilist")), "expecting start element \"mediatypecgilist\" in configuration at line " << xml.lineNumber());

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace())
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("mediatypecgi") || xml.name() == QStringLiteral("mimetypecgi")) {
				readMediaTypeCgiExecutableXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMediaTypeCgiExecutableXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && (xml.name() == QStringLiteral("mediatypecgi") || xml.name() == QStringLiteral("mimetypecgi")), "expecting start element \"mediatypecgi\" at line " << xml.lineNumber());
		QString mediaType;
		QString cgiExe;

		while(!xml.atEnd()) {
			xml.readNext();

			if(xml.isEndElement()) {
				break;
			}

			if(xml.isCharacters()) {
				if(!xml.isWhitespace()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: ignoring extraneous non-whitespace content at line " << xml.lineNumber() << "\n";
				}

				// ignore extraneous characters
				continue;
			}

			if(xml.name() == QStringLiteral("mediatype") || xml.name() == QStringLiteral("mimetype")) {
				mediaType = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("cgiexecutable")) {
				cgiExe = xml.readElementText();
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(mediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"mediatype\" element for \"mediatypecgi\" at line " << xml.lineNumber() << "\n";
			return false;
		}

		setMediaTypeCgi(mediaType, cgiExe);
		return true;
	}


	bool Configuration::saveAs(const QString & fileName) const {
		eqAssert(!fileName.isEmpty(), "file name must not be empty");
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
		writeCgiBinXml(xml);
		writeAllowServingFilesFromCgiBinXml(xml);
		writeAdministratorEmailXml(xml);
		writeDefaultConnectionPolicyXml(xml);
		writeDefaultMediaTypeXml(xml);
		writeDefaultActionXml(xml);
		writeAllowDirectoryListingsXml(xml);
		writeShowHiddenFilesInDirectoryListingsXml(xml);
		writeDirectoryListingSortOrderXml(xml);
		writeIpConnectionPoliciesXml(xml);
		writeFileExtensionMediaTypesXml(xml);
		writeMediaTypeActionsXml(xml);
		writeMediaTypeCgiExecutablesXml(xml);
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDocumentRootXml(QXmlStreamWriter & xml) const {
		for(const auto & platformDocRoot : m_documentRoot) {
			xml.writeStartElement(QStringLiteral("documentroot"));
			xml.writeAttribute(QStringLiteral("platform"), platformDocRoot.first);
			xml.writeCharacters(platformDocRoot.second);
			xml.writeEndElement();
		}

		return true;
	}


	bool Configuration::writeListenAddressXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("bindaddress"));
		xml.writeCharacters(m_listenAddress);
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeListenPortXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("bindport"));
		xml.writeCharacters(QString::number(m_listenPort));
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeCgiBinXml(QXmlStreamWriter & xml) const {
		for(const auto & platformCgiBin : m_cgiBin) {
			xml.writeStartElement(QStringLiteral("cgibin"));
			xml.writeAttribute(QStringLiteral("platform"), platformCgiBin.first);
			xml.writeCharacters(platformCgiBin.second);
			xml.writeEndElement();
		}

		return true;
	}


	bool Configuration::writeAllowServingFilesFromCgiBinXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("servefromcgibin"));
		xml.writeCharacters(m_allowServingFromCgiBin ? QStringLiteral("true") : QStringLiteral("false"));
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeAdministratorEmailXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("adminemail"));
		xml.writeCharacters(m_adminEmail);
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDefaultConnectionPolicyXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultconnectionpolicy"));
		xml.writeStartElement(QStringLiteral("connectionpolicy"));
		xml.writeCharacters(enumeratorString<QString>(m_defaultConnectionPolicy));
		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDefaultMediaTypeXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultmediatype"));
		xml.writeStartElement(QStringLiteral("mediatype"));
		xml.writeCharacters(m_defaultMediaType);
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


	bool Configuration::writeShowHiddenFilesInDirectoryListingsXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("showhiddenfiles"));
		xml.writeCharacters(m_showHiddenFilesInDirectoryListings ? "true" : "false");
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDirectoryListingSortOrderXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("directorylistingsortorder"));
		xml.writeCharacters(enumeratorString<QString>(m_directoryListingSortOrder));
		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeIpConnectionPoliciesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("ipconnectionpolicylist"));

		for(const auto & ip : m_ipConnectionPolicies) {
			xml.writeStartElement(QStringLiteral("ipconnectionpolicy"));
			xml.writeStartElement(QStringLiteral("ipaddress"));
			xml.writeCharacters(ip.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("connectionpolicy"));
			xml.writeCharacters(enumeratorString<QString>(ip.second));
			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeFileExtensionMediaTypesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("extensionmediatypelist"));

		for(const auto & entry : m_extensionMediaTypes) {
			xml.writeStartElement(QStringLiteral("extensionmediatype"));
			xml.writeStartElement(QStringLiteral("extension"));
			xml.writeCharacters(entry.first);
			xml.writeEndElement();

			for(const auto & mediaType : entry.second) {
				xml.writeStartElement(QStringLiteral("mediatype"));
				xml.writeCharacters(mediaType);
				xml.writeEndElement();
			}

			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeMediaTypeActionsXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("mediatypeactionlist"));

		for(const auto & mediaType : m_mediaTypeActions) {
			xml.writeStartElement(QStringLiteral("mediatypeaction"));
			xml.writeStartElement(QStringLiteral("mediatype"));
			xml.writeCharacters(mediaType.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("webserveraction"));
			xml.writeCharacters(enumeratorString<QString>(mediaType.second));
			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeMediaTypeCgiExecutablesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("mediatypecgilist"));

		for(const auto & mediaType : m_mediaTypeCgiExecutables) {
			xml.writeStartElement(QStringLiteral("mediatypecgi"));
			xml.writeStartElement(QStringLiteral("mediatype"));
			xml.writeCharacters(mediaType.first);
			xml.writeEndElement();
			xml.writeStartElement(QStringLiteral("cgiexecutable"));
			xml.writeCharacters(mediaType.second);
			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeDefaultActionXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultmediatypeaction"));
		xml.writeStartElement(QStringLiteral("webserveraction"));
		xml.writeCharacters(enumeratorString<QString>(m_defaultAction));
		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	void Configuration::setDefaults() {
		m_documentRoot.clear();
		m_cgiBin.clear();
		m_ipConnectionPolicies.clear();
		m_extensionMediaTypes.clear();
		m_mediaTypeActions.clear();
		m_mediaTypeCgiExecutables.clear();

		m_documentRoot.insert({RuntimePlatformString, DefaultDocumentRoot});
		m_listenAddress = DefaultBindAddress;
		m_listenPort = DefaultPort;
		m_defaultConnectionPolicy = BuiltInDefaultConnectionPolicy;
		m_defaultMediaType = BuiltInDefaultMediaType;
		m_defaultAction = BuiltInDefaultAction;
		m_allowDirectoryListings = DefaultAllowDirLists;
		m_showHiddenFilesInDirectoryListings = DefaultShowHiddenFiles;
		m_directoryListingSortOrder = DefaultDirListSortOrder;
		m_cgiTimeout = DefaultCgiTimeout;
		m_allowServingFromCgiBin = DefaultAllowServeFromCgiBin;

		addFileExtensionMediaType(QStringLiteral("html"), QStringLiteral("text/html"));
		addFileExtensionMediaType(QStringLiteral("htm"), QStringLiteral("text/html"));
		addFileExtensionMediaType(QStringLiteral("shtml"), QStringLiteral("text/html"));
		addFileExtensionMediaType(QStringLiteral("css"), QStringLiteral("text/css"));
		addFileExtensionMediaType(QStringLiteral("pdf"), QStringLiteral("application/pdf"));
		addFileExtensionMediaType(QStringLiteral("js"), QStringLiteral("application/x-javascript"));
		addFileExtensionMediaType(QStringLiteral("ico"), QStringLiteral("image/x-ico"));
		addFileExtensionMediaType(QStringLiteral("png"), QStringLiteral("image/png"));
		addFileExtensionMediaType(QStringLiteral("jpg"), QStringLiteral("image/jpeg"));
		addFileExtensionMediaType(QStringLiteral("jpeg"), QStringLiteral("image/jpeg"));
		addFileExtensionMediaType(QStringLiteral("gif"), QStringLiteral("image/gif"));
		addFileExtensionMediaType(QStringLiteral("bmp"), QStringLiteral("image/x-bmp"));

		setMediaTypeAction(QStringLiteral("text/html"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("text/css"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("application/pdf"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("application/x-javascript"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("image/png"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("image/jpeg"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("image/gif"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("image/x-ico"), WebServerAction::Serve);
		setMediaTypeAction(QStringLiteral("image/x-bmp"), WebServerAction::Serve);
	}


	bool Configuration::setListenAddress(const QString & listenAddress) {
		if(!isValidIpAddress(listenAddress)) {
			return false;
		}

		m_listenAddress = listenAddress;
		return true;
	}


	bool Configuration::setPort(int port) noexcept {
		if(port > 0 && port < 65536) {
			m_listenPort = port;
			return true;
		}

		return false;
	}


	const QString Configuration::documentRoot(const QString & platform) const {
		auto docRootIt = m_documentRoot.find(platform);
		const auto & end = m_documentRoot.cend();

		if(end == docRootIt) {
			docRootIt = m_documentRoot.find(RuntimePlatformString);

			if(end == docRootIt) {
				return {};
			}
		}

		return docRootIt->second;
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


	std::vector<QString> Configuration::registeredIpAddresses() const {
		std::vector<QString> ret;

		std::transform(m_ipConnectionPolicies.cbegin(), m_ipConnectionPolicies.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	std::vector<QString> Configuration::registeredFileExtensions() const {
		std::vector<QString> ret;

		std::transform(m_extensionMediaTypes.cbegin(), m_extensionMediaTypes.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	std::vector<QString> Configuration::registeredMediaTypes() const {
		std::vector<QString> ret;

		std::transform(m_mediaTypeActions.cbegin(), m_mediaTypeActions.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	std::vector<QString> Configuration::allKnownMediaTypes() const {
		// use set? or add all to vector then erase dupes? test performance of all three algorithms
		auto ret = registeredMediaTypes();

		std::for_each(m_extensionMediaTypes.cbegin(), m_extensionMediaTypes.cend(), [&ret](const auto & entry) {
			std::copy(entry.second.cbegin(), entry.second.cend(), std::back_inserter(ret));
		});

		std::sort(ret.begin(), ret.end());
		ret.erase(std::unique(ret.begin(), ret.end()), ret.cend());
		return ret;
	}


	bool Configuration::fileExtensionIsRegistered(const QString & ext) const {
		return m_extensionMediaTypes.cend() != m_extensionMediaTypes.find(ext);
	}


	bool Configuration::mediaTypeIsRegistered(const QString & mediaType) const {
		return m_mediaTypeActions.cend() != m_mediaTypeActions.find(mediaType);
	}


	bool Configuration::fileExtensionHasMediaType(const QString & ext, const QString & mediaType) const {
		const auto extIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.cend() == extIt) {
			return false;
		}

		const auto & mediaTypes = extIt->second;
		const auto & end = mediaTypes.cend();
		return std::find(mediaTypes.cbegin(), end, mediaType) != end;
	}


	bool Configuration::changeFileExtensionMediaType(const QString & ext, const QString & fromMediaType, const QString & toMediaType) {
		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no extension\n";
			return false;
		}

		if(fromMediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no media type to change\n";
			return false;
		}

		if(toMediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no new media type\n";
			return false;
		}

		if(fromMediaType == toMediaType) {
			return true;
		}

		const auto & mediaTypesIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.cend() == mediaTypesIt) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extension \"" << qPrintable(ext) << "\" is not registered\n";
			return false;
		}

		auto & mediaTypes = mediaTypesIt->second;
		const auto & begin = mediaTypes.cbegin();
		const auto & end = mediaTypes.cend();
		const auto mediaTypeIt = std::find(begin, end, fromMediaType);

		if(end != mediaTypeIt) {
			mediaTypes[static_cast<std::size_t>(std::distance(begin, mediaTypeIt))] = toMediaType;
			return true;
		}

		std::cerr << "\"" << qPrintable(fromMediaType) << "\" not registered for \"" << qPrintable(ext) << "\"\n";
		return false;
	}


	bool Configuration::addFileExtensionMediaType(const QString & ext, const QString & mediaType) {
		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no extension\n";
			return false;
		}

		if(mediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no media type\n";
			return false;
		}

		const auto & mediaTypesIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.cend() == mediaTypesIt) {
			m_extensionMediaTypes.emplace(ext, MediaTypeList({mediaType}));
			return true;
		}
		else {
			auto & mediaTypes = mediaTypesIt->second;
			const auto & end = mediaTypes.cend();
			const auto mediaTypeIt = std::find(mediaTypes.cbegin(), end, mediaType);

			if(end == mediaTypeIt) {
				mediaTypes.push_back(mediaType);
				return true;
			}
		}

		std::cerr << "\"" << qPrintable(mediaType) << "\" already registered for \"" << qPrintable(ext) << "\"\n";
		return false;
	}


	bool Configuration::removeFileExtensionMediaType(const QString & ext, const QString & mediaType) {
		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't remove media type from empty extension\n";
			return false;
		}

		if(mediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't remove empty media type from \"" << qPrintable(ext) << "\"\n";
			return false;
		}

		auto mediaTypesIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.cend() == mediaTypesIt) {
			return false;
		}

		auto & mediaTypes = mediaTypesIt->second;
		const auto & end = mediaTypes.cend();
		auto mediaTypeIt = std::find(mediaTypes.cbegin(), end, mediaType);

		if(mediaTypeIt == end) {
			return false;
		}

		mediaTypes.erase(mediaTypeIt);
		return true;
	}


	bool Configuration::changeFileExtension(const QString & oldExt, const QString & newExt) {
		if(oldExt.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't change an empty extension\n";
			return false;
		}

		if(newExt.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't change an extension to empty\n";
			return false;
		}

		if(oldExt == newExt) {
			return true;
		}

		const auto end = m_extensionMediaTypes.cend();
		auto extIt = m_extensionMediaTypes.find(newExt);

		if(extIt != end) {
			return false;
		}

		extIt = m_extensionMediaTypes.find(oldExt);

		if(extIt == end) {
			return false;
		}

		m_extensionMediaTypes.emplace(newExt, extIt->second);
		m_extensionMediaTypes.erase(extIt);
		return true;
	}


	bool Configuration::removeFileExtension(const QString & ext) {
		auto extIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.end() == extIt) {
			return false;
		}

		m_extensionMediaTypes.erase(extIt);
		return true;
	}


	int Configuration::fileExtensionMediaTypeCount(const QString & ext) const {
		if(ext.isEmpty()) {
			return 0;
		}

		const auto mediaTypesIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.cend() == mediaTypesIt) {
			return 0;
		}

		return static_cast<int>(mediaTypesIt->second.size());
	}


	Configuration::MediaTypeList Configuration::fileExtensionMediaTypes(const QString & ext) const {
		if(ext.isEmpty()) {
			return {};
		}

		const auto mediaTypesIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.cend() != mediaTypesIt) {
			return mediaTypesIt->second;
		}

		if(m_defaultMediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: there is no default media type specified.\n";
			return {};
		}

		return {m_defaultMediaType};
	}


	void Configuration::clearAllFileExtensions() {
		m_extensionMediaTypes.clear();
	}


	WebServerAction Configuration::mediaTypeAction(const QString & mediaType) const {
		if(mediaType.trimmed().isEmpty()) {
			return WebServerAction::Forbid;
		}

		auto mediaTypeActionIt = m_mediaTypeActions.find(mediaType);

		if(m_mediaTypeActions.cend() != mediaTypeActionIt) {
			return mediaTypeActionIt->second;
		}

		return m_defaultAction;
	}


	bool Configuration::setMediaTypeAction(const QString & mediaType, WebServerAction action) {
		if(mediaType.trimmed().isEmpty()) {
			return false;
		}

		m_mediaTypeActions.insert_or_assign(mediaType, action);
		return true;
	}


	bool Configuration::unsetMediaTypeAction(const QString & mediaType) {
		if(mediaType.trimmed().isEmpty()) {
			return false;
		}

		auto mediaTypeIt = m_mediaTypeActions.find(mediaType);

		if(m_mediaTypeActions.cend() == mediaTypeIt) {
			return false;
		}

		m_mediaTypeActions.erase(mediaTypeIt);
		return true;
	}


	void Configuration::clearAllMediaTypeActions() {
		m_mediaTypeActions.clear();
	}


	void Configuration::setDefaultMediaType(const QString & mediaType) {
		m_defaultMediaType = mediaType.trimmed().toLower();
	}


	void Configuration::unsetDefaultMediaType() {
		setDefaultMediaType(QString::null);
	}


	QString Configuration::cgiBin(const QString & platform) const {
		auto cgiBinIt = m_cgiBin.find(platform);
		const auto & end = m_cgiBin.cend();

		if(end == cgiBinIt) {
			cgiBinIt = m_cgiBin.find(RuntimePlatformString);

			if(end == cgiBinIt) {
				return {};
			}
		}

		return cgiBinIt->second;
	}


	bool Configuration::setCgiBin(const QString & bin, const QString & platform) {
		if(platform.isEmpty()) {
			m_cgiBin.insert_or_assign(RuntimePlatformString, bin);
		}
		else {
			m_cgiBin.insert_or_assign(platform, bin);
		}

		return true;
	}


	QString Configuration::mediaTypeCgi(const QString & mediaType) const {
		if(mediaType.trimmed().isEmpty()) {
			return QString();
		}

		auto mediaTypeIt = m_mediaTypeCgiExecutables.find(mediaType);

		if(m_mediaTypeCgiExecutables.cend() == mediaTypeIt) {
			return {};
		}

		return mediaTypeIt->second;
	}


	bool Configuration::setMediaTypeCgi(const QString & mediaType, const QString & cgiExe) {
		if(cgiExe.trimmed().isEmpty()) {
			return unsetMediaTypeCgi(mediaType);
		}

		if(mediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't set CGI for an empty media type\n";
			return false;
		}

		auto mediaTypeIt = m_mediaTypeCgiExecutables.find(mediaType);

		if(m_mediaTypeCgiExecutables.cend() != mediaTypeIt) {
			mediaTypeIt->second = cgiExe;
		}
		else {
			m_mediaTypeCgiExecutables.insert({mediaType, cgiExe});
		}

		return true;
	}


	bool Configuration::unsetMediaTypeCgi(const QString & mediaType) {
		if(mediaType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't unset CGI for an empty media type\n";
			return false;
		}

		auto mediaTypeIt = m_mediaTypeCgiExecutables.find(mediaType);

		if(m_mediaTypeCgiExecutables.cend() != mediaTypeIt) {
			m_mediaTypeCgiExecutables.erase(mediaType);
		}

		return true;
	}


	bool Configuration::ipAddressIsRegistered(const QString & addr) const {
		return m_ipConnectionPolicies.cend() != m_ipConnectionPolicies.find(addr);
	}


	ConnectionPolicy Configuration::ipAddressConnectionPolicy(const QString & addr) const {
		if(!isValidIpAddress(addr)) {
			return ConnectionPolicy::None;
		}

		auto policyIt = m_ipConnectionPolicies.find(addr);

		if(m_ipConnectionPolicies.cend() != policyIt) {
			return policyIt->second;
		}

		return defaultConnectionPolicy();
	}


	void Configuration::clearAllIpAddressConnectionPolicies() {
		m_ipConnectionPolicies.clear();
	}


	bool Configuration::setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy policy) {
		if(!isValidIpAddress(addr)) {
			return false;
		}

		m_ipConnectionPolicies.insert_or_assign(addr, policy);
		return true;
	}


	bool Configuration::unsetIpAddressConnectionPolicy(const QString & addr) {
		if(!isValidIpAddress(addr)) {
			return false;
		}

		auto policyIt = m_ipConnectionPolicies.find(addr);

		if(m_ipConnectionPolicies.cend() != policyIt) {
			m_ipConnectionPolicies.erase(policyIt);
		}

		return true;
	}


#if !defined(NDEBUG)
	void Configuration::dumpFileAssociationMediaTypes() {
		for(const auto & ext : m_extensionMediaTypes) {
			std::cout << qPrintable(ext.first) << ":\n";

			for(const auto & mediaType : ext.second) {
				std::cout << "   " << qPrintable(mediaType) << "\n";
			}

			std::cout << "\n";
		}

		std::cout << std::flush;
	}


	void Configuration::dumpFileAssociationMediaTypes(const QString & ext) {
		std::cout << qPrintable(ext) << ":\n";
		const auto mediaTypesIt = m_extensionMediaTypes.find(ext);

		if(m_extensionMediaTypes.cend() == mediaTypesIt) {
			std::cout << "   [not found]\n";
		}
		else {
			for(const auto & mediaType : mediaTypesIt->second) {
				std::cout << "   " << qPrintable(mediaType) << "\n";
			}
		}

		std::cout << std::flush;
	}
#endif


}  // namespace Anansi
