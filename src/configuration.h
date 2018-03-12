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

namespace std {
	template<>
	struct hash<QString> : public Equit::QtHash<QString> {};
}  // namespace std

namespace Anansi {

	class Configuration final {
	public:
		/// A list of MIME types
		using MimeTypeList = std::vector<QString>;

		/// Maps a file extension to list of MIME types.
		// uses ordered map so that the model can reliably use row indices
		using MimeTypeExtensionMap = std::map<QString, MimeTypeList>;

		/// Maps a MIME type to an action to take
		using MimeTypeActionMap = std::unordered_map<QString, WebServerAction>;

		/// Maps MIME type to a CGI script
		using MimeTypeCgiMap = std::unordered_map<QString, QString>;

		/// Maps an IP address to a connection policy
		using IpConnectionPolicyMap = std::unordered_map<QString, ConnectionPolicy>;

		static constexpr const uint16_t DefaultPort = 80;

		Configuration();
		Configuration(const QString & docRoot, const QString & listenAddress, int port);

		static std::optional<Configuration> loadFrom(const QString & fileName);
		bool save(const QString & fileName) const;
		bool read(const QString & fileName);

		// core server properties
		const QString & listenAddress() const;
		bool setListenAddress(const QString & listenAddress);

		int port() const noexcept;
		bool setPort(int port) noexcept;

		const QString documentRoot(const QString & platform = QStringLiteral()) const;
		bool setDocumentRoot(const QString & docRoot, const QString & platform = QStringLiteral());

		QString administratorEmail() const;
		void setAdministratorEmail(const QString & admin);

		bool directoryListingsAllowed() const noexcept;
		void setDirectoryListingsAllowed(bool) noexcept;

		bool showHiddenFilesInDirectoryListings() const noexcept;
		void setShowHiddenFilesInDirectoryListings(bool) noexcept;

		inline DirectoryListingSortOrder directoryListingSortOrder() const noexcept {
			return m_directoryListingSortOrder;
		}

		inline void setDirectoryListingSortOrder(DirectoryListingSortOrder sortOrder) noexcept {
			m_directoryListingSortOrder = sortOrder;
		}

		QString cgiBin(const QString & platform = QStringLiteral()) const;
		bool setCgiBin(const QString & bin, const QString & platform = QStringLiteral());

		int cgiTimeout() const noexcept;
		bool setCgiTimeout(int) noexcept;

		// if cgi-bin is inside document root and a request resolves to serving a file from
		// inside cgi-bin, do we allow it? (this is often a security leak)
		bool allowServingFilesFromCgiBin() const noexcept;
		void setAllowServingFilesFromCgiBin(bool allow) noexcept;

		std::vector<QString> registeredIpAddresses() const;
		std::vector<QString> registeredFileExtensions() const;
		std::vector<QString> registeredMimeTypes() const;
		std::vector<QString> allKnownMimeTypes() const;

		inline int registeredIpAddressCount() const noexcept {
			return static_cast<int>(m_ipConnectionPolicies.size());
		}

		inline int registeredFileExtensionCount() const noexcept {
			return static_cast<int>(m_extensionMimeTypes.size());
		}

		inline int registeredMimeTypeCount() const noexcept {
			return static_cast<int>(m_mimeActions.size());
		}

		// connection policies
		ConnectionPolicy defaultConnectionPolicy() const noexcept;
		void setDefaultConnectionPolicy(ConnectionPolicy) noexcept;

		bool ipAddressIsRegistered(const QString & addr) const;
		ConnectionPolicy ipAddressConnectionPolicy(const QString & addr) const;
		bool setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy p);
		bool unsetIpAddressConnectionPolicy(const QString & addr);
		void clearAllIpAddressConnectionPolicies();

		// file type associations
		bool fileExtensionIsRegistered(const QString & ext) const;
		bool fileExtensionHasMimeType(const QString & ext, const QString & mime) const;

		bool changeFileExtensionMimeType(const QString & ext, const QString & fromMime, const QString & toMime);
		bool addFileExtensionMimeType(const QString & ext, const QString & mime);
		void removeFileExtensionMimeType(const QString & ext, const QString & mime);

		bool changeFileExtension(const QString & oldExt, const QString & newExt);

		inline void removeFileExtension(const QString & ext) {
			removeFileExtensionMimeType(ext, QString::null);
		}

		int fileExtensionMimeTypeCount(const QString & ext) const;
		MimeTypeList mimeTypesForFileExtension(const QString & ext) const;
		void clearAllFileExtensions();

		// default MIME type
		QString defaultMimeType() const;
		void setDefaultMimeType(const QString & mime);
		void unsetDefaultMimeType();

		// actions to take for MIME types
		bool mimeTypeIsRegistered(const QString & mime) const;
		WebServerAction mimeTypeAction(const QString & mime) const;
		bool setMimeTypeAction(const QString & mime, const WebServerAction & action);
		void unsetMimeTypeAction(const QString & mime);
		void clearAllMimeTypeActions();

		WebServerAction defaultAction() const;
		void setDefaultAction(const WebServerAction & action);

		QString mimeTypeCgi(const QString & mime) const;
		void setMimeTypeCgi(const QString & mime, const QString & cgiExe);
		void unsetMimeTypeCgi(const QString & mime);

#if !defined(NDEBUG)
		void dumpFileAssociationMimeTypes();
		void dumpFileAssociationMimeTypes(const QString & ext);
#endif

	private:
		bool readWebserverXml(QXmlStreamReader &);
		bool writeWebserverXml(QXmlStreamWriter &) const;

		bool readDocumentRootXml(QXmlStreamReader &);
		bool readListenAddressXml(QXmlStreamReader &);
		bool readListenPortXml(QXmlStreamReader &);
		bool readCgiBinXml(QXmlStreamReader &);
		bool readAllowServingFilesFromCgiBin(QXmlStreamReader &);
		bool readAdministratorEmailXml(QXmlStreamReader &);
		bool readDefaultConnectionPolicyXml(QXmlStreamReader &);
		bool readDefaultMimeTypeXml(QXmlStreamReader &);
		bool readDefaultActionXml(QXmlStreamReader &);
		bool readAllowDirectoryListingsXml(QXmlStreamReader &);
		bool readShowHiddenFilesInDirectoryListingsXml(QXmlStreamReader &);
		bool readDirectoryListingSortOrderXml(QXmlStreamReader &);
		bool readIpConnectionPoliciesXml(QXmlStreamReader &);
		bool readIpConnectionPolicyXml(QXmlStreamReader &);
		bool readFileExtensionMimeTypesXml(QXmlStreamReader &);
		bool readFileExtensionMimeTypeXml(QXmlStreamReader &);
		bool readMimeTypeActionsXml(QXmlStreamReader &);
		bool readMimeTypeActionXml(QXmlStreamReader &);
		bool readMimeTypeCgiExecutablesXml(QXmlStreamReader &);
		bool readMimeTypeCgiExecutableXml(QXmlStreamReader &);

		bool writeStartXml(QXmlStreamWriter &) const;
		bool writeEndXml(QXmlStreamWriter &) const;
		bool writeDocumentRootXml(QXmlStreamWriter &) const;
		bool writeListenAddressXml(QXmlStreamWriter &) const;
		bool writeListenPortXml(QXmlStreamWriter &) const;
		bool writeCgiBinXml(QXmlStreamWriter &) const;
		bool writeAllowServingFilesFromCgiBinXml(QXmlStreamWriter &) const;
		bool writeAdministratorEmailXml(QXmlStreamWriter &) const;
		bool writeDefaultConnectionPolicyXml(QXmlStreamWriter &) const;
		bool writeDefaultMimeTypeXml(QXmlStreamWriter &) const;
		bool writeAllowDirectoryListingsXml(QXmlStreamWriter &) const;
		bool writeShowHiddenFilesInDirectoryListingsXml(QXmlStreamWriter &) const;
		bool writeDirectoryListingSortOrderXml(QXmlStreamWriter &) const;
		bool writeIpConnectionPoliciesXml(QXmlStreamWriter &) const;
		bool writeFileExtensionMimeTypesXml(QXmlStreamWriter &) const;
		bool writeMimeTypeActionsXml(QXmlStreamWriter &) const;
		bool writeMimeTypeCgiExecutablesXml(QXmlStreamWriter &) const;
		bool writeDefaultActionXml(QXmlStreamWriter &) const;

		void setDefaults();

		void setInvalid();																///< invalidate all options
		void setInvalidDocumentRoot(const QString & = QStringLiteral());  ///< invalidate the document root. prevents use of default doc root when invalid path is used to construct
		void setInvalidListenAddress();												///< invalidate the listen address. prevents use of default address when invalid address is used to construct
		void setInvalidListenPort();													///< invalidate the listen port. prevents use of default port when invalid port is used to construct

		QString m_listenIp;
		int m_listenPort;
		std::unordered_map<QString, QString> m_documentRoot;  ///< Maps document root per platform. Enables sharing of configs between platforms with only platform-specific items like paths not being shared.
		QString m_adminEmail;											///< The email address of the server administrator.

		IpConnectionPolicyMap m_ipConnectionPolicies;	///< The ip-specific connection policies
		MimeTypeExtensionMap m_extensionMimeTypes;		///< MIME types for extensions
		MimeTypeActionMap m_mimeActions;						///< Actions for MIME types
		MimeTypeCgiMap m_mimeCgiExecutables;				///< CGI scripts for MIME types
		std::unordered_map<QString, QString> m_cgiBin;  ///< Maps the CGI exe directory per platform. Enables sharing of configs between platforms with only platform-specific items like paths not being shared.
		bool m_allowServingFromCgiBin;

		ConnectionPolicy m_defaultConnectionPolicy;  ///< The default connection policy to use if an IP address is not specifically controlled
		QString m_defaultMimeType;							///< The default MIME type to use for unrecognised resource extensions.
		WebServerAction m_defaultAction;					///< The default action to use when no specific action is set for a MIME type
		int m_cgiTimeout;										///< The timeout, in msec, for CGI execution.

		bool m_allowDirectoryListings;								  ///< Whether or not the server allows directory listings to be sent.
		bool m_showHiddenFilesInDirectoryListings;				  ///< whether or not hidden files are available if directory listings are allowed
		DirectoryListingSortOrder m_directoryListingSortOrder;  ///< The order in which files and directories appear in a generated directory listing.
	};

}  // namespace Anansi

#endif  // ANANSI_CONFIGURATION_H
