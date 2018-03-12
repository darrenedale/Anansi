# Class: Configuration

Encapsulates the configuration of the web server. The configuration specifies
simple features such as the address and port on which the server listens, the
document root from which the server will serve content, to more complex setup
such as which connections are accepted and which are rejected, how file name
extensions map to media types, what the server should do with different media
types (e.g. serve, forbid).


## Detailed description

### Core server details

The address on which the server listens is set using
[setListenAddress()](#setListenAddress) and is queried using
[listenAddress()](#listenAddress). Any valid IP address will be accepted,
regardless of whether it is one assigned to the host on which the server is
running, and regardless of whether it is in a reserved range. This does not
mean the server will actually listen on that address - when started the server
will fail to bind to an address that is not available on the host on which it
is running. The port is set using [setPort()](#setPort) and queried using
[port()](#port). The port must be greater than 0 and less than 65535; an other
port number is invalid. Again, a valid port will be accepted in the
configuration regardless of whether that port is actually available. Starting
the server will fail if the port is not available at that time.

The server can be configured to show a directory listing when a request is made
for a resource that maps to a directory rather than a file. This is set using
[setDirectoryListingsAllowed()](#setDirectoryListingsAllowed) and queried using
[directoryListingsAllowed()](#directoryListingsAllowed). If directory listings
are available, it is possible to set whether or not hidden files on the host
are listed or hidden and the sort order for entries in directory listings.
These are set and queried using
[setShowHiddenFilesInDirectoryListings()](#setShowHiddenFilesInDirectoryListings)
and [showHiddenFilesInDirectoryListings()](#showHiddenFilesInDirectoryListings),
and [setDirectoryListingSortOrder()](#setDirectoryListingSortOrder) and
[directoryListingSortOrder()](#directoryListingSortOrder) respectively.

The [server.md](Server) class implements basic CGI 1.1. The directory in which
it looks for CGI scripts is set using [setCgiBin()](#setCgiBin) and queried
using [cgiBin()](#cgiBin). For security reasons this should be set to a
directory that is outside the document root unless you are certain that the
content (i.e. source) of your CGI scripts is neither private nor dangerous to
download, and that the content of the CGI binary directory cannot be altered by
any untrusted party (e.g. other user accounts on the host, FTP accounts, etc.).
You can configure the server to refuse to serve files that are inside the CGI
binary directory using
[setAllowServingFilesFromCgiBin()](#setAllowServingFilesFromCgiBin) (query the
setting using [allowServingFilesFromCgiBin()](#allowServingFilesFromCgiBin)).
The timeout for CGI script execution is set with
[setCgiTimeout()](#setCgiTimeout) and queried using [cgiTimeout()](#cgiTimeout).


### Connections

The settings governing what happens to incoming connections are managed by
specifying [ConnectionPolicy.md](connection policies) for individual IP
addresses. IP Address ranges or subnets are not (yet) supported. Policies are
set using [setIpAddressConnectionPolicy()](#setIpAddressConnectionPolicy) and
queried using [ipAddressConnectionPolicy()](#ipAddressConnectionPolicy). The
policy for an IP address can be cleared using
[unsetIpAddressConnectionPolicy()](#unsetIpAddressConnectionPolicy). All
policies can be cleared _en-masse_ with
[clearAllIpAddressConnectionPolicies()](#clearAllIpAddressConnectionPolicies).
You can query whether an IP address has a policy registered with
[ipAddressIsRegistered()](#ipAddressIsRegistered) (calling
[ipAddressConnectionPolicy()](#ipAddressConnectionPolicy) is not suitable for
this purpose because it returns `None` for cases where the policy has
explicitly been set to `None` and where no policy has been set).

In addition to IP-specific connection policies, there is a default connection
policy that is used for connections from IP addresses either without an
explictyly-set policy or where the policy has been set explicitly to `None`.
This is set using [setDefaultConnectionPolicy()](#setDefaultConnectionPolicy)
and queried using [defaultConnectionPolicy()](#defaultConnectionPolicy). Note
that where the default connection policy is set to `None`, the
[server.md](Server) will act as if the policy is `Reject`.

The full set of IP addresses for which policies exist can be retrieved using
[registeredIpAddresses()](#registeredIpAddresses); the number of IP addresses
for which explicit policies have been set can be queried using
[registeredIpAddressCount()](#registeredIpAddressCount) - this is faster than
counting the entries in the list returned by
[registeredIpAddresses()](#registeredIpAddresses).


### File type associations

Note: At present, this documentation refers to _media types_, which is the
preferred way to refer to what used to be known as MIME types. The code,
however, has yet to be updated for this change, and still refers to MIME types.
When reading this documentation and the code, the terms _media type_ and _MIME
type_ should be understood to refer to the same thing.

File type associations are handled by specifying a list of media types that can
be used for files whose names end in a given extension. The extension is always
considered to be the part of the file anme after last `.` character. Files whose
names have no `.` character, or whose only `.` character is the first in the
name, are considered to have no extension. The [server.md](Server) processes
the media types for a file extension in order from top to bottom. The first
media type for which it finds an action that is not `Ignore` is used, that
action is executed, and the remaining media types for that extension are not
processed.

A media type can be added for a file extension using
[addFileExtensionMimeType()](#addFileExtensionMimeType). If the provided
extension already has one or more associated media types, the provided media
type is added to its list; otherwise, a the extension is added with a media
type list containing just the provided media type. This is the only way to add
a new extension to the file associations. The list of media types for an
extension can be queried using
[mimeTypesForFileExtension()](#mimeTypesForFileExtension). To query whether an
extension has been registered, use
[fileExtensionIsRegistered()](#fileExtensionIsRegistered); to query whether an
extension has been associated with a particular media type use
[fileExtensionHasMimeType()](#fileExtensionHasMimeType) - this is faster than
retrieving the set of associated media types and examining it. To remove a
media type from a file extension call
[removeFileExtensionMimeType()](#removeFileExtensionMimeType) or to remove all
media types from an extension, and the extension itself, call
[removeFileExtension()](#removeFileExtension). All file extensions can be
removed _en-masse_ using [clearAllFileExtensions()](#clearAllFileExtensions). To
selectively change a media type for an extension from one media type to another
(i.e. effectively to rename one of the media types assocated with a file
extension) call [changeFileExtensionMimeType()](#changeFileExtensionMimeType).
To change a file type association from one extension to another (i.e. to remove
one file extension and replace it with another that has the identical media
type list) call [changeFileExtension()](#changeFileExtension).

The full set of file extensions for which media types have been assigned can be retrieved using
[registeredFileExtensions()](#registeredFileExtensions); the number of extensions
for which media types have been set can be queried using
[registeredFileExtensionCount()](#registeredFileExtensionCount)
- this is faster than counting the entries in the list returned by
[registeredFileExtensions()](#registeredFileExtensions).

The default media type, which will be used when a request is received for a
resource for which a specifically-associated media type has not been set, is
set using [setDefaultMimeType()](#setDefaultMimeType) and queried using
[defaultMimeType()](#defaultMimeType); it can be reset using
[unsetDefaultMimeType()](#unsetDefaultMimeType), which will set the default
media type to its built-in default value (currently
`application/octet-stream`).


### Media type actions

Once the [server.md](Server) has determined the media type to use for a given
request it needs to know what to do with the resource requested. This can be
to serve the local file identified by the request, forbid access to the resource,
pass the resource on be executed through CGI or simply to ignore the request
(see [WebServerAction](webserveraction.md)).

Actions are assigned to media types using
[setMimeTypeAction()](#setMimeTypeAction) and queried using
[mimeTypeAction()](#mimeTypeAction). A media type can have its explicit action
removed using [unsetMimeTypeAction()](#unsetMimeTypeAction); this can be done
for all media types _en-masse_ using
[clearAllMimeTypeActions()](#clearAllMimeTypeActions). A list of the media types
for which explicit actions have been set can be retrieved using
[registeredMimeTypes()](#registeredMimeTypes) (note the difference between this
an [allKnownMimeTypes()](#allKnownMimeTypes) which includes all media types
associated with a file extension regardless of whether an action has been
specified for that media type). The number of media types for which actions
have explicitly been set can be queried using
[registeredMimeTypeCount()](#registeredMimeTypeCount) - this is faster than
countin the entries in the list returned by
[registeredMimeTypes()](#registeredMimeTypes). To query whether a media type has
an explicitly associated action, call
[mimeTypeIsRegistered()](#mimeTypeIsRegistered) - again, this is faster than
calling [registeredMimeTypes()](#registeredMimeTypes) and searching for the
media type.

For media types that are registered to be executed through the CGI, the
interpreter used to execute the script is specified using
[setMimeTypeCgi()](#setMimeTypeCgi) and queried using
[mimeTypeCgi()](#mimeTypeCgi). To remove the interpreter for a media type use
[unsetMimeTypeCgi()](#unsetMimeTypeCgi). Note that for any media type for which
the action is set to `CGI` but whose CGI interpreter is unset or set to an
empty string, the [server.md](Server) will respond to the client with a
_Forbidden_ response - in other words, all CGI content served from within the
document root must be processed with an interpreter explicitly set in the
server configuration. This helps ensure that no CGI content can accidentally be
directly-executed from inside the document root.

The default action, which is used when requests for media types with no
explictly associated action are received, is set using
[setDefaultAction()](#setDefaultAction) and queried using
[defaultAction()](#defaultAction).


## Public constructors

### `Configuration()`

### `Configuration(const QString & docRoot, const QString & listenAddress, int port)`


## Public member functions

<a name="loadFrom" />
### `static std::optional<Configuration> loadFrom(const QString & fileName)`

<a name="save" />
### `bool save(const QString & fileName) const`

<a name="read" />
### `bool read(const QString & fileName)`

<a name="listenAddress" />
### `const QString & listenAddress() const`

<a name="setListenAddress" />
### `bool setListenAddress(const QString & listenAddress)`

<a name="port" />
### `int port() const noexcept`

<a name="setPort" />
### `bool setPort(int port) noexcept`

<a name="documentRoot" />
### `const QString documentRoot(const QString & platform = QStringLiteral()) const`

<a name="setDocumentRoot" />
### `bool setDocumentRoot(const QString & docRoot, const QString & platform = QStringLiteral())`

<a name="administratorEmail" />
### `QString administratorEmail() const`

<a name="setAdministratorEmail" />
### `void setAdministratorEmail(const QString & admin)`

<a name="directoryListingsAllowed" />
### `bool directoryListingsAllowed() const noexcept`

<a name="setDirectoryListingsAllowed" />
### `void setDirectoryListingsAllowed(bool) noexcept`

<a name="showHiddenFilesInDirectoryListings" />
### `bool showHiddenFilesInDirectoryListings() const noexcept`

<a name="setShowHiddenFilesInDirectoryListings" />
### `void setShowHiddenFilesInDirectoryListings(bool) noexcept`

<a name="directoryListingSortOrder" />
### `inline DirectoryListingSortOrder directoryListingSortOrder() const noexcept

<a name="setDirectoryListingSortOrder" />
### `inline void setDirectoryListingSortOrder(DirectoryListingSortOrder sortOrder) noexcept

<a name="cgiBin" />
### `QString cgiBin(const QString & platform = QStringLiteral()) const`

<a name="setCgiBin" />
### `bool setCgiBin(const QString & bin, const QString & platform = QStringLiteral())`

<a name="cgiTimeout" />
### `int cgiTimeout() const noexcept`

<a name="setCgiTimeout" />
### `bool setCgiTimeout(int) noexcept`

<a name="allowServingFilesFromCgiBin" />
### `bool allowServingFilesFromCgiBin() const noexcept`

<a name="setAllowServingFilesFromCgiBin" />
### `void setAllowServingFilesFromCgiBin(bool allow) noexcept`

<a name="registeredIpAddresses" />
### `std::vector<QString> registeredIpAddresses() const`

<a name="registeredFileExtensions" />
### `std::vector<QString> registeredFileExtensions() const`

<a name="registeredMimeTypes" />
### `std::vector<QString> registeredMimeTypes() const`

<a name="allKnownMimeTypes" />
### `std::vector<QString> allKnownMimeTypes() const`

<a name="registeredIpAddressCount" />
### `inline int registeredIpAddressCount() const noexcept`

<a name="registeredFileExtensionCount" />
### `inline int registeredFileExtensionCount() const noexcept`

<a name="registeredMimeTypeCount" />
### `inline int registeredMimeTypeCount() const noexcept`

<a name="defaultConnectionPolicy" />
### `ConnectionPolicy defaultConnectionPolicy() const noexcept`

<a name="setDefaultConnectionPolicy" />
### `void setDefaultConnectionPolicy(ConnectionPolicy) noexcept`

<a name="ipAddressIsRegistered" />
### `bool ipAddressIsRegistered(const QString & addr) const`

<a name="ipAddressConnectionPolicy" />
### `ConnectionPolicy ipAddressConnectionPolicy(const QString & addr) const`

<a name="setIpAddressConnectionPolicy" />
### `bool setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy p)`

<a name="unsetIpAddressConnectionPolicy" />
### `bool unsetIpAddressConnectionPolicy(const QString & addr)`

<a name="clearAllIpAddressConnectionPolicies" />
### `void clearAllIpAddressConnectionPolicies()`

<a name="fileExtensionIsRegistered" />
### `bool fileExtensionIsRegistered(const QString & ext) const`

<a name="fileExtensionHasMimeType" />
### `bool fileExtensionHasMimeType(const QString & ext, const QString & mime) const`

<a name="changeFileExtensionMimeType" />
### `bool changeFileExtensionMimeType(const QString & ext, const QString & fromMime, const QString & toMime)`

<a name="addFileExtensionMimeType" />
### `bool addFileExtensionMimeType(const QString & ext, const QString & mime)`

<a name="removeFileExtensionMimeType" />
### `void removeFileExtensionMimeType(const QString & ext, const QString & mime)`

<a name="changeFileExtension" />
### `bool changeFileExtension(const QString & oldExt, const QString & newExt)`

<a name="removeFileExtension" />
### `inline void removeFileExtension(const QString & ext)`

<a name="fileExtensionMimeTypeCount" />
### `int fileExtensionMimeTypeCount(const QString & ext) const`

<a name="mimeTypesForFileExtension" />
### `MimeTypeList mimeTypesForFileExtension(const QString & ext) const`

<a name="clearAllFileExtensions" />
### `void clearAllFileExtensions()`

<a name="defaultMimeType" />
### `QString defaultMimeType() const`

<a name="setDefaultMimeType" />
### `void setDefaultMimeType(const QString & mime)`

<a name="unsetDefaultMimeType" />
### `void unsetDefaultMimeType()`

<a name="mimeTypeIsRegistered" />
### `bool mimeTypeIsRegistered(const QString & mime) const`

<a name="mimeTypeAction" />
### `WebServerAction mimeTypeAction(const QString & mime) const`

<a name="setMimeTypeAction" />
### `bool setMimeTypeAction(const QString & mime, const WebServerAction & action)`

<a name="unsetMimeTypeAction" />
### `void unsetMimeTypeAction(const QString & mime)`

<a name="clearAllMimeTypeActions" />
### `void clearAllMimeTypeActions()`

<a name="defaultAction" />
### `WebServerAction defaultAction() const`

<a name="setDefaultAction" />
### `void setDefaultAction(const WebServerAction & action)`

<a name="mimeTypeCgi" />
### `QString mimeTypeCgi(const QString & mime) const`

<a name="setMimeTypeCgi" />
### `void setMimeTypeCgi(const QString & mime, const QString & cgiExe)`

<a name="unsetMimeTypeCgi" />
### `void unsetMimeTypeCgi(const QString & mime)`


## Private member functions


## Public member types


### `MimeTypeList`

A list of media types. This is an alias for `std::vector<QString>`.


<a name="MimeTypeExtensionMap" />
### `MimeTypeExtensionMap`

Maps a file extension to list of media types. This is an alias for
`std::map<QString, MimeTypeList>`. It uses an ordered rather than an unordered
map so that the model for UI interaction with file extensions can reliably use
row indices


<a name="MimeTypeActionMap" />
### `MimeTypeActionMap`

Maps media types to actions to take on receipt of requests for resources of
that type. This is an alias for `std::unordered_map<QString, [WebServerAction](webserveraction.md)>`.


<a name="MimeTypeCgiMap" />
### `MimeTypeCgiMap`

Maps media types to CGI script interpreters. This is an alias for
`std::unordered_map<QString, QString>`.


<a name="IpConnectionPolicyMap" />
### `IpConnectionPolicyMap`

Maps IP addresses to connection policies. This is an alias for
`std::unordered_map<QString, [ConnectionPolicy](connectionpolicy.md)>`.


## Public member variables


### `static constexpr const_ uint16_t DefaultPort`


## Private member variables


### `QString m_listenIp`


### `int m_listenPort`


### `std::unordered_map<QString, QString> m_documentRoot`

Maps document root per platform. Enables sharing of configs between platforms
with only platform-specific items like paths not being shared.


### `QString m_adminEmail`

The email address of the server administrator.


### `[IpConnectionPolicyMap](#IpConnectionPolicyMap) m_ipConnectionPolicies`

The ip-specific connection policies


### `[MimeTypeExtensionMap](#MimeTypeExtensionMap) m_extensionMimeTypes`

Media types for extensions


### `[MimeTypeActionMap](#MimeTypeActionMap) m_mimeActions`

Actions for Media types


### `[MimeTypeCgiMap](#MimeTypeCgiMap) m_mimeCgiExecutables`

CGI scripts for Media types


### `std::unordered_map<QString, QString> m_cgiBin`

Maps the CGI exe directory per platform. Enables sharing of configs between
platforms with only platform-specific items like paths not being shared.


### `bool m_allowServingFromCgiBin`

### `(ConnectionPolicy)[connectionpolicy.md] m_defaultConnectionPolicy`

The default connection policy to use if an IP address is not specifically
controlled


### `QString m_defaultMimeType`

The default MIME type to use for unrecognised resource extensions.


### `[WebServerAction](webserveraction.md) m_defaultAction`

The default action to use when no specific action is set for a MIME type


### `int m_cgiTimeout`

The timeout, in msec, for CGI execution.


### `bool m_allowDirectoryListings`

Whether or not the server allows directory listings to be sent.


### `bool m_showHiddenFilesInDirectoryListings`

whether or not hidden files are available if directory listings are allowed


### `[DirectoryListingSortOrder](directorylistingsortorder.md) m_directoryListingSortOrder`

The order in which files and directories appear in a generated directory
listing.
