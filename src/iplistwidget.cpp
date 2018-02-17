/** \file IpListWidget.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the bpIpListWidget class for EquitWebServer
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  */

#include "iplistwidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>


namespace EquitWebServer {


	IpListWidget::IpListWidget(QWidget * parent)
	: QTreeWidget(parent) {
		setColumnCount(2);
		QTreeWidgetItem * header = new QTreeWidgetItem;
		header->setText(0, tr("IP Address"));
		header->setText(1, tr("Policy"));
		QTreeWidget::setHeaderItem(header);
		QTreeWidget::setRootIsDecorated(false);
		QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);
	}


	void IpListWidget::contextMenuEvent(QContextMenuEvent * event) {
		if(itemAt(event->x(), event->y())) {
			QAction * removeIP = new QAction(QIcon::fromTheme(QStringLiteral("list-remove"), QIcon(QStringLiteral(":/icons/iplistwidget/menu/remove"))), tr("&Remove"), this);
			removeIP->setShortcut(tr("Ctrl+R"));
			removeIP->setToolTip(tr("Remove this IP address from the list"));
			removeIP->setStatusTip(tr("Remove this IP address from the list"));
			connect(removeIP, &QAction::triggered, this, &IpListWidget::removeSelectedIpAddress);
			QMenu menu(this);
			menu.addAction(removeIP);
			menu.exec(event->globalPos());
		}
		else {
			QTreeWidget::contextMenuEvent(event);
		}
	}


	void IpListWidget::removeIpAddress(int i) {
		QTreeWidgetItem * item = takeTopLevelItem(i);

		if(item) {
			Q_EMIT ipAddressRemoved(item->text(0));
			delete item;
		}
	}


	void IpListWidget::removeSelectedIpAddress() {
		removeIpAddress(currentIndex().row());
	}


}  // namespace EquitWebServer
