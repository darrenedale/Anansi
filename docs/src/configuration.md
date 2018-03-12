<a name="class_Configuration"></a>
# Class: Configuration
Encapsulates the configuration of the web server. The configuration specifies simple features such as the address and port on which the server listens, the document root from which the server will serve content, to more complex setup such as which connections are accepted and which are rejected, how file name extensions map to media types, what the server should do with different media types (e.g. serve, forbid).

## Detailed description

### Core server details
The address on which the server listens is set using [setListenAddress()](#fn_setListenAddress) and is queried using [listenAddress()](#fn_listenAddress). Any valid IP address will be accepted, regardless of whether it is one assigned to the host on which the server is running, and regardless of whether it is in a reserved range. This does not mean the server will actually listen on that address - when started the server will fail to bind to an address that is not available on the host on which it is running. The port is set using [setPort()](#fn_setPort) and queried using [port()](#fn_port). The port must be greater than 0 and less than 65535; an other port number is invalid. Again, a valid port will be accepted in the configuration regardless of whether that port is actually available. Starting the server will fail if the port is not available at that time.

The server can be configured to show a directory listing when a request is made for a resource that maps to a directory rather than a file. This is set using [setDirectoryListingsAllowed()](#fn_setDirectoryListingsAllowed) and queried using [directoryListingsAllowed()](#fn_directoryListingsAllowed). If directory listings are available, it is possible to set whether or not hidden files on the host are listed or hidden and the sort order for entries in directory listings. These are set and queried using [setShowHiddenFilesInDirectoryListings()](#fn_setShowHiddenFilesInDirectoryListings) and [showHiddenFilesInDirectoryListings()](#fn_showHiddenFilesInDirectoryListings), and [setDirectoryListingSortOrder()](#fn_setDirectoryListingSortOrder) and [directoryListingSortOrder()](#fn_directoryListingSortOrder) respectively.

The [Server](server.md) class implements basic CGI 1.1. The directory in which it looks for CGI scripts is set using [setCgiBin()](#fn_setCgiBin) and queried using [cgiBin()](#fn_cgiBin). For security reasons this should be set to a directory that is outside the document root unless you are certain that the content (i.e. source) of your CGI scripts is neither private nor dangerous to download, and that the content of the CGI binary directory cannot be altered by any untrusted party (e.g. other user accounts on the host, FTP accounts, etc.). You can configure the server to refuse to serve files that are inside the CGI binary directory using [setAllowServingFilesFromCgiBin()](#fn_setAllowServingFilesFromCgiBin) (query the setting using [allowServingFilesFromCgiBin()](#fn_allowServingFilesFromCgiBin)). The timeout for CGI script execution is set with [setCgiTimeout()](#fn_setCgiTimeout) and queried using [cgiTimeout()](#fn_cgiTimeout).


### Connections

The settings governing what happens to incoming connections are managed by specifying [ConnectionPolicy.md](connection policies) for individual IP addresses. IP Address ranges or subnets are not (yet) supported. Policies are set using [setIpAddressConnectionPolicy()](#fn_setIpAddressConnectionPolicy) and queried using [ipAddressConnectionPolicy()](#fn_ipAddressConnectionPolicy). The policy for an IP address can be cleared using [unsetIpAddressConnectionPolicy()](#fn_unsetIpAddressConnectionPolicy). All policies can be cleared _en-masse_ with [clearAllIpAddressConnectionPolicies()](#fn_clearAllIpAddressConnectionPolicies). You can query whether an IP address has a policy registered with [ipAddressIsRegistered()](#fn_ipAddressIsRegistered) (calling [ipAddressConnectionPolicy()](#fn_ipAddressConnectionPolicy) is not suitable for this purpose because it returns `None` for cases where the policy has explicitly been set to `None` and where no policy has been set).

In addition to IP-specific connection policies, there is a default connection policy that is used for connections from IP addresses either without an explictyly-set policy or where the policy has been set explicitly to `None`. This is set using [setDefaultConnectionPolicy()](#fn_setDefaultConnectionPolicy) and queried using [defaultConnectionPolicy()](#fn_defaultConnectionPolicy). Note that where the default connection policy is set to `None`, the [server.md](Server) will act as if the policy is `Reject`.

The full set of IP addresses for which policies exist can be retrieved using [registeredIpAddresses()](#fn_registeredIpAddresses); the number of IP addresses for which explicit policies have been set can be queried using [registeredIpAddressCount()](#fn_registeredIpAddressCount) - this is faster than counting the entries in the list returned by [registeredIpAddresses()](#fn_registeredIpAddresses).


### File type associations

Note: At present, this documentation refers to _media types_, which is the preferred way to refer to what used to be known as MIME types. The code, however, has yet to be updated for this change, and still refers to MIME types. When reading this documentation and the code, the terms _media type_ and _MIME type_ should be understood to refer to the same thing.

File type associations are handled by specifying a list of media types that can be used for files whose names end in a given extension. The extension is always considered to be the part of the file anme after last `.` character. Files whose names have no `.` character, or whose only `.` character is the first in the name, are considered to have no extension. The [server.md](Server) processes the media types for a file extension in order from top to bottom. The first media type for which it finds an action that is not `Ignore` is used, that action is executed, and the remaining media types for that extension are not processed.

A media type can be added for a file extension using [addFileExtensionMimeType()](#fn_addFileExtensionMimeType). If the provided extension already has one or more associated media types, the provided media type is added to its list; otherwise, a the extension is added with a media type list containing just the provided media type. This is the only way to add a new extension to the file associations. The list of media types for an extension can be queried using [mimeTypesForFileExtension()](#fn_mimeTypesForFileExtension). To query whether an extension has been registered, use [fileExtensionIsRegistered()](#fn_fileExtensionIsRegistered); to query whether an extension has been associated with a particular media type use [fileExtensionHasMimeType()](#fn_fileExtensionHasMimeType) - this is faster than retrieving the set of associated media types and examining it. To remove a media type from a file extension call [removeFileExtensionMimeType()](#fn_removeFileExtensionMimeType) or to remove all media types from an extension, and the extension itself, call [removeFileExtension()](#fn_removeFileExtension). All file extensions can be removed _en-masse_ using [clearAllFileExtensions()](#fn_clearAllFileExtensions). To selectively change a media type for an extension from one media type to another (i.e. effectively to rename one of the media types assocated with a file extension) call [changeFileExtensionMimeType()](#fn_changeFileExtensionMimeType). To change a file type association from one extension to another (i.e. to remove one file extension and replace it with another that has the identical media type list) call [changeFileExtension()](#fn_changeFileExtension).

The full set of file extensions for which media types have been assigned can be retrieved using [registeredFileExtensions()](#fn_registeredFileExtensions); the number of extensions for which media types have been set can be queried using [registeredFileExtensionCount()](#fn_registeredFileExtensionCount) - this is faster than counting the entries in the list returned by [registeredFileExtensions()](#fn_registeredFileExtensions).

The default media type, which will be used when a request is received for a resource for which a specifically-associated media type has not been set, is set using [setDefaultMimeType()](#fn_setDefaultMimeType) and queried using [defaultMimeType()](#fn_defaultMimeType); it can be reset using [unsetDefaultMimeType()](#fn_unsetDefaultMimeType), which will set the default media type to its built-in default value (currently `application/octet-stream`).


### Media type actions

Once the [Server](server.md) has determined the media type to use for a given request it needs to know what to do with the resource requested. This can be to serve the local file identified by the request, forbid access to the resource, pass the resource on be executed through CGI or simply to ignore the request (see [WebServerAction](webserveraction.md)).

Actions are assigned to media types using [setMimeTypeAction()](#fn_setMimeTypeAction) and queried using [mimeTypeAction()](#fn_mimeTypeAction). A media type can have its explicit action removed using [unsetMimeTypeAction()](#fn_unsetMimeTypeAction); this can be done for all media types _en-masse_ using [clearAllMimeTypeActions()](#fn_clearAllMimeTypeActions). A list of the media types for which explicit actions have been set can be retrieved using [registeredMimeTypes()](#fn_registeredMimeTypes) (note the difference between this an [allKnownMimeTypes()](#fn_allKnownMimeTypes) which includes all media types associated with a file extension regardless of whether an action has been specified for that media type). The number of media types for which actions have explicitly been set can be queried using [registeredMimeTypeCount()](#fn_registeredMimeTypeCount) - this is faster than countin the entries in the list returned by [registeredMimeTypes()](#fn_registeredMimeTypes). To query whether a media type has an explicitly associated action, call [mimeTypeIsRegistered()](#fn_mimeTypeIsRegistered) - again, this is faster than calling [registeredMimeTypes()](#fn_registeredMimeTypes) and searching for the media type.

For media types that are registered to be executed through the CGI, the interpreter used to execute the script is specified using [setMimeTypeCgi()](#fn_setMimeTypeCgi) and queried using [mimeTypeCgi()](#fn_mimeTypeCgi). To remove the interpreter for a media type use [unsetMimeTypeCgi()](#fn_unsetMimeTypeCgi). Note that for any media type for which the action is set to `CGI` but whose CGI interpreter is unset or set to an empty string, the [server.md](Server) will respond to the client with a _Forbidden_ response - in other words, all CGI content served from within the document root must be processed with an interpreter explicitly set in the server configuration. This helps ensure that no CGI content can accidentally be directly-executed from inside the document root.

The default action, which is used when requests for media types with no explictly associated action are received, is set using [setDefaultAction()](#fn_setDefaultAction) and queried using [defaultAction()](#fn_defaultAction).


## Public constructors

<a name="fn_Configuration"></a>
### `Configuration()`
Construct a default [Configuration](configuration.md) object.

<a name="fn_Configuration_Qstring_QString_int"></a>
### `Configuration(const QString & docRoot, const QString & listenAddress, int port)`
Construct a very basic [Configuration](configuration.md) object.
#### Parameters
- `docRoot` The document root.
- `listenAddress` The IPv4 address to listen on.
- `port` The port to listen on.
The listen address must be provided in dotted-decimal form. All other configuration options are set to their default values.

## Public member functions

<a name="fn_loadFrom"></a>
### `static std::optional<Configuration> loadFrom(const QString & fileName)`
Load a configuration from a file.
#### Parameters
- `fileName` The path to the file to load.
The returned optional will be empty if the configuration file could not be loaded or contained an invalid configuration; otherwise, it will contain a valid [Configuration](configuration.md) object.
#### Returns
The loaded configuration or an empty optional.

<a name="fn_saveAs"></a>
### `bool saveAs(const QString & fileName) const`

<a name="fn_listenAddress"></a>
### `const QString & listenAddress() const`

<a name="fn_setListenAddress"></a>
### `bool setListenAddress(const QString & listenAddress)`

<a name="fn_port"></a>
### `int port() const noexcept`

<a name="fn_setPort"></a>
### `bool setPort(int port) noexcept`

<a name="fn_documentRoot"></a>
### `const QString documentRoot(const QString & platform = QStringLiteral()) const`

<a name="fn_setDocumentRoot"></a>
### `bool setDocumentRoot(const QString & docRoot, const QString & platform = QStringLiteral())`

<a name="fn_administratorEmail"></a>
### `QString administratorEmail() const`

<a name="fn_setAdministratorEmail"></a>
### `void setAdministratorEmail(const QString & admin)`

<a name="fn_directoryListingsAllowed"></a>
### `bool directoryListingsAllowed() const noexcept`

<a name="fn_setDirectoryListingsAllowed"></a>
### `void setDirectoryListingsAllowed(bool) noexcept`

<a name="fn_showHiddenFilesInDirectoryListings"></a>
### `bool showHiddenFilesInDirectoryListings() const noexcept`

<a name="fn_setShowHiddenFilesInDirectoryListings"></a>
### `void setShowHiddenFilesInDirectoryListings(bool) noexcept`

<a name="fn_directoryListingSortOrder"></a>
### `inline DirectoryListingSortOrder directoryListingSortOrder() const noexcept`

<a name="fn_setDirectoryListingSortOrder"></a>
### `inline void setDirectoryListingSortOrder(DirectoryListingSortOrder sortOrder) noexcept`

<a name="fn_cgiBin"></a>
### `QString cgiBin(const QString & platform = QStringLiteral()) const`

<a name="fn_setCgiBin"></a>
### `bool setCgiBin(const QString & bin, const QString & platform = QStringLiteral())`

<a name="fn_cgiTimeout"></a>
### `int cgiTimeout() const noexcept`

<a name="fn_setCgiTimeout"></a>
### `bool setCgiTimeout(int) noexcept`

<a name="fn_allowServingFilesFromCgiBin"></a>
### `bool allowServingFilesFromCgiBin() const noexcept`

<a name="fn_setAllowServingFilesFromCgiBin"></a>
### `void setAllowServingFilesFromCgiBin(bool allow) noexcept`

<a name="fn_registeredIpAddresses"></a>
### `std::vector<QString> registeredIpAddresses() const`

<a name="fn_registeredFileExtensions"></a>
### `std::vector<QString> registeredFileExtensions() const`

<a name="fn_registeredMimeTypes"></a>
### `std::vector<QString> registeredMimeTypes() const`

<a name="fn_allKnownMimeTypes"></a>
### `std::vector<QString> allKnownMimeTypes() const`

<a name="fn_registeredIpAddressCount"></a>
### `inline int registeredIpAddressCount() const noexcept`

<a name="fn_registeredFileExtensionCount"></a>
### `inline int registeredFileExtensionCount() const noexcept`

<a name="fn_registeredMimeTypeCount"></a>
### `inline int registeredMimeTypeCount() const noexcept`

<a name="fn_defaultConnectionPolicy"></a>
### `ConnectionPolicy defaultConnectionPolicy() const noexcept`

<a name="fn_setDefaultConnectionPolicy"></a>
### `void setDefaultConnectionPolicy(ConnectionPolicy) noexcept`

<a name="fn_ipAddressIsRegistered"></a>
### `bool ipAddressIsRegistered(const QString & addr) const`

<a name="fn_ipAddressConnectionPolicy"></a>
### `ConnectionPolicy ipAddressConnectionPolicy(const QString & addr) const`

<a name="fn_setIpAddressConnectionPolicy"></a>
### `bool setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy p)`

<a name="fn_unsetIpAddressConnectionPolicy"></a>
### `bool unsetIpAddressConnectionPolicy(const QString & addr)`

<a name="fn_clearAllIpAddressConnectionPolicies"></a>
### `void clearAllIpAddressConnectionPolicies()`

<a name="fn_fileExtensionIsRegistered"></a>
### `bool fileExtensionIsRegistered(const QString & ext) const`

<a name="fn_fileExtensionHasMimeType"></a>
### `bool fileExtensionHasMimeType(const QString & ext, const QString & mime) const`

<a name="fn_changeFileExtensionMimeType"></a>
### `bool changeFileExtensionMimeType(const QString & ext, const QString & fromMime, const QString & toMime)`

<a name="fn_addFileExtensionMimeType"></a>
### `bool addFileExtensionMimeType(const QString & ext, const QString & mime)`

<a name="fn_removeFileExtensionMimeType"></a>
### `void removeFileExtensionMimeType(const QString & ext, const QString & mime)`

<a name="fn_changeFileExtension"></a>
### `bool changeFileExtension(const QString & oldExt, const QString & newExt)`

<a name="fn_removeFileExtension"></a>
### `inline void removeFileExtension(const QString & ext)`

<a name="fn_fileExtensionMimeTypeCount"></a>
### `int fileExtensionMimeTypeCount(const QString & ext) const`

<a name="fn_mimeTypesForFileExtension"></a>
### `MimeTypeList mimeTypesForFileExtension(const QString & ext) const`

<a name="fn_clearAllFileExtensions"></a>
### `void clearAllFileExtensions()`

<a name="fn_defaultMimeType"></a>
### `QString defaultMimeType() const`

<a name="fn_setDefaultMimeType"></a>
### `void setDefaultMimeType(const QString & mime)`

<a name="fn_unsetDefaultMimeType"></a>
### `void unsetDefaultMimeType()`

<a name="fn_mimeTypeIsRegistered"></a>
### `bool mimeTypeIsRegistered(const QString & mime) const`

<a name="fn_mimeTypeAction"></a>
### `WebServerAction mimeTypeAction(const QString & mime) const`

<a name="fn_setMimeTypeAction"></a>
### `bool setMimeTypeAction(const QString & mime, const WebServerAction & action)`

<a name="fn_unsetMimeTypeAction"></a>
### `void unsetMimeTypeAction(const QString & mime)`

<a name="fn_clearAllMimeTypeActions"></a>
### `void clearAllMimeTypeActions()`

<a name="fn_defaultAction"></a>
### `WebServerAction defaultAction() const`

<a name="fn_setDefaultAction"></a>
### `void setDefaultAction(const WebServerAction & action)`

<a name="fn_mimeTypeCgi"></a>
### `QString mimeTypeCgi(const QString & mime) const`

<a name="fn_setMimeTypeCgi"></a>
### `void setMimeTypeCgi(const QString & mime, const QString & cgiExe)`

<a name="fn_unsetMimeTypeCgi"></a>
### `void unsetMimeTypeCgi(const QString & mime)`


## Private member functions

<a name="fn_readWebserverXml"></a>
### `bool readWebserverXml(QXmlStreamReader &)`

<a name="fn_writeWebserverXml"></a>
### `bool writeWebserverXml(QXmlStreamWriter &) const`

<a name="fn_readDocumentRootXml"></a>
### `bool readDocumentRootXml(QXmlStreamReader &)`

<a name="fn_readListenAddressXml"></a>
### `bool readListenAddressXml(QXmlStreamReader &)`

<a name="fn_readListenPortXml"></a>
### `bool readListenPortXml(QXmlStreamReader &)`

<a name="fn_readCgiBinXml"></a>
### `bool readCgiBinXml(QXmlStreamReader &)`

<a name="fn_readAllowServingFilesFromCgiBin"></a>
### `bool readAllowServingFilesFromCgiBin(QXmlStreamReader &)`

<a name="fn_readAdministratorEmailXml"></a>
### `bool readAdministratorEmailXml(QXmlStreamReader &)`

<a name="fn_readDefaultConnectionPolicyXml"></a>
### `bool readDefaultConnectionPolicyXml(QXmlStreamReader &)`

<a name="fn_readDefaultMimeTypeXml"></a>
### `bool readDefaultMimeTypeXml(QXmlStreamReader &)`

<a name="fn_readDefaultActionXml"></a>
### `bool readDefaultActionXml(QXmlStreamReader &)`

<a name="fn_readAllowDirectoryListingsXml"></a>
### `bool readAllowDirectoryListingsXml(QXmlStreamReader &)`

<a name="fn_readShowHiddenFilesInDirectoryListingsXml"></a>
### `bool readShowHiddenFilesInDirectoryListingsXml(QXmlStreamReader &)`

<a name="fn_readDirectoryListingSortOrderXml"></a>
### `bool readDirectoryListingSortOrderXml(QXmlStreamReader &)`

<a name="fn_readIpConnectionPoliciesXml"></a>
### `bool readIpConnectionPoliciesXml(QXmlStreamReader &)`

<a name="fn_readIpConnectionPolicyXml"></a>
### `bool readIpConnectionPolicyXml(QXmlStreamReader &)`

<a name="fn_readFileExtensionMimeTypesXml"></a>
### `bool readFileExtensionMimeTypesXml(QXmlStreamReader &)`

<a name="fn_readFileExtensionMimeTypeXml"></a>
### `bool readFileExtensionMimeTypeXml(QXmlStreamReader &)`

<a name="fn_readMimeTypeActionsXml"></a>
### `bool readMimeTypeActionsXml(QXmlStreamReader &)`

<a name="fn_readMimeTypeActionXml"></a>
### `bool readMimeTypeActionXml(QXmlStreamReader &)`

<a name="fn_readMimeTypeCgiExecutablesXml"></a>
### `bool readMimeTypeCgiExecutablesXml(QXmlStreamReader &)`

<a name="fn_readMimeTypeCgiExecutableXml"></a>
### `bool readMimeTypeCgiExecutableXml(QXmlStreamReader &)`

<a name="fn_writeStartXml"></a>
### `bool writeStartXml(QXmlStreamWriter &) const`

<a name="fn_writeEndXml"></a>
### `bool writeEndXml(QXmlStreamWriter &) const`

<a name="fn_writeDocumentRootXml"></a>
### `bool writeDocumentRootXml(QXmlStreamWriter &) const`

<a name="fn_writeListenAddressXml"></a>
### `bool writeListenAddressXml(QXmlStreamWriter &) const`

<a name="fn_writeListenPortXml"></a>
### `bool writeListenPortXml(QXmlStreamWriter &) const`

<a name="fn_writeCgiBinXml"></a>
### `bool writeCgiBinXml(QXmlStreamWriter &) const`

<a name="fn_writeAllowServingFilesFromCgiBinXml"></a>
### `bool writeAllowServingFilesFromCgiBinXml(QXmlStreamWriter &) const`

<a name="fn_writeAdministratorEmailXml"></a>
### `bool writeAdministratorEmailXml(QXmlStreamWriter &) const`

<a name="fn_writeDefaultConnectionPolicyXml"></a>
### `bool writeDefaultConnectionPolicyXml(QXmlStreamWriter &) const`

<a name="fn_writeDefaultMimeTypeXml"></a>
### `bool writeDefaultMimeTypeXml(QXmlStreamWriter &) const`

<a name="fn_writeAllowDirectoryListingsXml"></a>
### `bool writeAllowDirectoryListingsXml(QXmlStreamWriter &) const`

<a name="fn_writeShowHiddenFilesInDirectoryListingsXml"></a>
### `bool writeShowHiddenFilesInDirectoryListingsXml(QXmlStreamWriter &) const`

<a name="fn_writeDirectoryListingSortOrderXml"></a>
### `bool writeDirectoryListingSortOrderXml(QXmlStreamWriter &) const`

<a name="fn_writeIpConnectionPoliciesXml"></a>
### `bool writeIpConnectionPoliciesXml(QXmlStreamWriter &) const`

<a name="fn_writeFileExtensionMimeTypesXml"></a>
### `bool writeFileExtensionMimeTypesXml(QXmlStreamWriter &) const`

<a name="fn_writeMimeTypeActionsXml"></a>
### `bool writeMimeTypeActionsXml(QXmlStreamWriter &) const`

<a name="fn_writeMimeTypeCgiExecutablesXml"></a>
### `bool writeMimeTypeCgiExecutablesXml(QXmlStreamWriter &) const`

<a name="fn_writeDefaultActionXml"></a>
### `bool writeDefaultActionXml(QXmlStreamWriter &) const`

<a name="fn_setDefaults"></a>
### `void setDefaults()`

## Public member types

<a name="type_MimeTypeList"></a>
### `MimeTypeList`
A list of media types. This is an alias for `std::vector<QString>`.

<a name="type_MimeTypeExtensionMap"></a>
### `MimeTypeExtensionMap`
Maps a file extension to list of media types. This is an alias for `std::map<QString, MimeTypeList>`. It uses an ordered rather than an unordered map so that the model for UI interaction with file extensions can reliably use row indices. See [MimeTypeList](#type_MimeTypeList).

<a name="type_MimeTypeActionMap"></a>
### `MimeTypeActionMap`
Maps media types to actions to take on receipt of requests for resources of that type. This is an alias for `std::unordered_map<QString, WebServerAction>`. See [WebServerAction](webserveraction.md).

<a name="type_MimeTypeCgiMap"></a>
### `MimeTypeCgiMap`
Maps media types to CGI script interpreters. This is an alias for `std::unordered_map<QString, QString>`.

<a name="type_IpConnectionPolicyMap"></a>
### `IpConnectionPolicyMap`
Maps IP addresses to connection policies. This is an alias for `std::unordered_map<QString, [ConnectionPolicy](connectionpolicy.md)>`.


## Public member variables

### `static constexpr const_ uint16_t DefaultPort`
The default listen port.


## Private member variables

### `QString m_listenIp`
The IP address on which to listen for connections.

### `int m_listenPort`
The port on which to listen for connections.

### `std::unordered_map<QString, QString> m_documentRoot`
Maps document root per platform. Enables sharing of configs between platforms with only platform-specific items like paths not being shared.

### `QString m_adminEmail`
The email address of the server administrator.

### `IpConnectionPolicyMap m_ipConnectionPolicies`
The ip-specific connection policies. See [IpConnectionPolicyMap](#type_IpConnectionPolicyMap).

### `MimeTypeExtensionMap m_extensionMimeTypes`
Media types for extensions. See [MimeTypeExtensionMap](#type_MimeTypeExtensionMap).

### `MimeTypeActionMap m_mimeActions`
Actions for Media types. See [MimeTypeActionMap](#type_MimeTypeActionMap).

### `MimeTypeCgiMap m_mimeCgiExecutables`
CGI scripts for Media types. See [MimeTypeCgiMap](#type_MimeTypeCgiMap).

### `std::unordered_map<QString, QString> m_cgiBin`
Maps the CGI exe directory per platform. Enables sharing of configs between platforms with only platform-specific items like paths not being shared.

### `bool m_allowServingFromCgiBin`
Whether or not files from inside the configured CGI binary directory can be served verbatim.

### `ConnectionPolicy m_defaultConnectionPolicy`
The default connection policy to use if an IP address is not specifically controlled. See [ConnectionPolicy](connectionpolicy.md).

### `QString m_defaultMimeType`
The default MIME type to use for unrecognised resource extensions.

### `WebServerAction m_defaultAction`
The default action to use when no specific action is set for a MIME type. See [WebServerAction](webserveraction.md).

### `int m_cgiTimeout`
The timeout, in msec, for CGI execution.

### `bool m_allowDirectoryListings`
Whether or not the server allows directory listings to be sent.

### `bool m_showHiddenFilesInDirectoryListings`
whether or not hidden files are available if directory listings are allowed

### `DirectoryListingSortOrder m_directoryListingSortOrder`
The order in which files and directories appear in a generated directory listing. See [DirectoryListingSortOrder](directorylistingsortorder.md).
