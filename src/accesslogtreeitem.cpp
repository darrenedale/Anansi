#include "accesslogtreeitem.h"

#include <QApplication>


namespace EquitWebServer {


	AccessLogTreeItem::AccessLogTreeItem(const QString & addr, quint16 port, const QString & resource, Configuration::WebServerAction action)
	: QTreeWidgetItem(ItemType) {
		setIpAddress(addr);
		setPort(port);
		setResource(resource);
		setAction(action);
	}


	AccessLogTreeItem::AccessLogTreeItem(const QString & addr, quint16 port, Configuration::ConnectionPolicy policy)
	: QTreeWidgetItem(ItemType) {
		setIpAddress(addr);
		setPort(port);
		setConnectionPolicy(policy);
	}


	void AccessLogTreeItem::setIpAddress(const QString & addr) {
		setText(0, addr);
	}


	void AccessLogTreeItem::setPort(uint16_t port) {
		setText(1, QString::number(port));
	}


	void AccessLogTreeItem::setResource(const QString & resource) {
		setText(2, resource);
	}


	void AccessLogTreeItem::setAction(Configuration::WebServerAction action) {
		switch(action) {
			case Configuration::WebServerAction::Ignore:
				setText(3, QApplication::QApplication::tr("Ignored"));
				break;

			case Configuration::WebServerAction::Serve:
				setText(3, QApplication::tr("Served"));
				break;

			case Configuration::WebServerAction::Forbid:
				setText(3, QApplication::tr("Forbidden, not found, or CGI failed"));
				break;

			case Configuration::WebServerAction::CGI:
				setText(3, QApplication::tr("Executed through CGI"));
				break;
		}

		setIcon(3, {});
	}


	void AccessLogTreeItem::setConnectionPolicy(Configuration::ConnectionPolicy policy) {
		switch(policy) {
			case Configuration::ConnectionPolicy::None:
				setText(3, QApplication::QApplication::tr("No Connection Policy"));
				setIcon(3, {});
				break;

			case Configuration::ConnectionPolicy::Reject:
				setText(3, QApplication::tr("Rejected"));
				setIcon(3, QIcon(":/icons/connectionpolicies/reject"));
				break;

			case Configuration::ConnectionPolicy::Accept:
				setText(3, QApplication::tr("Accepted"));
				setIcon(3, QIcon(":/icons/connectionpolicies/accept"));
				break;
		}
	}


}  // namespace EquitWebServer
