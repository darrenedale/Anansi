/** \file Configuration.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the Configuration class for EquitWebServer
  *
  * \todo class documentation.
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_CONFIGURATION_H
#define EQUITWEBSERVER_CONFIGURATION_H

#include <vector>
#include <unordered_map>
#include <optional>

#include <QString>

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "qtstdhash.h"

namespace std {
	template<>
	struct hash<QString> : public Equit::QtHash<QString> {};
}  // namespace std

namespace EquitWebServer {

	class Configuration {
	public:
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

		// TODO make this lot a MIME managing class?
		/// A list of MIME types
		using MimeTypeList = std::vector<QString>;

		/// Maps a file extension to list of MIME types.
		using MimeTypeExtensionMap = std::unordered_map<QString, MimeTypeList>;

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

		const QString & listenAddress() const;
		bool setListenAddress(const QString & listenAddress);

		int port() const;
		bool setPort(int port);

		const QString documentRoot(const QString & platform = QStringLiteral("")) const;
		bool setDocumentRoot(const QString & docRoot, const QString & platform = QStringLiteral(""));

		QStringList registeredIpAddressList() const;
		QStringList registeredFileExtensions() const;
		std::vector<QString> registeredMimeTypes() const;

		bool fileExtensionIsRegistered(const QString & ext) const;

		bool isDirectoryListingAllowed() const;
		void setAllowDirectoryListing(bool);

		bool addFileExtensionMimeType(const QString & ext, const QString & mime);
		void removeFileExtensionMimeType(const QString & ext, const QString & mime);

		inline void removeFileExtension(const QString & ext) {
			removeFileExtensionMimeType(ext, QString::null);
		}

		MimeTypeList mimeTypesForFileExtension(const QString & ext) const;
		void clearAllFileExtensions();

		WebServerAction mimeTypeAction(const QString & mime) const;
		bool setMimeTypeAction(const QString & mime, const WebServerAction & action);
		void unsetMimeTypeAction(const QString & mime);
		void clearAllMimeTypeActions();

		QString defaultMimeType() const;
		void setDefaultMimeType(const QString & mime);
		void unsetDefaultMimeType();

		WebServerAction defaultAction() const;
		void setDefaultAction(const WebServerAction & action);

		QString cgiBin() const;
		void setCgiBin(const QString & bin);

		QString mimeTypeCgi(const QString & mime) const;
		void setMimeTypeCgi(const QString & mime, const QString & cgiExe);
		void unsetMimeTypeCgi(const QString & mime);
		int cgiTimeout() const;
		bool setCgiTimeout(int);

		QString adminEmail() const;
		void setAdminEmail(const QString & admin);

		ConnectionPolicy defaultConnectionPolicy() const;
		void setDefaultConnectionPolicy(ConnectionPolicy);

		ConnectionPolicy ipAddressPolicy(const QString & addr) const;
		bool setIpAddressPolicy(const QString & addr, ConnectionPolicy p);
		bool clearIpAddressPolicy(const QString & addr);
		void clearAllIpAddressPolicies();

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
		bool readDefaultConnectionPolicyXml(QXmlStreamReader &);
		bool readDefaultMIMETypeXml(QXmlStreamReader &);
		bool readDefaultActionXml(QXmlStreamReader &);
		bool readAllowDirectoryListingsXml(QXmlStreamReader &);
		bool readIPConnectionPoliciesXml(QXmlStreamReader &);
		bool readIPConnectionPolicyXml(QXmlStreamReader &);
		bool readFileExtensionMIMETypesXml(QXmlStreamReader &);
		bool readFileExtensionMIMETypeXml(QXmlStreamReader &);
		bool readMIMETypeActionsXml(QXmlStreamReader &);
		bool readMIMETypeActionXml(QXmlStreamReader &);
		bool readMIMETypeCGIExecutablesXml(QXmlStreamReader &);
		bool readMIMETypeCGIExecutableXml(QXmlStreamReader &);

		bool writeStartXml(QXmlStreamWriter &) const;
		bool writeEndXml(QXmlStreamWriter &) const;
		bool writeDocumentRootXml(QXmlStreamWriter &) const;
		bool writeListenAddressXml(QXmlStreamWriter &) const;
		bool writeListenPortXml(QXmlStreamWriter &) const;
		bool writeDefaultConnectionPolicyXml(QXmlStreamWriter &) const;
		bool writeDefaultMIMETypeXml(QXmlStreamWriter &) const;
		bool writeAllowDirectoryListingsXml(QXmlStreamWriter &) const;
		bool writeIpConnectionPoliciesXml(QXmlStreamWriter &) const;
		bool writeFileExtensionMIMETypesXml(QXmlStreamWriter &) const;
		bool writeMimeTypeActionsXml(QXmlStreamWriter &) const;
		bool writeMimeTypeCGIExecutablesXml(QXmlStreamWriter &) const;
		bool writeDefaultActionXml(QXmlStreamWriter &) const;

		void setDefaults();

		void setInvalid();																  ///< invalidate all options
		void setInvalidDocumentRoot(const QString & = QStringLiteral(""));  ///< invalidate the document root. prevents use of default doc root when invalid path is used to construct
		void setInvalidListenAddress();												  ///< invalidate the listen address. prevents use of default address when invalid address is used to construct
		void setInvalidListenPort();													  ///< invalidate the listen port. prevents use of default port when invalid port is used to construct

	private:
		QString m_listenIP;
		int m_listenPort;
		std::unordered_map<QString, QString> m_documentRoot;

		IpConnectionPolicyMap m_ipConnectionPolicy;  ///< The ip-specific connection policies
		MimeTypeExtensionMap m_extensionMIMETypes;	///< MIME types for extensions
		MimeTypeActionMap m_mimeActions;					///< Actions for MIME types
		MimeTypeCgiMap m_mimeCgi;							///< CGI scripts for MIME types
		QString m_cgiBin;										///< The CGI exe directory. This is a relative path within document root, which will not contain '..'

		ConnectionPolicy m_defaultConnectionPolicy;  ///< The default connection policy to use if an IP address is not specifically controlled
		QString m_defaultMIMEType;							///< The default MIME type to use for unrecognised resource extensions.
		WebServerAction m_defaultAction;					///< The default action to use when no specific action is set for a MIME type
		int m_cgiTimeout;										///< The timeout, in msec, for CGI execution.

		bool m_allowDirectoryListings;  ///< Whether or not the server allows directory listings to be sent.
		QString m_adminEmail;			  ///< The email address of the server administrator.
	};

}  // namespace EquitWebServer

#endif
