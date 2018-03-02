/// \file configuration.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the Configuration class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_CONFIGURATION_H
#define EQUITWEBSERVER_CONFIGURATION_H

#include <vector>
#include <map>
#include <unordered_map>
#include <optional>

#include <QString>

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "types.h"
#include "numerics.h"
#include "qtstdhash.h"

namespace std {
	template<>
	struct hash<QString> : public Equit::QtHash<QString> {};
}  // namespace std

namespace EquitWebServer {

	class Configuration {
	public:
		// TODO make this lot a MIME managing class?
		/// A list of MIME types
		using MimeTypeList = std::vector<QString>;

		/// Maps a file extension to list of MIME types.
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

		int port() const;
		bool setPort(int port);

		const QString documentRoot(const QString & platform = QStringLiteral("")) const;
		bool setDocumentRoot(const QString & docRoot, const QString & platform = QStringLiteral(""));

		QString administratorEmail() const;
		void setAdministratorEmail(const QString & admin);

		bool directoryListingsAllowed() const;
		void setDirectoryListingsAllowed(bool);

		bool showHiddenFilesInDirectoryListings() const;
		void setShowHiddenFilesInDirectoryListings(bool);

		inline DirectoryListingSortOrder directoryListingSortOrder() const {
			return m_directoryListingSortOrder;
		}

		inline void setDirectoryListingSortOrder(DirectoryListingSortOrder sortOrder) {
			m_directoryListingSortOrder = sortOrder;
		}

		QString cgiBin() const;
		void setCgiBin(const QString & bin);

		int cgiTimeout() const;
		bool setCgiTimeout(int);

		std::vector<QString> registeredIpAddresses() const;
		std::vector<QString> registeredFileExtensions() const;
		std::vector<QString> registeredMimeTypes() const;
		std::vector<QString> allKnownMimeTypes() const;

		inline int registeredIpAddressCount() const {
			return static_cast<int>(m_ipConnectionPolicy.size());
		}

		inline int registeredFileExtensionCount() const {
			return static_cast<int>(m_extensionMimeTypes.size());
		}

		inline int registeredMimeTypeCount() const {
			return static_cast<int>(m_mimeActions.size());
		}

		// connection policies
		ConnectionPolicy defaultConnectionPolicy() const;
		void setDefaultConnectionPolicy(ConnectionPolicy);

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

		QString defaultMimeType() const;
		void setDefaultMimeType(const QString & mime);
		void unsetDefaultMimeType();

		// actions to take for MIME types
		bool mimeTypeIsRegistered(const QString & mimeType) const;
		WebServerAction mimeTypeAction(const QString & mime) const;
		bool setMimeTypeAction(const QString & mime, const WebServerAction & action);
		void unsetMimeTypeAction(const QString & mime);
		void clearAllMimeTypeActions();

		WebServerAction defaultAction() const;
		void setDefaultAction(const WebServerAction & action);

		QString mimeTypeCgi(const QString & mime) const;
		void setMimeTypeCgi(const QString & mime, const QString & cgiExe);
		void unsetMimeTypeCgi(const QString & mime);

		bool readWebserverXml(QXmlStreamReader &);
		bool writeWebserverXml(QXmlStreamWriter &) const;

#if !defined(NDEBUG)
		void dumpFileAssociationMimeTypes();
		void dumpFileAssociationMimeTypes(const QString & ext);
#endif
	protected:
		bool readDocumentRootXml(QXmlStreamReader &);
		bool readListenAddressXml(QXmlStreamReader &);
		bool readListenPortXml(QXmlStreamReader &);
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

	private:
		QString m_listenIp;
		int m_listenPort;
		std::unordered_map<QString, QString> m_documentRoot;

		IpConnectionPolicyMap m_ipConnectionPolicy;  ///< The ip-specific connection policies
		MimeTypeExtensionMap m_extensionMimeTypes;	///< MIME types for extensions
		MimeTypeActionMap m_mimeActions;					///< Actions for MIME types
		MimeTypeCgiMap m_mimeCgi;							///< CGI scripts for MIME types
		QString m_cgiBin;										///< The CGI exe directory. This is a relative path within document root, which will not contain '..'

		ConnectionPolicy m_defaultConnectionPolicy;  ///< The default connection policy to use if an IP address is not specifically controlled
		QString m_defaultMimeType;							///< The default MIME type to use for unrecognised resource extensions.
		WebServerAction m_defaultAction;					///< The default action to use when no specific action is set for a MIME type
		int m_cgiTimeout;										///< The timeout, in msec, for CGI execution.

		bool m_allowDirectoryListings;								  ///< Whether or not the server allows directory listings to be sent.
		bool m_showHiddenFilesInDirectoryListings;				  ///< whether or not hidden files are available if directory listings are allowed
		DirectoryListingSortOrder m_directoryListingSortOrder;  ///< The order in which files and directories appear in a generated directory listing.
		QString m_adminEmail;											  ///< The email address of the server administrator.
	};

}  // namespace EquitWebServer

#endif
