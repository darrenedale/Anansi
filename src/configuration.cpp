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

/// \file configuration.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the Configuration class for Anansi..
///
/// \dep
/// - "configuration.h"
/// - <QtGlobal>
/// - <QStringList>
/// - <QFile>
/// - <QDir>
/// - <QHostAddress>
/// - <QXmlStreamWriter>
/// - <QXmlStreamReader>
///
/// \par Changes
/// - (2018-03) First release.

#include "configuration.h"

#include <iostream>

#include <QtGlobal>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


namespace Anansi {


	static bool isValidIpAddress(const QString & addr) {
		return !QHostAddress(addr).isNull();
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


	template<class StringType>
	static ConnectionPolicy parseConnectionPolicyText(const StringType & policy) {
		if(StringType("RejectConnection") == policy || StringType("Reject") == policy) {
			return ConnectionPolicy::Reject;
		}
		else if(StringType("AcceptConnection") == policy || StringType("Accept") == policy) {
			return ConnectionPolicy::Accept;
		}

		return ConnectionPolicy::None;
	}


	template<class StringType>
	static WebServerAction parseActionText(const StringType & action) {
		if(StringType("Forbid") == action) {
			return WebServerAction::Forbid;
		}

		if(StringType("Serve") == action) {
			return WebServerAction::Serve;
		}

		if(StringType("CGI") == action) {
			return WebServerAction::CGI;
		}

		return WebServerAction::Ignore;
	}


	template<class StringType>
	static DirectoryListingSortOrder parseDirectoryListingSortOrder(const StringType & order) {
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

		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid directory listing sort order string, returning default\n";
		return DirectoryListingSortOrder::AscendingDirectoriesFirst;
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

	static constexpr const WebServerAction InitialDefaultAction = WebServerAction::Forbid;
	static constexpr const ConnectionPolicy InitialDefaultConnectionPolicy = ConnectionPolicy::Accept;
	static constexpr const int DefaultCgiTimeout = 30000;
	static constexpr const char * DefaultBindAddress = "127.0.0.1";
	static constexpr bool DefaultAllowDirLists = true;
	static constexpr bool DefaultAllowServeFromCgiBin = false;
	static constexpr bool DefaultShowHiddenFiles = false;


	Configuration::Configuration(void)
	: m_allowServingFromCgiBin(DefaultAllowServeFromCgiBin),
	  m_defaultConnectionPolicy(InitialDefaultConnectionPolicy),
	  m_defaultAction(InitialDefaultAction),
	  m_cgiTimeout(DefaultCgiTimeout),
	  m_allowDirectoryListings(DefaultAllowDirLists) {
		setDefaults();
	}


	Configuration::Configuration(const QString & docRoot, const QString & listenAddress, int port)
	: Configuration() {
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
		m_extensionMimeTypes.clear();
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


	bool Configuration::readCgiBinXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("cgibin"));

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
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("servefromcgibin"));
		setAllowServingFilesFromCgiBin(parseBooleanText(xml.readElementText(), false));
		return true;
	}


	bool Configuration::readAdministratorEmailXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("adminemail"));
		setAdministratorEmail(xml.readElementText());
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


	bool Configuration::readDefaultMimeTypeXml(QXmlStreamReader & xml) {
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
		setDirectoryListingsAllowed(parseBooleanText(xml.readElementText(), false));
		return true;
	}


	bool Configuration::readShowHiddenFilesInDirectoryListingsXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("showhiddenfiles"));
		setShowHiddenFilesInDirectoryListings(parseBooleanText(xml.readElementText(), false));
		return true;
	}

	bool Configuration::readDirectoryListingSortOrderXml(QXmlStreamReader & xml) {
		Q_ASSERT(xml.isStartElement() && xml.name() == QStringLiteral("directorylistingsortorder"));
		setDirectoryListingSortOrder(parseDirectoryListingSortOrder(xml.readElementText()));
		return true;
	}


	bool Configuration::readIpConnectionPoliciesXml(QXmlStreamReader & xml) {
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
				readIpConnectionPolicyXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}
		return true;
	}


	bool Configuration::readIpConnectionPolicyXml(QXmlStreamReader & xml) {
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

		setIpAddressConnectionPolicy(ipAddress, parseConnectionPolicyText(policy));
		return true;
	}


	bool Configuration::readFileExtensionMimeTypesXml(QXmlStreamReader & xml) {
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
				readFileExtensionMimeTypeXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readFileExtensionMimeTypeXml(QXmlStreamReader & xml) {
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


	bool Configuration::readMimeTypeActionsXml(QXmlStreamReader & xml) {
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
				readMimeTypeActionXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMimeTypeActionXml(QXmlStreamReader & xml) {
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


	bool Configuration::readMimeTypeCgiExecutablesXml(QXmlStreamReader & xml) {
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
				readMimeTypeCgiExecutableXml(xml);
			}
			else {
				readUnknownElementXml(xml);
			}
		}

		return true;
	}


	bool Configuration::readMimeTypeCgiExecutableXml(QXmlStreamReader & xml) {
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
			xml.writeAttribute("platform", platformDocRoot.first);
			xml.writeCharacters(platformDocRoot.second);
			xml.writeEndElement();
		}

		return true;
	}


	bool Configuration::writeListenAddressXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("bindaddress"));
		xml.writeCharacters(m_listenIp);
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
			xml.writeAttribute("platform", platformCgiBin.first);
			xml.writeCharacters(platformCgiBin.second);
			xml.writeEndElement();
		}

		return true;
	}


	bool Configuration::writeAllowServingFilesFromCgiBinXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("servefromcgibin"));
		xml.writeCharacters(m_allowServingFromCgiBin ? "true" : "false");
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

		switch(m_directoryListingSortOrder) {
			case DirectoryListingSortOrder::AscendingDirectoriesFirst:
				xml.writeCharacters(QStringLiteral("AscendingDirectoriesFirst"));
				break;

			case DirectoryListingSortOrder::AscendingFilesFirst:
				xml.writeCharacters(QStringLiteral("AscendingFilesFirst"));
				break;

			case DirectoryListingSortOrder::Ascending:
				xml.writeCharacters(QStringLiteral("Ascending"));
				break;

			case DirectoryListingSortOrder::DescendingDirectoriesFirst:
				xml.writeCharacters(QStringLiteral("DescendingDirectoriesFirst"));
				break;

			case DirectoryListingSortOrder::DescendingFilesFirst:
				xml.writeCharacters(QStringLiteral("DescendingFilesFirst"));
				break;

			case DirectoryListingSortOrder::Descending:
				xml.writeCharacters(QStringLiteral("Descending"));
				break;
		}

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


	bool Configuration::writeFileExtensionMimeTypesXml(QXmlStreamWriter & xml) const {
		xml.writeStartElement(QStringLiteral("extensionmimetypelist"));

		//		for(const auto & ext : m_extensionMIMETypes.keys()) {
		for(const auto & entry : m_extensionMimeTypes) {
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


	bool Configuration::writeMimeTypeCgiExecutablesXml(QXmlStreamWriter & xml) const {
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
			m_documentRoot.insert_or_assign(RuntimePlatformString, QString::null);
		}
	}


	void Configuration::setInvalidListenAddress(void) {
		m_listenIp = QString::null;
	}


	void Configuration::setInvalidListenPort(void) {
		m_listenPort = -1;
	}


	void Configuration::setDefaults(void) {
		m_documentRoot.insert({RuntimePlatformString, InitialDocumentRoot});
		m_listenIp = DefaultBindAddress;
		m_listenPort = DefaultPort;
		m_cgiTimeout = DefaultCgiTimeout;
		m_allowServingFromCgiBin = DefaultAllowServeFromCgiBin;
		m_allowDirectoryListings = DefaultAllowDirLists;
		m_showHiddenFilesInDirectoryListings = DefaultShowHiddenFiles;
		m_directoryListingSortOrder = DirectoryListingSortOrder::AscendingDirectoriesFirst;
		m_extensionMimeTypes.clear();
		m_mimeActions.clear();
		m_mimeCgi.clear();
		clearAllIpAddressConnectionPolicies();
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
		return m_listenIp;
	}


	bool Configuration::setListenAddress(const QString & listenAddress) {
		if(isValidIpAddress(listenAddress)) {
			m_listenIp = listenAddress;
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


	std::vector<QString> Configuration::registeredIpAddresses(void) const {
		std::vector<QString> ret;

		std::transform(m_ipConnectionPolicy.cbegin(), m_ipConnectionPolicy.cend(), std::back_inserter(ret), [](const auto & entry) {
			return entry.first;
		});

		return ret;
	}


	std::vector<QString> Configuration::registeredFileExtensions(void) const {
		std::vector<QString> ret;

		std::transform(m_extensionMimeTypes.cbegin(), m_extensionMimeTypes.cend(), std::back_inserter(ret), [](const auto & entry) {
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


	void Configuration::removeFileExtensionMimeType(const QString & ext, const QString & mime) {
		if(ext.isEmpty()) {
			return;
		}

		auto mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.end() == mimeTypesIt) {
			return;
		}

		if(mime.isEmpty()) {
			m_extensionMimeTypes.erase(mimeTypesIt);
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

		const auto end = m_extensionMimeTypes.cend();
		auto extIt = m_extensionMimeTypes.find(newExt);

		if(extIt != end) {
			// new extension already exists
			return false;
		}

		extIt = m_extensionMimeTypes.find(oldExt);

		if(extIt == end) {
			// old extension does not exist
			return false;
		}

		m_extensionMimeTypes.emplace(newExt, extIt->second);
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


	Configuration::MimeTypeList Configuration::mimeTypesForFileExtension(const QString & ext) const {
		if(ext.isEmpty()) {
			return {};
		}

		const auto mimeTypesIt = m_extensionMimeTypes.find(ext);

		if(m_extensionMimeTypes.cend() != mimeTypesIt) {
			return mimeTypesIt->second;
		}

		/* if no defalt MIME type, return an empty vector */
		if(m_defaultMimeType.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: there is no default MIME type specified.\n";
			return {};
		}

		return {m_defaultMimeType};
	}


	void Configuration::clearAllFileExtensions(void) {
		m_extensionMimeTypes.clear();
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
	WebServerAction Configuration::defaultAction(void) const {
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
		return m_defaultMimeType;
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
		m_defaultMimeType = mime.trimmed().toLower();
	}


	/// \brief Unsets the default MIME type.
	///
	/// \see defaultMimeType(), setDefaultMimeType();
	///
	/// This method ensures that resources with unknown MIME types are not served.
	void Configuration::unsetDefaultMimeType(void) {
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
		if(0 < msec) {
			m_cgiTimeout = msec;
			return true;
		}

		return false;
	}


	bool Configuration::allowServingFilesFromCgiBin() const {
		return m_allowServingFromCgiBin;
	}


	void Configuration::setAllowServingFilesFromCgiBin(bool allow) {
		m_allowServingFromCgiBin = allow;
	}


	QString Configuration::administratorEmail(void) const {
		return m_adminEmail;
	}


	void Configuration::setAdministratorEmail(const QString & admin) {
		m_adminEmail = admin;
	}


	ConnectionPolicy Configuration::defaultConnectionPolicy(void) const {
		return m_defaultConnectionPolicy;
	}


	void Configuration::setDefaultConnectionPolicy(ConnectionPolicy p) {
		m_defaultConnectionPolicy = p;
	}


	bool Configuration::ipAddressIsRegistered(const QString & addr) const {
		const auto end = m_ipConnectionPolicy.cend();
		return end != m_ipConnectionPolicy.find(addr);
	}


	ConnectionPolicy Configuration::ipAddressConnectionPolicy(const QString & addr) const {
		if(!isValidIpAddress(addr)) {
			return ConnectionPolicy::None;
		}

		auto policyIt = m_ipConnectionPolicy.find(addr);

		if(m_ipConnectionPolicy.cend() != policyIt) {
			return policyIt->second;
		}

		return defaultConnectionPolicy();
	}


	void Configuration::clearAllIpAddressConnectionPolicies(void) {
		m_ipConnectionPolicy.clear();
	}


	bool Configuration::setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy policy) {
		if(!isValidIpAddress(addr)) {
			return false;
		}

		m_ipConnectionPolicy.insert_or_assign(addr, policy);
		return true;
	}


	bool Configuration::unsetIpAddressConnectionPolicy(const QString & addr) {
		if(!isValidIpAddress(addr)) {
			return false;
		}

		auto policyIt = m_ipConnectionPolicy.find(addr);

		if(m_ipConnectionPolicy.cend() != policyIt) {
			m_ipConnectionPolicy.erase(policyIt);
		}

		return true;
	}


	bool Configuration::directoryListingsAllowed(void) const {
		return m_allowDirectoryListings;
	}


	void Configuration::setDirectoryListingsAllowed(bool allow) {
		m_allowDirectoryListings = allow;
	}


	bool Configuration::showHiddenFilesInDirectoryListings() const {
		return m_showHiddenFilesInDirectoryListings;
	}


	void Configuration::setShowHiddenFilesInDirectoryListings(bool show) {
		m_showHiddenFilesInDirectoryListings = show;
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
