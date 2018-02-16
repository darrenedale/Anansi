#include "ipaddressconnectionpolicytreeitem.h"

#include <QApplication>


namespace EquitWebServer {


	IpAddressConnectionPolicyTreeItem::IpAddressConnectionPolicyTreeItem(const QString & addr, Configuration::ConnectionPolicy policy)
	: QTreeWidgetItem(ItemType) {
		setIpAddress(addr);
		setConnectionPolicy(policy);
	}


	QString IpAddressConnectionPolicyTreeItem::ipAddress() const {
		return text(0);
	}


	void IpAddressConnectionPolicyTreeItem::setIpAddress(const QString & addr) {
		setText(0, addr);
	}


	void IpAddressConnectionPolicyTreeItem::setConnectionPolicy(Configuration::ConnectionPolicy policy) {
		m_policy = policy;

		switch(policy) {
			case Configuration::ConnectionPolicy::Accept:
				setText(1, "Accept Connection");
				setIcon(1, QIcon(":/icons/connectionpolicies/accept"));
				break;

			case Configuration::ConnectionPolicy::Reject:
				setText(1, QApplication::tr("Reject Connection"));
				setIcon(1, QIcon(":/icons/connectionpolicies/reject"));
				break;

			case Configuration::ConnectionPolicy::None:
				setText(1, QApplication::tr("No policy"));
				setIcon(1, {});
				break;
		}
	}


}  // namespace EquitWebServer
