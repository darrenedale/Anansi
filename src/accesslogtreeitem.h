#ifndef EQUITWEBSERVER_ACCESSLOGTREEITEM_H
#define EQUITWEBSERVER_ACCESSLOGTREEITEM_H

#include <QTreeWidgetItem>

#include "configuration.h"

namespace EquitWebServer {

	class AccessLogTreeItem : public QTreeWidgetItem {
	public:
		static constexpr const int ItemType = QTreeWidgetItem::UserType + 6482;

		explicit AccessLogTreeItem(const QString & addr, quint16 port, const QString & resource, Configuration::WebServerAction action);
		explicit AccessLogTreeItem(const QString & addr, quint16 port, Configuration::ConnectionPolicy policy);

		void setIpAddress(const QString & addr);
		void setPort(quint16 port);
		void setResource(const QString & resource);
		void setAction(Configuration::WebServerAction action);
		void setConnectionPolicy(Configuration::ConnectionPolicy policy);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ACCESSLOGTREEITEM_H
