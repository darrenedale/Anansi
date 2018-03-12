# Anansi Web Server

The Anansi web server offers a simple web server with a comfortable GUI that is inteded for site developers to use to test their sites. It provides some security options and tries not to be inefficient, but it does not provide comprehensive security features and has not been robustly tested. It should **never be used for publicly-accessible sites** - use apache, nginx or another web server that has been written with security and performance in mind, and has had far more eyes scrutinise it than Anansi has.

The server features control over media type associations, which (and how) media types are served and a basic CGI 1.1 environment for providing, for example, the ability to run PHP scripts. It can also provide directory listings when the resource requested is a directory.

Some basic security features are provided: direct access to cgi-bin (if it is inside the document root) can be forbidden; listing of hidden files can be suppressed; and direct execution of files within the document root is always forbidden (except in cases where cgi-bin is inside the document root and a valid cgi-bin request is received). It also always warns you if you've set the server to listen on a public IP address or if you've set the cgi-bin to a directory inside your document root. The default configuration switches on all the security features provided.

There are a number of security features that are not provided. The server does not check file permissions. If the server process is able to read a file, it will allow access to it (subject to the media type restrictions configured in the GUI). It also does not provide per-subdirectory restrictions on access to content: if a request is made for a resource of a media type that the server is configured to serve, it will be served.

Basically, **do not use this software for anything important**. It is intended as a development tool only.

## CGI

A basic CGI 1.1 environment is implemented. This is inefficient and prone to security issues since, for every CGI request, a new process on the host is started, run to completion and destroyed. But it is useful in the right circumstances for basic prototyping and in-development testing. The CGI implementation has not been rigorously tested and may diverge from the standard.

There are two ways to set up the server to work with CGI. You should familiarise yourself with the security implications of CGI in general, and both of the following approaches in particular, before using the CGI features of Anansi.

The first is to place your CGI scripts in a subdirectory (outside your document root) and set this as the CGI bin directory in the Anansi GUI. Any requests for `/cgi-bin/myscript` will then be redirected to execute the _myscript_ file from within your configured CGI bin directory. You can place any file that is executable within the directory, and it will be executed upon request. You *must* ensure that the directory you configure is secure, including (but not limited to) knowing that all users of the machine who can write to it are trustworthy and that it does not permit remote access, especially not remote write access, (e.g. via FTP). Ideally, nobody else would have access to these files.

The second is to place your scripts (e.g. PHP, python, ruby) somewhere inside your document root and configure Anansi to recognise the files as an appropriate media type (e.g. _application/x-php_, _application/x-python_, _application/x-ruby_) and to serve those media types through the CGI environment using a specified interpreter (e.g. `/usr/bin/php-cgi`, ...). This method does not allow for direct-execution of files because a valid script interpreter is required (so, for example, you couldn't simply place a binary executable in your document root and set it to be served through CGI). This is slightly more secure than the _cgi-bin_ approach, but still requires you to be vigilant about access to your document root.

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
