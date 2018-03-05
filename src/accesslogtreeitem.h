/// \file accesslogtreeitem.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the AccessLogTreeItem class for EquitWebServer
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_ACCESSLOGTREEITEM_H
#define EQUITWEBSERVER_ACCESSLOGTREEITEM_H

#include <QTreeWidgetItem>

#include "types.h"

class QString;

namespace EquitWebServer {

	class AccessLogTreeItem : public QTreeWidgetItem {
	public:
		explicit AccessLogTreeItem(const QString & addr, uint16_t port, const QString & resource, WebServerAction action);
		explicit AccessLogTreeItem(const QString & addr, uint16_t port, ConnectionPolicy policy);

		void setIpAddress(const QString & addr);
		void setPort(uint16_t port);
		void setResource(const QString & resource);
		void setAction(WebServerAction action);
		void setConnectionPolicy(ConnectionPolicy policy);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ACCESSLOGTREEITEM_H
