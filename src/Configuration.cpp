/** \file Configuration.cpp
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the Configuration class for EquitWebServer.
  *
  * \todo
  * - decide on application license
  *
  * \par Current Changes
  * - (2012-06-19) fixed parsing of XML stream where allowdirectorylistings
  *   element would be overlooked.
  * - (2012-06-19) file documentation created.
  */

#include "Configuration.h"

#include <QDebug>
#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


#define EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTACTION Forbid
#define EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTCONNECTIONPOLICY AcceptConnection
#define EQUITWEBSERVERCONFIGURATION_DEFAULTBINDADDRESS "127.0.0.1"
#define EQUITWEBSERVERCONFIGURATION_DEFAULTBINDPORT 80
#define EQUITWEBSERVERCONFIGURATION_DEFAULTCGITIMEOUT 30000
#define EQUITWEBSERVERCONFIGURATION_DEFAULTALLOWDIRLISTS true

/*
** lower-case platform strings for use when attempting to preserve paths in config files across platforms
*/
#if defined(Q_OS_LINUX)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "linux"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_WIN32)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "win32"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_MACX)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "osx"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/Sites")
#elif defined(Q_OS_FREEBSD)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "freebsd"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_OS2)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "os2"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_SOLARIS)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "solaris"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#elif defined(Q_OS_UNIX)
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "unix"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#else
#define EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING "undefined"
#define EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT (QDir::homePath() + "/public_html")
#endif


const int EquitWebServer::Configuration::DefaultPort = EQUITWEBSERVERCONFIGURATION_DEFAULTBINDPORT;

EquitWebServer::Configuration::Configuration(void)
: m_defaultConnectionPolicy(EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTCONNECTIONPOLICY),
  m_defaultAction(EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTACTION),
  m_cgiTimeout(EQUITWEBSERVERCONFIGURATION_DEFAULTCGITIMEOUT),
  m_allowDirectoryListings(EQUITWEBSERVERCONFIGURATION_DEFAULTALLOWDIRLISTS) {
	setDefaults();
}


EquitWebServer::Configuration::Configuration(const QString & docRoot, const QString & listenAddress, int port)
: m_defaultConnectionPolicy(EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTCONNECTIONPOLICY),
  m_defaultAction(EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTACTION),
  m_cgiTimeout(EQUITWEBSERVERCONFIGURATION_DEFAULTCGITIMEOUT),
  m_allowDirectoryListings(EQUITWEBSERVERCONFIGURATION_DEFAULTALLOWDIRLISTS) {
	setDefaults();
	setDocumentRoot(docRoot);
	setListenAddress(listenAddress);
	setPort(port);
}


bool EquitWebServer::Configuration::load(const QString & fileName) {
	if(fileName.isEmpty())
		return false;
	QFile f(fileName);
	f.open(QIODevice::ReadOnly);
	if(!f.isOpen())
		return false;

	//bpWebServer::Configuration loading;
	QXmlStreamReader xml(&f);

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isStartElement()) {
			if(xml.name() == "webserver")
				parseWebserverXML(xml);
			else
				xml.readElementText();
		}
	}

	return true;
}


bool EquitWebServer::Configuration::parseWebserverXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "webserver");

	bool ret = true;

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		//		qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - TokenType: " << xml.tokenType() << "  ::  StartElement TokenType: " << QXmlStreamReader::StartElement;
		//		qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - parsing XML element \"" << qPrintable(xml.name().toString()) << "\"";

		if(xml.name() == "documentroot")
			ret = parseDocumentRootXML(xml);
		else if(xml.name() == "bindaddress")
			ret = parseListenAddressXML(xml);
		else if(xml.name() == "bindport")
			ret = parseListenPortXML(xml);
		else if(xml.name() == "defaultconnectionpolicy")
			ret = parseDefaultConnectionPolicyXML(xml);
		else if(xml.name() == "defaultmimetype")
			ret = parseDefaultMIMETypeXML(xml);
		else if(xml.name() == "defaultmimetypeaction")
			ret = parseDefaultActionXML(xml);
		else if(xml.name() == "ipconnectionpolicylist")
			ret = parseIPConnectionPoliciesXML(xml);
		else if(xml.name() == "extensionmimetypelist")
			ret = parseFileExtensionMIMETypesXML(xml);
		else if(xml.name() == "mimetypeactionlist")
			ret = parseMIMETypeActionsXML(xml);
		else if(xml.name() == "mimetypecgilist")
			ret = parseMIMETypeCGIExecutablesXML(xml);
		else if(xml.name() == "allowdirectorylistings")
			ret = parseAllowDirectoryListingsXML(xml);
		else
			parseUnknownElementXML(xml);
	}

	return ret;
}


void EquitWebServer::Configuration::parseUnknownElementXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement());
	qDebug() << "bpWebServer::parseXML() - unknown element \"" << xml.name().toString().toUtf8().constData() << "\"";

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.isStartElement())
			parseUnknownElementXML(xml);
	}
}


bool EquitWebServer::Configuration::parseDocumentRootXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "documentroot");

	QXmlStreamAttributes attrs = xml.attributes();

	if(attrs.value("platform").isEmpty()) {
		/* for legacy compatability, the current platform will be used if it has not been
		** set already from the config file in cases where the config file documentroot
		** element does not have a "platform" attribute. if the current platform is provided
		** with a specific document root later in the config file, the specific one will overwrite
		** the assumed one used here. when writing back out, the platform attribute is always
		** written
		*/
		if(m_documentRoot.contains(EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING)) {
			/* just ignore it if the platform docroot is already set */
			xml.readElementText();
			return true;
		}

		attrs.append("platform", EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING);
	}

	setDocumentRoot(xml.readElementText(), attrs.value("platform").toString());
	return true;
}


bool EquitWebServer::Configuration::parseListenAddressXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "bindaddress");

	setListenAddress(xml.readElementText());
	return true;
}


bool EquitWebServer::Configuration::parseListenPortXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "bindport");

	setPort(xml.readElementText().toInt());
	return true;
}


bool EquitWebServer::Configuration::parseDefaultConnectionPolicyXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "defaultconnectionpolicy");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "connectionpolicy")
			setDefaultConnectionPolicy(parseConnectionPolicyText(xml.readElementText()));
		else
			parseUnknownElementXML(xml);
	}

	return true;
}


EquitWebServer::Configuration::ConnectionPolicy EquitWebServer::Configuration::parseConnectionPolicyText(const QString & policy) {
	if(policy == "RejectConnection")
		return RejectConnection;
	else if(policy == "AcceptConnection")
		return AcceptConnection;
	return NoConnectionPolicy;
}


bool EquitWebServer::Configuration::parseDefaultMIMETypeXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "defaultmimetype");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "mimetype")
			setDefaultMIMEType((xml.readElementText()));
		else
			parseUnknownElementXML(xml);
	}

	return true;
}


bool EquitWebServer::Configuration::parseDefaultActionXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "defaultmimetypeaction");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "webserveraction")
			setDefaultAction(parseActionText(xml.readElementText()));
		else
			parseUnknownElementXML(xml);
	}

	return true;
}


EquitWebServer::Configuration::WebServerAction EquitWebServer::Configuration::parseActionText(const QString & action) {
	if(action == "Forbid")
		return Forbid;
	else if(action == "Serve")
		return Serve;
	else if(action == "CGI")
		return CGI;
	return Ignore;
}


bool EquitWebServer::Configuration::parseBooleanText(const QString & boolean, bool def = false) {
	if(boolean.trimmed().toLower() == "true")
		return true;
	else if(boolean.trimmed().toLower() == "false")
		return false;
	return def;
}


bool EquitWebServer::Configuration::parseAllowDirectoryListingsXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "allowdirectorylistings");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "allowdirectorylistings")
			setAllowDirectoryListing(parseBooleanText(xml.readElementText(), false));
		else
			parseUnknownElementXML(xml);
	}

	return true;
}


bool EquitWebServer::Configuration::parseIPConnectionPoliciesXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "ipconnectionpolicylist");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "ipconnectionpolicy")
			parseIPConnectionPolicyXML(xml);
		else
			parseUnknownElementXML(xml);
	}
	return true;
}


bool EquitWebServer::Configuration::parseIPConnectionPolicyXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "ipconnectionpolicy");

	QString ipAddress, policy;

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "ipaddress")
			ipAddress = xml.readElementText();
		else if(xml.name() == "connectionpolicy")
			policy = xml.readElementText();
		else
			parseUnknownElementXML(xml);
	}

	setIPAddressPolicy(ipAddress, parseConnectionPolicyText(policy));
	return true;
}


bool EquitWebServer::Configuration::parseFileExtensionMIMETypesXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "extensionmimetypelist");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "extensionmimetype")
			parseFileExtensionMIMETypeXML(xml);
		else
			parseUnknownElementXML(xml);
	}

	return true;
}


bool EquitWebServer::Configuration::parseFileExtensionMIMETypeXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "extensionmimetype");

	QString ext;
	QStringList mimes;

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "extension")
			ext = xml.readElementText();
		else if(xml.name() == "mimetype")
			mimes << xml.readElementText();
		else
			parseUnknownElementXML(xml);
	}

	if(mimes.count() > 0) {
		foreach(QString mime, mimes)
			addFileExtensionMIMEType(ext, mime);
	}

	return true;
}


bool EquitWebServer::Configuration::parseMIMETypeActionsXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypeactionlist");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "mimetypeaction")
			parseMIMETypeActionXML(xml);
		else
			parseUnknownElementXML(xml);
	}

	return true;
}


bool EquitWebServer::Configuration::parseMIMETypeActionXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypeaction");

	QString mime, action;

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "mimetype")
			mime = xml.readElementText();
		else if(xml.name() == "webserveraction")
			action = xml.readElementText();
		else
			parseUnknownElementXML(xml);
	}

	setMIMETypeAction(mime, parseActionText(action));
	return true;
}


bool EquitWebServer::Configuration::parseMIMETypeCGIExecutablesXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypecgilist");

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "mimetypecgi")
			parseMIMETypeCGIExecutableXML(xml);
		else
			parseUnknownElementXML(xml);
	}

	return true;
}


bool EquitWebServer::Configuration::parseMIMETypeCGIExecutableXML(QXmlStreamReader & xml) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "mimetypecgi");

	QString mime, exe;

	while(!xml.atEnd()) {
		xml.readNext();

		if(xml.isEndElement())
			break;

		if(xml.isCharacters()) {
			if(!xml.isWhitespace())
				qDebug() << "bpWebServer::bpWebServer::Configuration::parseWebserverXML() - ignoring extraneous non-whitespace content at line " << xml.lineNumber();

			/* ignore extraneous characters */
			continue;
		}

		if(xml.name() == "mimetype")
			mime = xml.readElementText();
		else if(xml.name() == "cgiexecutable")
			exe = xml.readElementText();
		else
			parseUnknownElementXML(xml);
	}

	setMIMETypeCGI(mime, exe);
	return true;
}


bool EquitWebServer::Configuration::save(const QString & fileName) const {
	if(fileName.isEmpty())
		return false;

	QFile f(fileName);
	f.open(QIODevice::WriteOnly);
	if(!f.isOpen())
		return false;
	QXmlStreamWriter xml(&f);
	xml.setAutoFormatting(true);
	bool ret = startXML(xml) && writeXML(xml) && endXML(xml);
	f.close();
	return ret;
}


bool EquitWebServer::Configuration::startXML(QXmlStreamWriter & xml) const {
	xml.writeStartDocument();
	xml.writeStartElement("webserver");
	return true;
}


bool EquitWebServer::Configuration::endXML(QXmlStreamWriter & xml) const {
	xml.writeEndElement();
	xml.writeEndDocument();
	return true;
}


bool EquitWebServer::Configuration::writeXML(QXmlStreamWriter & xml) const {
	documentRootXML(xml);
	listenAddressXML(xml);
	listenPortXML(xml);
	defaultConnectionPolicyXML(xml);
	defaultMIMETypeXML(xml);
	defaultActionXML(xml);
	allowDirectoryListingsXML(xml);
	ipConnectionPoliciesXML(xml);
	fileExtensionMIMETypesXML(xml);
	mimeTypeActionsXML(xml);
	mimeTypeCGIExecutablesXML(xml);
	return true;
}


bool EquitWebServer::Configuration::documentRootXML(QXmlStreamWriter & xml) const {
	foreach(QString platform, m_documentRoot.keys()) {
		xml.writeStartElement("documentroot");
		xml.writeAttribute("platform", platform);
		xml.writeCharacters(m_documentRoot[platform]);
		xml.writeEndElement();
	}
	return true;
}


bool EquitWebServer::Configuration::listenAddressXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("bindaddress");
	xml.writeCharacters(m_listenIP);
	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::listenPortXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("bindport");
	xml.writeCharacters(QString::number(m_listenPort));
	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::defaultConnectionPolicyXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("defaultconnectionpolicy");
	xml.writeStartElement("connectionpolicy");

	switch(m_defaultConnectionPolicy) {
		default:
		case NoConnectionPolicy:
			xml.writeCharacters("NoConnectionPolicy");
			break;

		case RejectConnection:
			xml.writeCharacters("RejectConnection");
			break;

		case AcceptConnection:
			xml.writeCharacters("AcceptConnection");
			break;
	}

	xml.writeEndElement();
	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::defaultMIMETypeXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("defaultmimetype");
	xml.writeStartElement("mimetype");
	xml.writeCharacters(m_defaultMIMEType);
	xml.writeEndElement();
	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::allowDirectoryListingsXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("allowdirectorylistings");
	xml.writeCharacters(m_allowDirectoryListings ? "true" : "false");
	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::ipConnectionPoliciesXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("ipconnectionpolicylist");

	foreach(QString ip, m_ipConnectionPolicy.keys()) {
		xml.writeStartElement("ipconnectionpolicy");
		xml.writeStartElement("ipaddress");
		xml.writeCharacters(ip);
		xml.writeEndElement();
		xml.writeStartElement("connectionpolicy");

		switch(m_ipConnectionPolicy[ip]) {
			default:
			case NoConnectionPolicy:
				xml.writeCharacters("NoConnectionPolicy");
				break;

			case RejectConnection:
				xml.writeCharacters("RejectConnection");
				break;

			case AcceptConnection:
				xml.writeCharacters("AcceptConnection");
				break;
		}

		xml.writeEndElement();
		xml.writeEndElement();
	}

	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::fileExtensionMIMETypesXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("extensionmimetypelist");

	foreach(QString ext, m_extensionMIMETypes.keys()) {
		xml.writeStartElement("extensionmimetype");
		xml.writeStartElement("extension");
		xml.writeCharacters(ext);
		xml.writeEndElement();

		foreach(QString mime, m_extensionMIMETypes[ext]) {
			xml.writeStartElement("mimetype");
			xml.writeCharacters(mime);
			xml.writeEndElement();
		}

		xml.writeEndElement();
	}

	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::mimeTypeActionsXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("mimetypeactionlist");

	foreach(QString mime, m_mimeActions.keys()) {
		xml.writeStartElement("mimetypeaction");
		xml.writeStartElement("mimetype");
		xml.writeCharacters(mime);
		xml.writeEndElement();

		xml.writeStartElement("webserveraction");

		switch(m_mimeActions[mime]) {
			default:
			case Ignore:
				xml.writeCharacters("Ignore");
				break;

			case Serve:
				xml.writeCharacters("Serve");
				break;

			case CGI:
				xml.writeCharacters("CGI");
				break;

			case Forbid:
				xml.writeCharacters("Forbid");
				break;
		}

		xml.writeEndElement();
		xml.writeEndElement();
	}

	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::mimeTypeCGIExecutablesXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("mimetypecgilist");

	foreach(QString mime, m_mimeCGI.keys()) {
		xml.writeStartElement("mimetypecgi");
		xml.writeStartElement("mimetype");
		xml.writeCharacters(mime);
		xml.writeEndElement();
		xml.writeStartElement("cgiexecutable");
		xml.writeCharacters(m_mimeCGI[mime]);
		xml.writeEndElement();
		xml.writeEndElement();
	}

	xml.writeEndElement();
	return true;
}


bool EquitWebServer::Configuration::defaultActionXML(QXmlStreamWriter & xml) const {
	xml.writeStartElement("defaultmimetypeaction");
	xml.writeStartElement("webserveraction");

	switch(m_defaultAction) {
		default:
		case Ignore:
			xml.writeCharacters("Ignore");
			break;

		case Serve:
			xml.writeCharacters("Serve");
			break;

		case CGI:
			xml.writeCharacters("CGI");
			break;

		case Forbid:
			xml.writeCharacters("Forbid");
			break;
	}

	xml.writeEndElement();
	xml.writeEndElement();
	return true;
}


void EquitWebServer::Configuration::setInvalid(void) {
	setInvalidDocumentRoot();
	setInvalidListenAddress();
	setInvalidListenPort();
}


void EquitWebServer::Configuration::setInvalidDocumentRoot(const QString & platform) {
	if(m_documentRoot.contains(platform))
		m_documentRoot[platform] = QString::null;
	else
		m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING] = QString::null;
}


void EquitWebServer::Configuration::setInvalidListenAddress(void) {
	m_listenIP = QString::null;
}


void EquitWebServer::Configuration::setInvalidListenPort(void) {
	m_listenPort = -1;
}


void EquitWebServer::Configuration::setDefaults(void) {
	m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING] = EQUITWEBSERVERCONFIGURATION_INITIALDOCUMENTROOT;
	m_listenIP = EQUITWEBSERVERCONFIGURATION_DEFAULTBINDADDRESS;
	m_listenPort = EQUITWEBSERVERCONFIGURATION_DEFAULTBINDPORT;
	m_cgiTimeout = EQUITWEBSERVERCONFIGURATION_DEFAULTCGITIMEOUT;
	m_allowDirectoryListings = EQUITWEBSERVERCONFIGURATION_DEFAULTALLOWDIRLISTS;
	m_extensionMIMETypes.clear();
	m_mimeActions.clear();
	m_mimeCGI.clear();
	clearAllIPAddressPolicies();
	setDefaultConnectionPolicy(EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTCONNECTIONPOLICY);

	addFileExtensionMIMEType("html", "text/html");
	addFileExtensionMIMEType("htm", "text/html");
	addFileExtensionMIMEType("shtml", "text/html");

	addFileExtensionMIMEType("css", "text/css");

	addFileExtensionMIMEType("pdf", "application/pdf");

	addFileExtensionMIMEType("js", "application/x-javascript");

	addFileExtensionMIMEType("ico", "image/x-ico");
	addFileExtensionMIMEType("png", "image/png");
	addFileExtensionMIMEType("jpg", "image/jpeg");
	addFileExtensionMIMEType("jpeg", "image/jpeg");
	addFileExtensionMIMEType("gif", "image/gif");
	addFileExtensionMIMEType("bmp", "image/x-bmp");

	setMIMETypeAction("text/html", Serve);
	setMIMETypeAction("text/css", Serve);
	setMIMETypeAction("application/pdf", Serve);
	setMIMETypeAction("application/x-javascript", Serve);
	setMIMETypeAction("image/png", Serve);
	setMIMETypeAction("image/jpeg", Serve);
	setMIMETypeAction("image/gif", Serve);
	setMIMETypeAction("image/x-ico", Serve);
	setMIMETypeAction("image/x-bmp", Serve);

	setDefaultMIMEType("application/octet-stream");
	setDefaultAction(EQUITWEBSERVERCONFIGURATION_INITIALDEFAULTACTION);
}


bool EquitWebServer::Configuration::isValidIPAddress(const QString & addr) {
	static QHostAddress h;
	h.setAddress(addr);
	return !h.isNull();
}


const QString & EquitWebServer::Configuration::listenAddress(void) const {
	return m_listenIP;
}


bool EquitWebServer::Configuration::setListenAddress(const QString & listenAddress) {
	if(isValidIPAddress(listenAddress)) {
		m_listenIP = listenAddress;
		return true;
	}

	return false;
}


int EquitWebServer::Configuration::port(void) const {
	return m_listenPort;
}


bool EquitWebServer::Configuration::setPort(int port) {
	if(port > 0 && port < 65536) {
		m_listenPort = port;
		return true;
	}

	return false;
}


const QString EquitWebServer::Configuration::getDocumentRoot(const QString & platform) const {
	if(m_documentRoot.contains(platform))
		return m_documentRoot[platform];

	return m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING];
}


bool EquitWebServer::Configuration::setDocumentRoot(const QString & docRoot, const QString & platform) {
	if(platform.isEmpty())
		m_documentRoot[EQUITWEBSERVERCONFIGURATION_RUNTIMEPLATFORMSTRING] = docRoot;
	else
		m_documentRoot[platform] = docRoot;

	return true;
}


QStringList EquitWebServer::Configuration::getRegisteredIPAddressList(void) const {
	return m_ipConnectionPolicy.keys();
}


QStringList EquitWebServer::Configuration::getRegisteredFileExtensions(void) const {
	return m_extensionMIMETypes.keys();
}


QStringList EquitWebServer::Configuration::getRegisteredMIMETypes(void) const {
	return m_mimeActions.keys();
}


bool EquitWebServer::Configuration::addFileExtensionMIMEType(const QString & ext, const QString & mime) {
	QString realExt = ext.trimmed().toLower();
	QString realMime = mime.trimmed();

	if(realExt.isEmpty() || realMime.isEmpty()) {
		qDebug() << "bpWebServer::bpWebServer::Configuration::addFileExtensionMIMEType() - no extension or no MIME type";
		return false;
	}

	if(!m_extensionMIMETypes.contains(realExt)) {
		m_extensionMIMETypes[realExt] = QVector<QString>();
		m_extensionMIMETypes[realExt] << realMime;
		return true;
	}
	else {
		if(!m_extensionMIMETypes[realExt].contains(realMime)) {
			m_extensionMIMETypes[realExt] << realMime;
			return true;
		}
	}

	return false;
}


void EquitWebServer::Configuration::removeFileExtensionMIMEType(const QString & ext, const QString & mime) {
	QString realExt = ext.trimmed().toLower();
	if(realExt.isEmpty() || !m_extensionMIMETypes.contains(realExt))
		return;

	QString realMime = mime.trimmed();
	if(realMime.isEmpty())
		m_extensionMIMETypes.remove(realExt);
	else {
		int i = m_extensionMIMETypes[realExt].indexOf(mime);
		if(i != -1)
			m_extensionMIMETypes[realExt].remove(i);
	}
}


void EquitWebServer::Configuration::removeFileExtension(const QString & ext) {
	removeFileExtensionMIMEType(ext, QString::null);
}


QVector<QString> EquitWebServer::Configuration::getMIMETypesForFileExtension(const QString & ext) const {
	QString realExt = ext.trimmed().toLower();
	if(!realExt.isEmpty() && m_extensionMIMETypes.contains(realExt))
		return m_extensionMIMETypes[realExt];
	QVector<QString> ret;

	/* if no defalt MIME type, return an empty vector */
	if(!m_defaultMIMEType.isEmpty())
		ret << m_defaultMIMEType;
	else
		qDebug() << "bpWebServer::bpWebServer::Configuration::getMIMETypesForFileExtension() - there is no default MIME type specified.";

	return ret;
}


void EquitWebServer::Configuration::clearAllFileExtensions(void) {
	m_extensionMIMETypes.clear();
}


EquitWebServer::Configuration::WebServerAction EquitWebServer::Configuration::getMIMETypeAction(const QString & mime) const {
	QString realMime = mime.trimmed();

	if(mime.isEmpty())
		return Forbid;

	if(m_mimeActions.contains(realMime))
		return m_mimeActions[realMime];

	return m_defaultAction;
}


bool EquitWebServer::Configuration::setMIMETypeAction(const QString & mime, const WebServerAction & action) {
	QString realMime = mime.trimmed();

	if(realMime.isEmpty())
		return false;

	m_mimeActions[realMime] = action;
	return true;
}


void EquitWebServer::Configuration::unsetMIMETypeAction(const QString & mime) {
	m_mimeActions.remove(mime);
}


void EquitWebServer::Configuration::clearAllMIMETypeActions(void) {
	m_mimeActions.clear();
}


EquitWebServer::Configuration::WebServerAction EquitWebServer::Configuration::getDefaultAction(void) const {
	return m_defaultAction;
}


void EquitWebServer::Configuration::setDefaultAction(const WebServerAction & action) {
	m_defaultAction = action;
}


QString EquitWebServer::Configuration::getDefaultMIMEType(void) const {
	return m_defaultMIMEType;
}


void EquitWebServer::Configuration::setDefaultMIMEType(const QString & mime) {
	m_defaultMIMEType = mime.trimmed().toLower();
}


void EquitWebServer::Configuration::unsetDefaultMIMEType(void) {
	setDefaultMIMEType(QString::null);
}


QString EquitWebServer::Configuration::getCGIBin(void) const {
	return m_cgiBin;
}


void EquitWebServer::Configuration::setCGIBin(const QString & bin) {
	/// TODO preprocess the path to ensure it's safe - i.e. no '..' and not absolute
	m_cgiBin = bin;
}


QString EquitWebServer::Configuration::getMIMETypeCGI(const QString & mime) const {
	QString realMime = mime.trimmed();
	if(realMime.isEmpty())
		return QString();

	if(m_mimeCGI.contains(realMime))
		return m_mimeCGI[realMime];

	return QString();
}


void EquitWebServer::Configuration::setMIMETypeCGI(const QString & mime, const QString & cgiExe) {
	QString realMime = mime.trimmed();
	if(realMime.isEmpty())
		return;

	QString realCGI = cgiExe.trimmed();
	if(realCGI.isEmpty())
		m_mimeCGI.remove(realMime);
	else
		m_mimeCGI[realMime] = realCGI;
}


void EquitWebServer::Configuration::unsetMIMETypeCGI(const QString & mime) {
	setMIMETypeCGI(mime, QString::null);
}


int EquitWebServer::Configuration::getCGITimeout(void) const {
	return m_cgiTimeout;
}


bool EquitWebServer::Configuration::setCGITimeout(int msec) {
	if(msec > 0) {
		m_cgiTimeout = msec;
		return true;
	}

	return false;
}


QString EquitWebServer::Configuration::getAdminEmail(void) const {
	return m_adminEmail;
}


void EquitWebServer::Configuration::setAdminEmail(const QString & admin) {
	m_adminEmail = admin;
}


EquitWebServer::Configuration::ConnectionPolicy EquitWebServer::Configuration::getDefaultConnectionPolicy(void) const {
	return m_defaultConnectionPolicy;
}


void EquitWebServer::Configuration::setDefaultConnectionPolicy(EquitWebServer::Configuration::ConnectionPolicy p) {
	m_defaultConnectionPolicy = p;
}


EquitWebServer::Configuration::ConnectionPolicy EquitWebServer::Configuration::getIPAddressPolicy(const QString & addr) const {
	if(!isValidIPAddress(addr))
		return NoConnectionPolicy;
	if(m_ipConnectionPolicy.contains(addr))
		return m_ipConnectionPolicy[addr];
	return getDefaultConnectionPolicy();
}


void EquitWebServer::Configuration::clearAllIPAddressPolicies(void) {
	m_ipConnectionPolicy.clear();
}


bool EquitWebServer::Configuration::setIPAddressPolicy(const QString & addr, ConnectionPolicy p) {
	if(isValidIPAddress(addr)) {
		m_ipConnectionPolicy[addr] = p;
		return true;
	}

	return false;
}


bool EquitWebServer::Configuration::clearIPAddressPolicy(const QString & addr) {
	if(isValidIPAddress(addr)) {
		if(m_ipConnectionPolicy.contains(addr))
			m_ipConnectionPolicy.remove(addr);

		return true;
	}

	return false;
}


bool EquitWebServer::Configuration::isDirectoryListingAllowed(void) const {
	return m_allowDirectoryListings;
}


void EquitWebServer::Configuration::setAllowDirectoryListing(bool allow) {
	m_allowDirectoryListings = allow;
}
