/** \file HostNetworkInfo.h
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the HostNetworkInfo class for EquitWebServer
  *
  * \todo
  * - decide on application license
  *
  * \par Current Changes
  * - (2012-06-19) class created to replace use of QHostInfo.
  * - (2012-06-19) file documentation created.
  */

#ifndef HOSTNETWORKINFO_H
#define HOSTNETWORKINFO_H

#include <QList>
#include <QHostAddress>

namespace EquitWebServer {
	class HostNetworkInfo {
		public:
			enum Protocol {
				IPv4 = 0x00000001,
				IPv6 = 0x00000002
			};

			Q_DECLARE_FLAGS(Protocols, Protocol)

			static const QList<QHostAddress> & localHostAddresses( const Protocols & = Protocols(IPv4 | IPv6) );

		private:
			static QList<QHostAddress> s_localhostAddresses;
	};
}


Q_DECLARE_OPERATORS_FOR_FLAGS(EquitWebServer::HostNetworkInfo::Protocols)

#endif // HOSTNETWORKINFO_H
