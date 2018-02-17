/** \file IpListWidget.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the bpIpListWidget class for EquitWebServer
  *
  * \todo class documentation.
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_IPLISTWIDGET_H
#define EQUITWEBSERVER_IPLISTWIDGET_H

#include <QTreeWidget>

#include "ipaddressconnectionpolicytreeitem.h"

namespace EquitWebServer {

	class IpAddressConnectionPolicyTreeItem;

	class IpListWidget : public QTreeWidget {
		Q_OBJECT

	public:
		explicit IpListWidget(QWidget * parent = nullptr);
		void setSelectionMode() = delete;

		inline void insertTopLevelItem(int idx, IpAddressConnectionPolicyTreeItem * item) {
			QTreeWidget::insertTopLevelItem(idx, item);
		}

		inline void addTopLevelItem(IpAddressConnectionPolicyTreeItem * item) {
			QTreeWidget::addTopLevelItem(item);
		}

		// this would be deleted too but UI files can't be made to not call it
		// so we just make it a useless call instead
		inline void setHeaderItem(QTreeWidgetItem *) {}

	protected:
		void contextMenuEvent(QContextMenuEvent * event);

	protected Q_SLOTS:
		void removeIpAddress(int i);
		void removeSelectedIpAddress();

	Q_SIGNALS:
		void ipAddressRemoved(const QString &);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_IPLISTWIDGET_H
