# Anansi Web Server

The Anansi web server offers a simple HTTP/1.1 web server with a GUI, and is inteded primarily as a programming exercise for the author and for site developers to use to test their sites. The two required HTTP/1.1 methods - _GET_ and _HEAD_ - are both implemented. It also implements the optional _POST_ method. No other methods are implemented. It also provides a basic CGI/1.1 environment. Neither the HTTP nor the CGI implementations have been rigorously tested to be standards compliant. Hints and pull requests welcome ;) Anansi provides some basic security options and tries not to be inefficient, but it does not provide comprehensive security features and has not been robustly tested. It should **never be used for publicly-accessible sites** let alone for production. There are numerous far superior options readily available.

Three content encodings are supported:
- Identity
- GZip
- Deflate

The content encoding selected depends on the value of the _accept-encoding_ request header. Anansi does a relatively poor (read: quickly cobbled together) job of validating and parsing this header but it should always select the encoding with the highest preference that the client has requested. The GZip and Deflate encodings are supported using [https://zlib.net/](zlib). It is rare that a client sends a HTTP request that does not include the _Identity_ encoding, at least for requests that aren't part of a custom application's protocol, so Anansi should be able to service all requests it receives.

## CGI

A basic CGI 1.1 environment is implemented. CGI is old, inefficient and prone to security issues. For every CGI request, a new process on the host is started, run to completion, and destroyed. But it is useful in the right circumstances for basic prototyping and in-development testing. You should familiarise yourself with the security implications of CGI in general, and both of the approaches Anansi takes (see below) in particular, before using the CGI features of Anansi. CGI has long since been superseded by FCGI and other server-side technologies. A future update may include support for FCGI.

There are two ways to set up Anansi to work with CGI:

- The first method, which is **not** recommended, is to place all your CGI scripts in a subdirectory (_outside_ your document root) and set this as the CGI bin directory in the Anansi GUI. Any requests for `/cgi-bin/myscript` will then be redirected to execute the _myscript_ file from within your configured CGI bin directory. You can place any file that is executable within the directory, and it will be executed upon request. You *must* ensure that the directory you configure is secure, including (but not limited to) knowing that all users of the machine who can write to it are trustworthy and that it does not permit remote access, especially not remote write access, (e.g. via FTP). Ideally, nobody else would have access to these files.

- The second is to place your scripts (e.g. PHP, python, ruby) somewhere inside your document root and configure Anansi to recognise the files as an specific media type (e.g. _application/x-php_, _application/x-python_, _application/x-ruby_) and to serve those media types through the CGI environment using a specified interpreter (e.g. `/usr/bin/php-cgi`, ...). This method does not allow for direct-execution of files because a valid script interpreter is required (so, for example, you couldn't simply place a binary executable in your document root and set it to be served through CGI). This is slightly more secure than the _cgi-bin_ approach, but still requires you to be vigilant about access to your document root.

Some basic security features are provided: access to CGI scripts is forbidden by default but can be overridden; listing of hidden files can be suppressed; and direct execution of files within the document root is always forbidden (except in cases where a valid cgi-bin request is received and cgi-bin is inside the document root, which is very strongly **not** recommended). Anansi also always warns you if you've set the server to listen on a public IP address or if you've set the cgi-bin to a directory inside your document root.

There are a number of security features not provided. Anansi does not check file permissions: if the server process is able to read a file, it will allow access to it (subject to the media type restrictions configured in the UI, see below). It also does not provide per-subdirectory restrictions on access to content: if a request is made for a resource of a media type that the server is configured to serve, it will be served. It does not implement SSL, so plain HTTP is all you get.

Basically, **do not use this software for anything important**. It is intended as a development tool only.

## Configuration

The configuration GUI (and a good portion of the backend) uses Qt5. It definitely uses features that were introduced in Qt 5.7 so that version is an absolute minimum; it might use newer features, I've yet to do a full audit.

The UI is split into four sections, three of which control the server's settings. The first section (Server details) features controls for the basic web server settings such as the document root (where files are served from), the address and port on which to listen and where (if anywhere) the server's cgi-bin directory is. Anansi will warn you if you set the server to listen on a public IP address and if you set the _cgi-bin_ to a directory inside your document root.

The second section (Access control) sets rules for which IP addresses are allowed to connect to the server. You are strongly recommended to set a default policy of _Reject_ and to accept connections only from the IP addresses you know are acceptable.

The third section (Content control) sets associations between files and media types (formerly known as MIME types) and sets actions for media types. The file associations tell Anansi what sort of content is in different files and the actions tell Anansi what to do with different sorts of content. File associations are based on filename extensions mapping to one or more media types; each media type can have exactly one action associated with it. Anansi will process the media types for a file in the order in which they are shown in the UI until it finds an action other than _Ignore_ for the media type.

This section also indicates whether Anansi's directory listings feature is turned on or off, and how it operates.

## Access log

The fourth section of the UI is the access log. This contains one line per connection attempt, and one line per resource requested. In both cases, the log lists the source IP address and port and the action that Anansi took as a result. In the case of requests for resources, the path of the requested resource is also listed.

## Icons

Some icons from the KDE Oxygen icons project are used under the LGPL v3, the text of which is included with this application. As required by the license, the icons themselves are also distributed. The license text and icons can be found in the following platform-dependent locations:

#### OSX
- icons/ directory on the disk image
- lgpl-3.0.txt on the disk image

#### Linux
- /usr/share/doc/equitwebserver/lgpl-3.0.txt
- /usr/share/equit/equitwebserver/icons.tar.gz

#### Windows
- icons/ folder in the application installation folder
- lgpl-3.0.txt in the application installation folder
