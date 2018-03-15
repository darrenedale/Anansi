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
	static const QString BuiltInDefaultMimeType = QStringLiteral("application/octet-stream");
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
		config.m_extensionMimeTypes.clear();
		config.m_mimeActions.clear();
		config.m_mimeCgiExecutables.clear();

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
			else if(xml.name() == QStringLiteral("defaultmimetype")) {
				ret = readDefaultMimeTypeXml(xml);
			}
			else if(xml.name() == QStringLiteral("defaultmimetypeaction")) {
				ret = readDefaultActionXml(xml);
			}
			else if(xml.name() == QStringLiteral("ipconnectionpolicylist")) {
				ret = readIpConnectionPoliciesXml(xml);
			}
			else if(xml.name() == QStringLiteral("extensionmimetypelist")) {
				ret = readFileExtensionMimeTypesXml(xml);
			}
			else if(xml.name() == QStringLiteral("mimetypeactionlist")) {
				ret = readMimeTypeActionsXml(xml);
			}
			else if(xml.name() == QStringLiteral("mimetypecgilist")) {
				ret = readMimeTypeCgiExecutablesXml(xml);
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


	bool Configuration::readDefaultMimeTypeXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("defaultmimetype"), "expecting start element \"defaultmimetype\" in configuration at line " << xml.lineNumber());

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
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("defaultmimetypeaction"), "expecting start element \"" << qPrintable(xml.name().toString()) << "\" in XML stream");
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
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extra \"webserveraction\" element for \"defaultmimetypeaction\" in XML stream at line " << xml.lineNumber() << "\n";
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
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"webserveraction\" element for \"defaultmimetypeaction\" in XML stream at line " << xml.lineNumber() << "\n";
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
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("ipconnectionpolicy"), "invalid XML state: expected start element \"" << qPrintable(xml.name().toString()) << "\"");
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


	bool Configuration::readFileExtensionMimeTypesXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("extensionmimetypelist"), "expecting start element \"extensionmimetypelist\" in configuration at line " << xml.lineNumber());

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

			if(xml.name() == QStringLiteral("extensionmimetype")) {
				readFileExtensionMimeTypeXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readFileExtensionMimeTypeXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("extensionmimetype"), "expecting start element \"extensionmimetype\" in configuration at line " << xml.lineNumber());
		QString ext;
		std::vector<QString> mimes;

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
			else if(xml.name() == QStringLiteral("mimetype")) {
				mimes.push_back(xml.readElementText());
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing or empty \"extension\" element for \"extensionmimetype\" element in configuration at line " << xml.lineNumber() << "\n";
			return false;
		}

		for(const QString & mime : mimes) {
			addFileExtensionMimeType(ext, mime);
		}

		return true;
	}


	bool Configuration::readMimeTypeActionsXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("mimetypeactionlist"), "expecting start element \"mimetypeactionlist\" in configuration at line " << xml.lineNumber());

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

			if(xml.name() == QStringLiteral("mimetypeaction")) {
				readMimeTypeActionXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMimeTypeActionXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("mimetypeaction"), "expecting start element \"mimetypeaction\" in configuration at line " << xml.lineNumber());
		QString mime;
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

			if(xml.name() == QStringLiteral("mimetype")) {
				mime = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("webserveraction")) {
				if(action) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extra \"webserveraction\" element found for \"mimetypeaction\" at line " << xml.lineNumber() << "\n";
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
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"webserveraction\" element for \"mimetypeaction\" at line " << xml.lineNumber() << "\n";
			return false;
		}

		if(mime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"mimetype\" element for \"mimetypeaction\" at line " << xml.lineNumber() << "\n";
			return false;
		}

		setMimeTypeAction(mime, *action);
		return true;
	}


	bool Configuration::readMimeTypeCgiExecutablesXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("mimetypecgilist"), "expecting start element \"mimetypecgilist\" in configuration at line " << xml.lineNumber());

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

			if(xml.name() == QStringLiteral("mimetypecgi")) {
				readMimeTypeCgiExecutableXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMimeTypeCgiExecutableXml(QXmlStreamReader & xml) {
		eqAssert(xml.isStartElement() && xml.name() == QStringLiteral("mimetypecgi"), "expecting start element \"mimetypecgi\" at line " << xml.lineNumber());
		QString mime;
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

			if(xml.name() == QStringLiteral("mimetype")) {
				mime = xml.readElementText();
			}
			else if(xml.name() == QStringLiteral("cgiexecutable")) {
				cgiExe = xml.readElementText();
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		if(mime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: missing \"mimetype\" element for \"mimetypecgi\" at line " << xml.lineNumber() << "\n";
			return false;
		}

		setMimeTypeCgi(mime, cgiExe);
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
		writeDefaultMimeTypeXml(xml);
		writeDefaultActionXml(xml);
		writeAllowDirectoryListingsXml(xml);
		writeShowHiddenFilesInDirectoryListingsXml(xml);
		writeDirectoryListingSortOrderXml(xml);
		writeIpConnectionPoliciesXml(xml);
		writeFileExtensionMimeTypesXml(xml);
		writeMimeTypeActionsXml(xml);
		writeMimeTypeCgiExecutablesXml(xml);
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


	bool Configuration::writeDefaultMimeTypeXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("defaultmimetype"));
		xml.writeStartElement(QStringLiteral("mimetype"));
		xml.writeCharacters(m_defaultMimeType);
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


	bool Configuration::writeFileExtensionMimeTypesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("extensionmimetypelist"));

		for(const auto & entry : m_extensionMimeTypes) {
			xml.writeStartElement(QStringLiteral("extensionmimetype"));
			xml.writeStartElement(QStringLiteral("extension"));
			xml.writeCharacters(entry.first);
			xml.writeEndElement();

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
			xml.writeCharacters(enumeratorString<QString>(mime.second));
			xml.writeEndElement();
			xml.writeEndElement();
		}

		xml.writeEndElement();
		return true;
	}


	bool Configuration::writeMimeTypeCgiExecutablesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("mimetypecgilist"));

		for(const auto & mime : m_mimeCgiExecutables) {
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
		xml.writeCharacters(enumeratorString<QString>(m_defaultAction));
		xml.writeEndElement();
		xml.writeEndElement();
		return true;
	}


	void Configuration::setDefaults() {
		m_documentRoot.clear();
		m_cgiBin.clear();
		m_ipConnectionPolicies.clear();
		m_extensionMimeTypes.clear();
		m_mimeActions.clear();
		m_mimeCgiExecutables.clear();

		m_documentRoot.insert({RuntimePlatformString, DefaultDocumentRoot});
		m_listenAddress = DefaultBindAddress;
		m_listenPort = DefaultPort;
		m_defaultConnectionPolicy = BuiltInDefaultConnectionPolicy;
		m_defaultMimeType = BuiltInDefaultMimeType;
		m_defaultAction = BuiltInDefaultAction;
		m_allowDirectoryListings = DefaultAllowDirLists;
		m_showHiddenFilesInDirectoryListings = DefaultShowHiddenFiles;
		m_directoryListingSortOrder = DefaultDirListSortOrder;
		m_cgiTimeout = DefaultCgiTimeout;
		m_allowServingFromCgiBin = DefaultAllowServeFromCgiBin;

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

		std::transform(m_extensionMimeTypes.cbegin(), m_extensionMimeTypes.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	std::vector<QString> Configuration::registeredMimeTypes() const {
		std::vector<QString> ret;

		std::transform(m_mimeActions.cbegin(), m_mimeActions.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	std::vector<QString> Configuration::allKnownMimeTypes() const {
		// use set? or add all to vector then erase dupes? test performance of all three algorithms
		auto ret = registeredMimeTypes();

		std::for_each(m_extensionMimeTypes.cbegin(), m_extensionMimeTypes.cend(), [&ret](const auto & entry) {
			std::copy(entry.second.cbegin(), entry.second.cend(), std::back_inserter(ret));
		});

		std::sort(ret.begin(), ret.end());
		ret.erase(std::unique(ret.begin(), ret.end()), ret.cend());
		return ret;
	}


	bool Configuration::fileExtensionIsRegistered(const QString & ext) const {
		return m_extensionMimeTypes.cend() != m_extensionMimeTypes.find(ext);
	}


	bool Configuration::mimeTypeIsRegistered(const QString & mimeType) const {
		return m_mimeActions.cend() != m_mimeActions.find(mimeType);
	}


	bool Configuration::fileExtensionHasMimeType(const QString & ext, const QString & mime) const {
		const auto extIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() == extIt) {
			return false;
		}

		const auto & mimeTypes = extIt->second;
		const auto & end = mimeTypes.cend();
		return std::find(mimeTypes.cbegin(), end, mime) != end;
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

		const auto & mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() == mimeTypesIt) {
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


	bool Configuration::addFileExtensionMimeType(const QString & ext, const QString & mime) {
		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no extension\n";
			return false;
		}

		if(mime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no MIME type\n";
			return false;
		}

		const auto & mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() == mimeTypesIt) {
			m_extensionMimeTypes.emplace(ext, MimeTypeList({mime}));
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


	bool Configuration::removeFileExtensionMimeType(const QString & ext, const QString & mime) {
		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't remove media type from empty extension\n";
			return false;
		}

		if(mime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't remove empty media type from \"" << qPrintable(ext) << "\"\n";
			return false;
		}

		auto mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() == mimeTypesIt) {
			return false;
		}

		auto & mimeTypes = mimeTypesIt->second;
		const auto & end = mimeTypes.cend();
		auto mimeIt = std::find(mimeTypes.cbegin(), end, mime);

		if(mimeIt == end) {
			return false;
		}

		mimeTypes.erase(mimeIt);
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

		const auto end = m_extensionMimeTypes.cend();
		auto extIt = m_extensionMimeTypes.find(newExt);

		if(extIt != end) {
			return false;
		}

		extIt = m_extensionMimeTypes.find(oldExt);

		if(extIt == end) {
			return false;
		}

		m_extensionMimeTypes.emplace(newExt, extIt->second);
		m_extensionMimeTypes.erase(extIt);
		return true;
	}


	bool Configuration::removeFileExtension(const QString & ext) {
		auto extIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.end() == extIt) {
			return false;
		}

		m_extensionMimeTypes.erase(extIt);
		return true;
	}


	int Configuration::fileExtensionMimeTypeCount(const QString & ext) const {
		if(ext.isEmpty()) {
			return 0;
		}

		const auto mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() == mimeTypesIt) {
			return 0;
		}

		return static_cast<int>(mimeTypesIt->second.size());
	}


	Configuration::MimeTypeList Configuration::fileExtensionMimeTypes(const QString & ext) const {
		if(ext.isEmpty()) {
			return {};
		}

		const auto mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() != mimeTypesIt) {
			return mimeTypesIt->second;
		}

		if(m_defaultMimeType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: there is no default MIME type specified.\n";
			return {};
		}

		return {m_defaultMimeType};
	}


	void Configuration::clearAllFileExtensions() {
		m_extensionMimeTypes.clear();
	}


	WebServerAction Configuration::mimeTypeAction(const QString & mime) const {
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


	bool Configuration::setMimeTypeAction(const QString & mime, WebServerAction action) {
		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			return false;
		}

		m_mimeActions.insert_or_assign(myMime, action);
		return true;
	}


	bool Configuration::unsetMimeTypeAction(const QString & mime) {
		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			return false;
		}

		auto mimeTypeIt = m_mimeActions.find(myMime);

		if(m_mimeActions.cend() == mimeTypeIt) {
			return false;
		}

		m_mimeActions.erase(mimeTypeIt);
		return true;
	}


	void Configuration::clearAllMimeTypeActions() {
		m_mimeActions.clear();
	}


	void Configuration::setDefaultMimeType(const QString & mime) {
		m_defaultMimeType = mime.trimmed().toLower();
	}


	void Configuration::unsetDefaultMimeType() {
		setDefaultMimeType(QString::null);
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


	QString Configuration::mimeTypeCgi(const QString & mime) const {
		QString myMime = mime.trimmed();

		if(myMime.isEmpty()) {
			return QString();
		}

		auto mimeTypeIt = m_mimeCgiExecutables.find(myMime);

		if(m_mimeCgiExecutables.cend() == mimeTypeIt) {
			return {};
		}

		return mimeTypeIt->second;
	}


	bool Configuration::setMimeTypeCgi(const QString & mime, const QString & cgiExe) {
		if(cgiExe.trimmed().isEmpty()) {
			return unsetMimeTypeCgi(mime);
		}

		if(mime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't set CGI for an empty media type\n";
			return false;
		}

		auto mimeTypeIt = m_mimeCgiExecutables.find(mime);

		if(m_mimeCgiExecutables.cend() != mimeTypeIt) {
			mimeTypeIt->second = cgiExe;
		}
		else {
			m_mimeCgiExecutables.insert({mime, cgiExe});
		}

		return true;
	}


	bool Configuration::unsetMimeTypeCgi(const QString & mime) {
		if(mime.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't unset CGI for an empty media type\n";
			return false;
		}

		auto mimeTypeIt = m_mimeCgiExecutables.find(mime);

		if(m_mimeCgiExecutables.cend() != mimeTypeIt) {
			m_mimeCgiExecutables.erase(mime);
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
	void Configuration::dumpFileAssociationMimeTypes() {
		for(const auto & ext : m_extensionMimeTypes) {
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
		const auto mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() == mimeTypesIt) {
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


}  // namespace Anansi
