Equit Web Server
================

The equit web server offers a simple web server with a comfortable GUI
that is inteded for site developers to use to test their sites. It
provides some security options but should never be used for publicly-
accessible sites.

The server features control over MIME type associations, which (and how) MIME
types are served and a functional CGI environment for providing, for example,
the ability to run PHP scripts. It also provides directory listings when the
resource requested is a directory.

The server does not check file permissions. If the server process is able to
read a file, it will allow access to it, subject to the MIME type restrictions
configured in the GUI.

The server currently always provides directory listings if requested (providing
the directory exists within the document root, of course), although an option to
switch this on and off will be provided in the GUI soon. It also always omits
"hidden" files from directory listings. (A hidden file is any file whose name
starts with a ".".) Again, this will be configurable from the GUI soon.


Icons
=====

Some icons from the KDE Oxygen icons project are used under the LGPL v3,
the text of which is included with this application. As required by the license,
the icons themselves are also distributed. The license text and icons can be
found in the following platform-dependent locations:

  OSX
  ---
  icons/ directory on the disk image
  lgpl-3.0.txt on the disk image

  Linux
  -----
  /usr/share/doc/equitwebserver/lgpl-3.0.txt
  /usr/share/equit/equitwebserver/icons.tar.gz

  Windows
  -------
  icons/ folder in the application installation folder
  lgpl-3.0.txt in the application installation folder
