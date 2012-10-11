/** \file bpIpListWidget.cpp
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

#include "bpIpListWidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>


bpIpListWidget::bpIpListWidget( QWidget * parent )
:	QTreeWidget(parent) {
	setColumnCount(2);
	QTreeWidgetItem * header = new QTreeWidgetItem();
	header->setText(0, tr("IP Address"));
	header->setText(1, tr("Policy"));
	setHeaderItem(header);
	setRootIsDecorated(false);
	setSelectionMode(QAbstractItemView::SingleSelection);
}


void bpIpListWidget::contextMenuEvent( QContextMenuEvent * event ) {
	QTreeWidgetItem * i = itemAt(event->x(), event->y());
	
	if(i) {
		QAction * removeIP = new QAction(tr("&Remove"), this);
		removeIP->setShortcut(tr("Ctrl+R"));
		removeIP->setStatusTip(tr("Remove this IP address from the list"));
		removeIP->setIcon(QIcon::fromTheme("list-remove", QIcon(":/icons/iplistwidget/menu/remove")));
		connect(removeIP, SIGNAL(triggered()), this, SLOT(removeSelectedIPAddress()));
		QMenu menu(this);
		menu.addAction(removeIP);
		menu.exec(event->globalPos());
	}
	else
		QTreeWidget::contextMenuEvent(event);
}


void bpIpListWidget::removeIPAddress( int i ) {
	QTreeWidgetItem * it = takeTopLevelItem(i);

	if(it) 	{
		emit(ipAddressRemoved(it->text(0)));
		delete it;
	}
}


void bpIpListWidget::removeSelectedIPAddress( void ) {
	removeIPAddress(currentIndex().row());
}


void bpIpListWidget::insertTopLevelItem ( int index, QTreeWidgetItem * item ) {
	if(!item)
		return;
	
	// ensure item has ip address not in list already
	int itemCount = topLevelItemCount();
	QTreeWidgetItem * it;
	
	for(int i = 0; i < itemCount; i++) 	{
		it = topLevelItem(i);
		
		if(it->text(0) == item->text(0))
			return;
	}
	
	QTreeWidget::insertTopLevelItem(index, item);
}


void bpIpListWidget::setSelectionMode( QAbstractItemView::SelectionMode mode ) {
	(void) mode;
	QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);
}
