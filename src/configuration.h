/** \file Configuration.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the Configuration class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_CONFIGURATION_H
#define EQUITWEBSERVER_CONFIGURATION_H

#include <vector>
#include <unordered_map>

#include <QString>
#include <QVector>
#include <QHash>

/* can't just use a forward declaration because there is an issue with the
 *OSX headers for these classes */
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
		enum WebServerAction {
			Ignore = 0, /* ignore the resource and try the action for the next mime type for a resource extension */
			Serve,		/* serve the content of the resource as-is (i.e. dump its contents to the socket) */
			CGI,			/* attempt to execute the file through CGI */
			Forbid		/* forbid access to the resource */
		};

		enum ConnectionPolicy {
			NoConnectionPolicy = 0,
			RejectConnection,
			AcceptConnection
		};

		static constexpr const uint16_t DefaultPort = 80;

	protected:
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

		// document root (indexed by platform identifier)
		QHash<QString, QString> m_documentRoot;

		QString m_listenIP;
		int m_listenPort;
		ConnectionPolicy m_defaultConnectionPolicy;  ///< The default connection policy to use if an IP address is not specifically controlled
		IpConnectionPolicyMap m_ipConnectionPolicy;  ///< The ip-specific connection policies

		MimeTypeExtensionMap m_extensionMIMETypes;  ///< Hash of extensions for MIME types, keyed by extension
		MimeTypeActionMap m_mimeActions;				  ///< Hash of actions for MIME types, keyed by MIME type
		MimeTypeCgiMap m_mimeCgi;						  ///< Hash of CGI scripts for MIME types, keyed by MIME type
		QString m_cgiBin;									  ///< The CGI exe directory. This is a relative path within document root, which will not contain '..'

		QString m_defaultMIMEType;			 ///< The default MIME type to use for unrecognised resource extensions.
		WebServerAction m_defaultAction;  ///< The default action to use when no specific action is set for a MIME type
		int m_cgiTimeout;						 ///< The timeout, in msec, for CGI execution.
		bool m_allowDirectoryListings;	 ///< Whether or not the server allows directory listings to be sent.

		QString m_adminEmail;  ///< The email address of the server administrator.

		void setDefaults();

		void setInvalid();											 ///< invalidate all options
		void setInvalidDocumentRoot(const QString & = "");  ///< invalidate the document root. prevents use of default doc root when invalid path is used to construct
		void setInvalidListenAddress();							 ///< invalidate the listen address. prevents use of default address when invalid address is used to construct
		void setInvalidListenPort();								 ///< invalidate the listen port. prevents use of default port when invalid port is used to construct

		static bool isValidIPAddress(const QString & addr);


	public:
		Configuration();
		Configuration(const QString & docRoot, const QString & listenAddress, int port);

		bool save(const QString & fileName) const;
		bool load(const QString & fileName);

		static ConnectionPolicy parseConnectionPolicyText(const QString &);
		static WebServerAction parseActionText(const QString &);
		static bool parseBooleanText(const QString &, bool);

		const QString & listenAddress() const;
		bool setListenAddress(const QString & listenAddress);

		int port() const;
		bool setPort(int port);

		const QString documentRoot(const QString & platform = "") const;
		bool setDocumentRoot(const QString & docRoot, const QString & platform = "");

		QStringList registeredIPAddressList() const;
		QStringList registeredFileExtensions() const;

		/**
			  * \brief Gets a list of MIME types with registered actions.
			  *
			  * \note The returned list will not include any MIME types associated
			  * with file extensions that do not have specific registered actions.
			  *
			  * \return A list of MIME types that have specific registered actions.
			*/
		QStringList registeredMIMETypes() const;

		bool isDirectoryListingAllowed() const;
		void setAllowDirectoryListing(bool);

		/**
			  * \brief Adds a MIME type for a file extension.
			  *
			  * \param ext is the file extension WITHOUT the leading '.'
			  * \param mime is the MIME type.
			  *
			  * The only validation carried out is to ensure that neither the extension
			  * nor the MIME type is empty.
			  *
			  * \return \c true if a new association was made between the extension and
			  * the MIME type, \c false otherwise. Note that \c false will be returned
			  * if the MIME type is already associated with the extension.
			*/
		bool addFileExtensionMIMEType(const QString & ext, const QString & mime);
		void removeFileExtensionMIMEType(const QString & ext, const QString & mime);

		inline void removeFileExtension(const QString & ext) {
			removeFileExtensionMIMEType(ext, QString::null);
		}

		MimeTypeList mimeTypesForFileExtension(const QString & ext) const;
		void clearAllFileExtensions();

		/**
			  * \brief Gets the action configured for a MIME type.
			  *
			  * \param mime is the MIME type.
			  *
			  * \note If the MIME type provided is empty, the action will always be Forbid.
			  * This is because an empty MIME type is only given for a file extension when
			  * the server is configured not to provide a default MIME type, in other words
			  * when the server is configured not to serve files of types it does not
			  * recognise. To serve files even when the server does not recognise the
			  * extension, set a default MIME type, which will guarantee that all extensions
			  * will resolve to a MIME type.
			  *
			  * \return The action associated with the MIME type, or the default action
			  * if no specific action has been defined for the MIME type.
			*/
		WebServerAction mimeTypeAction(const QString & mime) const;
		bool setMimeTypeAction(const QString & mime, const WebServerAction & action);
		void unsetMimeTypeAction(const QString & mime);
		void clearAllMimeTypeActions();

		/**
			  * \brief Gets the default MIME type.
			  *
			  * \see setDefaultMimeType(), unsetDefaultMIMEType();
			  *
			  * \return The default MIME type, or an empty string if no default MIME type
			  * is set.
			*/
		QString defaultMIMEType() const;

		/**
			  * \brief Sets the default MIME type.
			  *
			  * \param mime is the MIME type to use as the default.
			  *
			  * \see getDefaultMimeType(), unsetDefaultMIMEType();
			  *
			  * The default MIME type is used when a resource extension cannot be translated
			  * into a MIME type. If it is set to an empty string, no default MIME type will
			  * be used, and resources whose extension is not recognised will not be served.
			*/
		void setDefaultMIMEType(const QString & mime);

		/**
			  * \brief Unsets the default MIME type.
			  *
			  * \see getDefaultMimeType(), setDefaultMIMEType();
			  *
			  * This method ensures that resources with unknown MIME types are not served.
			*/
		void unsetDefaultMIMEType();

		/**
			  * \brief Gets the default action.
			  *
			  * \see setDefaultAction()
			  *
			  * \return The default action.
			*/
		WebServerAction defaultAction() const;

		/**
			  * \brief Sets the default action.
			  *
			  * \param action is the default action to use.
			  *
			  * The default action is given when a MIME type does not have a specific action
			  * attached to it.
			*/
		void setDefaultAction(const WebServerAction & action);

		QString cgiBin() const;
		void setCgiBin(const QString & bin);

		/**
			  * \brief Adds a CGI handler for a MIME type.
			  *
			  * \param mime is the MIME type for which to add a CGI handler.
			  * \param cgiExe is the executable to use for CGI execution.
			  *
			  * Note that this method does not guarantee that a MIME type will be handled
			  * by CGI. The MIME type will only be handled by CGI if the action for that
			  * MIME type is set to \c CGI in setMIMETypeAction().
			  *
			  * The execution will always respect the setting for CGIBin. Only executables
			  * found in the directory specified in CGIBin will be used. If the executable
			  * provided to this method is not in that directory, CGI execution will fail at
			  * runtime.
			*/
		QString mimeTypeCgi(const QString & mime) const;
		void setMimeTypeCgi(const QString & mime, const QString & cgiExe);
		void unsetMIMETypeCGI(const QString & mime);
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

		/* XML IO methods. */
		bool parseWebserverXml(QXmlStreamReader &);
		void parseUnknownElementXml(QXmlStreamReader &);
		bool parseDocumentRootXml(QXmlStreamReader &);
		bool parseListenAddressXml(QXmlStreamReader &);
		bool parseListenPortXml(QXmlStreamReader &);
		bool parseDefaultConnectionPolicyXml(QXmlStreamReader &);
		bool parseDefaultMIMETypeXml(QXmlStreamReader &);
		bool parseDefaultActionXml(QXmlStreamReader &);
		bool parseAllowDirectoryListingsXml(QXmlStreamReader &);
		bool parseIPConnectionPoliciesXml(QXmlStreamReader &);
		bool parseIPConnectionPolicyXml(QXmlStreamReader &);
		bool parseFileExtensionMIMETypesXml(QXmlStreamReader &);
		bool parseFileExtensionMIMETypeXml(QXmlStreamReader &);
		bool parseMIMETypeActionsXml(QXmlStreamReader &);
		bool parseMIMETypeActionXml(QXmlStreamReader &);
		bool parseMIMETypeCGIExecutablesXml(QXmlStreamReader &);
		bool parseMIMETypeCGIExecutableXml(QXmlStreamReader &);

		bool startXml(QXmlStreamWriter &) const;
		bool endXml(QXmlStreamWriter &) const;
		bool writeXml(QXmlStreamWriter &) const;
		bool documentRootXml(QXmlStreamWriter &) const;
		bool listenAddressXml(QXmlStreamWriter &) const;
		bool listenPortXml(QXmlStreamWriter &) const;
		bool defaultConnectionPolicyXml(QXmlStreamWriter &) const;
		bool defaultMIMETypeXml(QXmlStreamWriter &) const;
		bool allowDirectoryListingsXml(QXmlStreamWriter &) const;
		bool ipConnectionPoliciesXml(QXmlStreamWriter &) const;
		bool fileExtensionMIMETypesXml(QXmlStreamWriter &) const;
		bool mimeTypeActionsXml(QXmlStreamWriter &) const;
		bool mimeTypeCGIExecutablesXml(QXmlStreamWriter &) const;
		bool defaultActionXml(QXmlStreamWriter &) const;
	}; /* Configuration class */
}  // namespace EquitWebServer

#endif
