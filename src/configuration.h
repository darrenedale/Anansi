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

/// \file configuration.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the Configuration class for Anansi.
///
/// \dep
/// - <cstdint>
/// - <vector>
/// - <map>
/// - <unordered_map>
/// - <optional>
/// - <QString>
/// - types.h
/// - qtstdhash.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_CONFIGURATION_H
#define ANANSI_CONFIGURATION_H

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>

#include <QString>

#include "types.h"
#include "qtstdhash.h"

class QXmlStreamWriter;
class QXmlStreamReader;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace std {
	template<>
	struct hash<QString> : public Equit::QtHash<QString> {};
}  // namespace std
#endif

namespace Anansi {

	class Configuration final {
	public:
		using MediaTypeList = std::vector<QString>;
		using MediaTypeExtensionMap = std::map<QString, MediaTypeList>;
		using MediaTypeActionMap = std::unordered_map<QString, WebServerAction>;
		using MediaTypeCgiMap = std::unordered_map<QString, QString>;
		using IpConnectionPolicyMap = std::unordered_map<QString, ConnectionPolicy>;

		static constexpr const uint16_t DefaultPort = 80;

		Configuration();
		Configuration(const QString & docRoot, const QString & listenAddress, int port);

		static std::optional<Configuration> loadFrom(const QString & fileName);
		bool saveAs(const QString & fileName) const;

		// core server properties
		inline const QString & listenAddress() const {
			return m_listenAddress;
		}

		bool setListenAddress(const QString & listenAddress);

		inline int port() const noexcept {
			return m_listenPort;
		}

		bool setPort(int port) noexcept;

		const QString documentRoot(const QString & platform = QStringLiteral("")) const;
		bool setDocumentRoot(const QString & docRoot, const QString & platform = QStringLiteral(""));

		inline const QString & administratorEmail() const {
			return m_adminEmail;
		}

		inline void setAdministratorEmail(const QString & adminEmail) {
			m_adminEmail = adminEmail;
		}

		inline bool directoryListingsAllowed() const noexcept {
			return m_allowDirectoryListings;
		}

		inline void setDirectoryListingsAllowed(bool allow) noexcept {
			m_allowDirectoryListings = allow;
		}

		inline bool showHiddenFilesInDirectoryListings() const noexcept {
			return m_showHiddenFilesInDirectoryListings;
		}

		inline void setShowHiddenFilesInDirectoryListings(bool show) noexcept {
			m_showHiddenFilesInDirectoryListings = show;
		}

		inline DirectoryListingSortOrder directoryListingSortOrder() const noexcept {
			return m_directoryListingSortOrder;
		}

		inline void setDirectoryListingSortOrder(DirectoryListingSortOrder sortOrder) noexcept {
			m_directoryListingSortOrder = sortOrder;
		}

		QString cgiBin(const QString & platform = QStringLiteral("")) const;
		bool setCgiBin(const QString & bin, const QString & platform = QStringLiteral(""));

		inline int cgiTimeout() const noexcept {
			return m_cgiTimeout;
		}

		inline bool setCgiTimeout(int msec) noexcept {
			if(0 < msec) {
				m_cgiTimeout = msec;
				return true;
			}

			return false;
		}

		// if cgi-bin is inside document root and a request resolves to serving a file from
		// inside cgi-bin, is it actually served? (this is a security leak)
		inline bool allowServingFilesFromCgiBin() const noexcept {
			return m_allowServingFromCgiBin;
		}

		inline void setAllowServingFilesFromCgiBin(bool allow) noexcept {
			m_allowServingFromCgiBin = allow;
		}

		std::vector<QString> registeredIpAddresses() const;
		std::vector<QString> registeredFileExtensions() const;
		std::vector<QString> registeredMediaTypes() const;
		std::vector<QString> allKnownMediaTypes() const;

		inline int registeredIpAddressCount() const noexcept {
			return static_cast<int>(m_ipConnectionPolicies.size());
		}

		inline int registeredFileExtensionCount() const noexcept {
			return static_cast<int>(m_extensionMediaTypes.size());
		}

		inline int registeredMediaTypeCount() const noexcept {
			return static_cast<int>(m_mediaTypeActions.size());
		}

		// connection policies
		inline ConnectionPolicy defaultConnectionPolicy() const noexcept {
			return m_defaultConnectionPolicy;
		}

		inline void setDefaultConnectionPolicy(ConnectionPolicy policy) noexcept {
			m_defaultConnectionPolicy = policy;
		}

		bool ipAddressIsRegistered(const QString & addr) const;
		ConnectionPolicy ipAddressConnectionPolicy(const QString & addr) const;
		bool setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy p);
		bool unsetIpAddressConnectionPolicy(const QString & addr);
		void clearAllIpAddressConnectionPolicies();

		// file type associations
		bool fileExtensionIsRegistered(const QString & ext) const;
		bool fileExtensionHasMediaType(const QString & ext, const QString & mediaType) const;

		bool changeFileExtensionMediaType(const QString & ext, const QString & fromMediaType, const QString & toMediaType);
		bool addFileExtensionMediaType(const QString & ext, const QString & mediaType);
		bool removeFileExtensionMediaType(const QString & ext, const QString & mediaType);
		bool changeFileExtension(const QString & oldExt, const QString & newExt);
		bool removeFileExtension(const QString & ext);

		int fileExtensionMediaTypeCount(const QString & ext) const;
		MediaTypeList fileExtensionMediaTypes(const QString & ext) const;
		void clearAllFileExtensions();

		// default media type
		inline QString defaultMediaType() const {
			return m_defaultMediaType;
		}

		void setDefaultMediaType(const QString & mediaType);
		void unsetDefaultMediaType();

		// actions to take for media types
		bool mediaTypeIsRegistered(const QString & mediaType) const;
		WebServerAction mediaTypeAction(const QString & mediaType) const;
		bool setMediaTypeAction(const QString & mediaType, WebServerAction action);
		bool unsetMediaTypeAction(const QString & mediaType);
		void clearAllMediaTypeActions();

		inline WebServerAction defaultAction() const {
			return m_defaultAction;
		}

		inline void setDefaultAction(WebServerAction action) {
			m_defaultAction = action;
		}

		QString mediaTypeCgi(const QString & mediaType) const;
		bool setMediaTypeCgi(const QString & mediaType, const QString & cgiExe);
		bool unsetMediaTypeCgi(const QString & mediaType);

#if !defined(NDEBUG)
		void dumpFileAssociationMediaTypes();
		void dumpFileAssociationMediaTypes(const QString & ext);
#endif

	private:
		void setDefaults();

		bool readWebserverXml(QXmlStreamReader &);
		bool writeWebserverXml(QXmlStreamWriter &) const;

		bool readDocumentRootXml(QXmlStreamReader &);
		bool readListenAddressXml(QXmlStreamReader &);
		bool readListenPortXml(QXmlStreamReader &);
		bool readCgiBinXml(QXmlStreamReader &);
		bool readAllowServingFilesFromCgiBin(QXmlStreamReader &);
		bool readAdministratorEmailXml(QXmlStreamReader &);
		bool readDefaultConnectionPolicyXml(QXmlStreamReader &);
		bool readDefaultMediaTypeXml(QXmlStreamReader &);
		bool readDefaultActionXml(QXmlStreamReader &);
		bool readAllowDirectoryListingsXml(QXmlStreamReader &);
		bool readShowHiddenFilesInDirectoryListingsXml(QXmlStreamReader &);
		bool readDirectoryListingSortOrderXml(QXmlStreamReader &);
		bool readIpConnectionPoliciesXml(QXmlStreamReader &);
		bool readIpConnectionPolicyXml(QXmlStreamReader &);
		bool readFileExtensionMediaTypesXml(QXmlStreamReader &);
		bool readFileExtensionMediaTypeXml(QXmlStreamReader &);
		bool readMediaTypeActionsXml(QXmlStreamReader &);
		bool readMediaTypeActionXml(QXmlStreamReader &);
		bool readMediaTypeCgiExecutablesXml(QXmlStreamReader &);
		bool readMediaTypeCgiExecutableXml(QXmlStreamReader &);

		bool writeStartXml(QXmlStreamWriter &) const;
		bool writeEndXml(QXmlStreamWriter &) const;
		bool writeDocumentRootXml(QXmlStreamWriter &) const;
		bool writeListenAddressXml(QXmlStreamWriter &) const;
		bool writeListenPortXml(QXmlStreamWriter &) const;
		bool writeCgiBinXml(QXmlStreamWriter &) const;
		bool writeAllowServingFilesFromCgiBinXml(QXmlStreamWriter &) const;
		bool writeAdministratorEmailXml(QXmlStreamWriter &) const;
		bool writeDefaultConnectionPolicyXml(QXmlStreamWriter &) const;
		bool writeDefaultMediaTypeXml(QXmlStreamWriter &) const;
		bool writeAllowDirectoryListingsXml(QXmlStreamWriter &) const;
		bool writeShowHiddenFilesInDirectoryListingsXml(QXmlStreamWriter &) const;
		bool writeDirectoryListingSortOrderXml(QXmlStreamWriter &) const;
		bool writeIpConnectionPoliciesXml(QXmlStreamWriter &) const;
		bool writeFileExtensionMediaTypesXml(QXmlStreamWriter &) const;
		bool writeMediaTypeActionsXml(QXmlStreamWriter &) const;
		bool writeMediaTypeCgiExecutablesXml(QXmlStreamWriter &) const;
		bool writeDefaultActionXml(QXmlStreamWriter &) const;

		QString m_listenAddress;
		int m_listenPort;
		std::unordered_map<QString, QString> m_documentRoot;
		QString m_adminEmail;

		IpConnectionPolicyMap m_ipConnectionPolicies;
		MediaTypeExtensionMap m_extensionMediaTypes;
		MediaTypeActionMap m_mediaTypeActions;
		MediaTypeCgiMap m_mediaTypeCgiExecutables;
		std::unordered_map<QString, QString> m_cgiBin;
		bool m_allowServingFromCgiBin;

		ConnectionPolicy m_defaultConnectionPolicy;
		QString m_defaultMediaType;
		WebServerAction m_defaultAction;
		int m_cgiTimeout;

		bool m_allowDirectoryListings;
		bool m_showHiddenFilesInDirectoryListings;
		DirectoryListingSortOrder m_directoryListingSortOrder;
	};

}  // namespace Anansi

#endif  // ANANSI_CONFIGURATION_H
