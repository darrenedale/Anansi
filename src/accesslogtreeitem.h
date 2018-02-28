/// \file accesslogtreeitem.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the AccessLogTreeItem class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_ACCESSLOGTREEITEM_H
#define EQUITWEBSERVER_ACCESSLOGTREEITEM_H

#include <QTreeWidgetItem>

#include "configuration.h"

namespace EquitWebServer {

	class AccessLogTreeItem : public QTreeWidgetItem {
	public:
		static constexpr const int ItemType = QTreeWidgetItem::UserType + 9003;

		explicit AccessLogTreeItem(const QString & addr, quint16 port, const QString & resource, WebServerAction action);
		explicit AccessLogTreeItem(const QString & addr, quint16 port, ConnectionPolicy policy);

		void setIpAddress(const QString & addr);
		void setPort(quint16 port);
		void setResource(const QString & resource);
		void setAction(WebServerAction action);
		void setConnectionPolicy(ConnectionPolicy policy);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ACCESSLOGTREEITEM_H
