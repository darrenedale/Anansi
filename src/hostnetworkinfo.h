/** \file HostNetworkInfo.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the HostNetworkInfo class for EquitWebServer
  *
  * \todo
  * - decide on application license
  *
  * \par Changes
  * - (2012-06-19) class created to replace use of QHostInfo.
  * - (2012-06-19) file documentation created.
  */

#ifndef HOSTNETWORKINFO_H
#define HOSTNETWORKINFO_H

#include <unordered_set>
#include <QHostAddress>

#include "qtstdhash.h"

namespace std {
	template<>
	struct hash<QHostAddress> : public Equit::QtHash<QHostAddress> {};
}  // namespace std

namespace EquitWebServer {

	using HostAddressList = std::unordered_set<QHostAddress>;

	class HostNetworkInfo {
	public:
		enum Protocol : uint32_t {
			IPv4 = 0x00000001,
			IPv6 = 0x00000002,
		};

		Q_DECLARE_FLAGS(Protocols, Protocol)

		static const HostAddressList & localHostAddresses(const Protocols & = Protocols(Protocol::IPv4 | Protocol::IPv6));

	private:
		static HostAddressList s_localhostAddresses;
	};
}  // namespace EquitWebServer


Q_DECLARE_OPERATORS_FOR_FLAGS(EquitWebServer::HostNetworkInfo::Protocols)

#endif  // HOSTNETWORKINFO_H
