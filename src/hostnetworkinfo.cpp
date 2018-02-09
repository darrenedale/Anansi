/** \file HostNetworkInfo.cpp
  * \author Darren Edale
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
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#include "hostnetworkinfo.h"

#include <QHostAddress>

/* Q_OS_UNIX is basically anything other than ms-dos, os/2 and windows */
#if defined(Q_OS_UNIX) || defined(Q_OS_CYGWIN) || (defined(Q_OS_WIN) && defined(Q_CC_GNU))

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif


namespace EquitWebServer {


	HostAddressList EquitWebServer::HostNetworkInfo::s_localhostAddresses;


	const HostAddressList & EquitWebServer::HostNetworkInfo::localHostAddresses(const EquitWebServer::HostNetworkInfo::Protocols & protocols) {
#if defined(Q_OS_UNIX) || defined(Q_OS_CYGWIN) || (defined(Q_OS_WIN) && defined(Q_CC_GNU))
		/* POSIX comptaible (enough) environments */

		/* this code is adapted from
	 *
	 * http://stackoverflow.com/questions/212528/linux-c-get-the-ip-address-of-local-computer
	 */
		struct ifaddrs * ifaddrs = nullptr;
		struct ifaddrs * ifa = nullptr;
		getifaddrs(&ifaddrs);

		for(ifa = ifaddrs; ifa; ifa = ifa->ifa_next) {
			if(protocols & IPv4 && AF_INET == ifa->ifa_addr->sa_family) {
				s_localhostAddresses.emplace(ifa->ifa_addr);
			}
			else if(protocols & IPv6 && AF_INET6 == ifa->ifa_addr->sa_family) {
				s_localhostAddresses.emplace(ifa->ifa_addr);
			}
		}

		if(ifaddrs) {
			freeifaddrs(ifaddrs);
		}

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

}  // namespace EquitWebServer
