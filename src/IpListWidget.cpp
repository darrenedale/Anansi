/** \file IpListWidget.cpp
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the bpIpListWidget class for EquitWebServer
  *
  * \todo
  * - decide on application license
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  */

#include "IpListWidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>


namespace EquitWebServer {


	IpListWidget::IpListWidget( QWidget * parent )
	:	QTreeWidget(parent) {
		setColumnCount(2);
		QTreeWidgetItem * header = new QTreeWidgetItem();
		header->setText(0, tr("IP Address"));
		header->setText(1, tr("Policy"));
		setHeaderItem(header);
		setRootIsDecorated(false);
		QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);
	}


	void IpListWidget::contextMenuEvent(QContextMenuEvent * event) {
		if(itemAt(event->x(), event->y())) {
			QAction * removeIP = new QAction(tr("&Remove"), this);
			removeIP->setShortcut(tr("Ctrl+R"));
			removeIP->setStatusTip(tr("Remove this IP address from the list"));
			removeIP->setIcon(QIcon::fromTheme("list-remove", QIcon(":/icons/iplistwidget/menu/remove")));
			connect(removeIP, SIGNAL(triggered()), this, SLOT(removeSelectedIPAddress()));
			QMenu menu(this);
			menu.addAction(removeIP);
			menu.exec(event->globalPos());
		}
		else {
			QTreeWidget::contextMenuEvent(event);
		}
	}


	void IpListWidget::removeIPAddress(int i) {
		QTreeWidgetItem * item = takeTopLevelItem(i);

		if(item) 	{
			emit(ipAddressRemoved(item->text(0)));
			delete item;
		}
	}


	void IpListWidget::removeSelectedIPAddress() {
		removeIPAddress(currentIndex().row());
	}


	void IpListWidget::insertTopLevelItem (int index, QTreeWidgetItem * item) {
		if(!item) {
			return;
		}

		// ensure item has ip address not in list already
		int itemCount = topLevelItemCount();

		for(int i = 0; i < itemCount; i++) 	{
			auto * myItem = topLevelItem(i);

			if(myItem->text(0) == item->text(0)) {
				return;
			}
		}

		QTreeWidget::insertTopLevelItem(index, item);
	}
}
