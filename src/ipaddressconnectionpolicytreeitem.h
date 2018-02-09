#ifndef IPADDRESSCONNECTIONPOLICYTREEITEM_H
#define IPADDRESSCONNECTIONPOLICYTREEITEM_H

#include <QTreeWidgetItem>

#include "configuration.h"

namespace EquitWebServer {

	class IpAddressConnectionPolicyTreeItem : public QTreeWidgetItem {
	public:
		static constexpr const int ItemType = QTreeWidgetItem::UserType + 9014;

		explicit IpAddressConnectionPolicyTreeItem(const QString & ip, Configuration::ConnectionPolicy policy);

		QString ipAddress() const;

		inline Configuration::ConnectionPolicy connectionPolicy() const {
			return m_policy;
		}

		void setIpAddress(const QString & addr);
		void setConnectionPolicy(Configuration::ConnectionPolicy policy);

	private:
		Configuration::ConnectionPolicy m_policy;
	};

}  // namespace EquitWebServer

#endif  // IPADDRESSCONNECTIONPOLICYTREEITEM_H
