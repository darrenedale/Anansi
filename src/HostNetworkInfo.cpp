/** \file HostNetworkInfo.cpp
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the HostNetworkInfo class for EquitWebServer
  *
  * \todo
  * - the following platform implementations are required for
  *   localHostAddresses():
  *   - Q_OS_WIN
  *   - Q_OS_OS2
  * - decide on application license
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  *
  */

#include "HostNetworkInfo.h"

#include <QHostAddress>

/* Q_OS_UNIX is basically anything other than ms-dos, os/2 and windows */
#if defined(Q_OS_UNIX) || defined(Q_OS_CYGWIN) || (defined(Q_OS_WIN) && defined(Q_CC_GNU))

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif


QList<QHostAddress> EquitWebServer::HostNetworkInfo::s_localhostAddresses;


const QList<QHostAddress> & EquitWebServer::HostNetworkInfo::localHostAddresses( const EquitWebServer::HostNetworkInfo::Protocols & protocols ) {
#if defined(Q_OS_UNIX) || defined(Q_OS_CYGWIN) || (defined(Q_OS_WIN) && defined(Q_CC_GNU))
	/* POSIX comptaible (enough) environments */

	/* this code is adapted from
	 *
	 * http://stackoverflow.com/questions/212528/linux-c-get-the-ip-address-of-local-computer
	 */
	struct ifaddrs * ifaddrs = 0, * ifa = 0 ;
	getifaddrs(&ifaddrs);

	for(ifa = ifaddrs; ifa; ifa = ifa->ifa_next) {
		if(protocols & IPv4 && AF_INET == ifa->ifa_addr->sa_family) {
			QHostAddress ha(ifa->ifa_addr);
			if(!s_localhostAddresses.contains(ha)) s_localhostAddresses.append(ha);
		}
		else if(protocols & IPv6 && AF_INET6 == ifa->ifa_addr->sa_family) {
			QHostAddress ha(ifa->ifa_addr);
			if(!s_localhostAddresses.contains(ha)) s_localhostAddresses.append(ha);
		}
	}

	if(ifaddrs) freeifaddrs(ifaddrs);

#elif defined(Q_OS_WIN)

/* non-mingw and non-cygwin */
#error HostNetworkInfo::localHostAddresses() has no implementation for windows

#elif defined(Q_OS_OS2)

#error HostNetworkInfo::localHostAddresses() has no implementation for os/2

#else

#error HostNetworkInfo::localHostAddresses() has no implementation for this (unsupported) platform

#endif

	return s_localhostAddresses;
}
