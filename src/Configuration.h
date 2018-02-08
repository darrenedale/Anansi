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

#include <QString>
#include <QVector>
#include <QHash>

/* can't just use a forward declaration because there is an issue with the
 *OSX headers for these classes */
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


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
		typedef QHash<QString, QVector<QString>> MimeTypeExtensionMap;	// Maps MIME types to a file extension
		typedef QHash<QString, WebServerAction> MimeTypeActionMap;		  // Maps an action to a MIME type
		typedef QHash<QString, QString> MimeTypeCGIMap;						  // Maps a CGI script to a MIME type
		typedef QHash<QString, ConnectionPolicy> IPConnectionPolicyMap;  // Maps IP addresses to a connection policy

		QHash<QString, QString> m_documentRoot;
		QString m_listenIP;
		int m_listenPort;
		ConnectionPolicy m_defaultConnectionPolicy;  ///< The default connection policy to use if an IP address is not specifically controlled
		IPConnectionPolicyMap m_ipConnectionPolicy;  ///< The ip-specific connection policies

		MimeTypeExtensionMap m_extensionMIMETypes;  ///< Hash of extensions for MIME types, keyed by extension
		MimeTypeActionMap m_mimeActions;				  ///< Hash of actions for MIME types, keyed by MIME type
		MimeTypeCGIMap m_mimeCGI;						  ///< Hash of CGI scripts for MIME types, keyed by MIME type
		QString m_cgiBin;									  ///< The CGI exe directory. This is a relative path within document root, which will not contain '..'

		QString m_defaultMIMEType;			 ///< The default MIME type to use for unrecognised resource extensions.
		WebServerAction m_defaultAction;  ///< The default action to use when no specific action is set for a MIME type
		int m_cgiTimeout;						 ///< The timeout, in msec, for CGI execution.
		bool m_allowDirectoryListings;	 ///< Whether or not the server allows directory listings to be sent.

		QString m_adminEmail;  ///< The email address of the server administrator.

		void setDefaults(void);

		void setInvalid(void);										 ///< invalidate all options
		void setInvalidDocumentRoot(const QString & = "");  ///< invalidate the document root. prevents use of default doc root when invalid path is used to construct
		void setInvalidListenAddress(void);						 ///< invalidate the listen address. prevents use of default address when invalid address is used to construct
		void setInvalidListenPort(void);							 ///< invalidate the listen port. prevents use of default port when invalid port is used to construct

		static bool isValidIPAddress(const QString & addr);


	public:
		Configuration(void);
		Configuration(const QString & docRoot, const QString & listenAddress, int port);

		bool save(const QString & fileName) const;
		bool load(const QString & fileName);

		static ConnectionPolicy parseConnectionPolicyText(const QString &);
		static WebServerAction parseActionText(const QString &);
		static bool parseBooleanText(const QString &, bool);

		const QString & listenAddress(void) const;
		bool setListenAddress(const QString & listenAddress);

		int port(void) const;
		bool setPort(int port);

		const QString getDocumentRoot(const QString & platform = "") const;
		bool setDocumentRoot(const QString & docRoot, const QString & platform = "");

		QStringList registeredIPAddressList(void) const;
		QStringList registeredFileExtensions(void) const;

		/**
			  * \brief Gets a list of MIME types with registered actions.
			  *
			  * \note The returned list will not include any MIME types associated
			  * with file extensions that do not have specific registered actions.
			  *
			  * \return A list of MIME types that have specific registered actions.
			*/
		QStringList registeredMIMETypes(void) const;

		bool isDirectoryListingAllowed(void) const;
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
		void removeFileExtension(const QString & ext);
		QVector<QString> getMIMETypesForFileExtension(const QString & ext) const;
		void clearAllFileExtensions(void);

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
		WebServerAction getMIMETypeAction(const QString & mime) const;
		bool setMIMETypeAction(const QString & mime, const WebServerAction & action);
		void unsetMIMETypeAction(const QString & mime);
		void clearAllMIMETypeActions(void);

		/**
			  * \brief Gets the default MIME type.
			  *
			  * \see setDefaultMimeType(), unsetDefaultMIMEType();
			  *
			  * \return The default MIME type, or an empty string if no default MIME type
			  * is set.
			*/
		QString getDefaultMIMEType(void) const;

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
		void unsetDefaultMIMEType(void);

		/**
			  * \brief Gets the default action.
			  *
			  * \see setDefaultAction()
			  *
			  * \return The default action.
			*/
		WebServerAction getDefaultAction(void) const;

		/**
			  * \brief Sets the default action.
			  *
			  * \param action is the default action to use.
			  *
			  * The default action is given when a MIME type does not have a specific action
			  * attached to it.
			*/
		void setDefaultAction(const WebServerAction & action);

		QString getCGIBin(void) const;
		void setCGIBin(const QString & bin);

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
		QString getMIMETypeCGI(const QString & mime) const;
		void setMIMETypeCGI(const QString & mime, const QString & cgiExe);
		void unsetMIMETypeCGI(const QString & mime);
		int getCGITimeout(void) const;
		bool setCGITimeout(int);

		QString getAdminEmail(void) const;
		void setAdminEmail(const QString & admin);

		ConnectionPolicy getDefaultConnectionPolicy(void) const;
		void setDefaultConnectionPolicy(ConnectionPolicy);

		ConnectionPolicy ipAddressPolicy(const QString & addr) const;
		bool setIPAddressPolicy(const QString & addr, ConnectionPolicy p);
		bool clearIPAddressPolicy(const QString & addr);
		void clearAllIPAddressPolicies(void);

		/* XML IO methods. */
		bool parseWebserverXML(QXmlStreamReader &);
		void parseUnknownElementXML(QXmlStreamReader &);
		bool parseDocumentRootXML(QXmlStreamReader &);
		bool parseListenAddressXML(QXmlStreamReader &);
		bool parseListenPortXML(QXmlStreamReader &);
		bool parseDefaultConnectionPolicyXML(QXmlStreamReader &);
		bool parseDefaultMIMETypeXML(QXmlStreamReader &);
		bool parseDefaultActionXML(QXmlStreamReader &);
		bool parseAllowDirectoryListingsXML(QXmlStreamReader &);
		bool parseIPConnectionPoliciesXML(QXmlStreamReader &);
		bool parseIPConnectionPolicyXML(QXmlStreamReader &);
		bool parseFileExtensionMIMETypesXML(QXmlStreamReader &);
		bool parseFileExtensionMIMETypeXML(QXmlStreamReader &);
		bool parseMIMETypeActionsXML(QXmlStreamReader &);
		bool parseMIMETypeActionXML(QXmlStreamReader &);
		bool parseMIMETypeCGIExecutablesXML(QXmlStreamReader &);
		bool parseMIMETypeCGIExecutableXML(QXmlStreamReader &);

		bool startXML(QXmlStreamWriter &) const;
		bool endXML(QXmlStreamWriter &) const;
		bool writeXML(QXmlStreamWriter &) const;
		bool documentRootXML(QXmlStreamWriter &) const;
		bool listenAddressXML(QXmlStreamWriter &) const;
		bool listenPortXML(QXmlStreamWriter &) const;
		bool defaultConnectionPolicyXML(QXmlStreamWriter &) const;
		bool defaultMIMETypeXML(QXmlStreamWriter &) const;
		bool allowDirectoryListingsXML(QXmlStreamWriter &) const;
		bool ipConnectionPoliciesXML(QXmlStreamWriter &) const;
		bool fileExtensionMIMETypesXML(QXmlStreamWriter &) const;
		bool mimeTypeActionsXML(QXmlStreamWriter &) const;
		bool mimeTypeCGIExecutablesXML(QXmlStreamWriter &) const;
		bool defaultActionXML(QXmlStreamWriter &) const;
	}; /* Configuration class */
}  // namespace EquitWebServer

#endif
