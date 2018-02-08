/** \file bpEditableTreeWidget.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the bpEditableTreeWidget class for EquitWebServer
  *
  * \todo
  * - decide on application license
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#include "bpEditableTreeWidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QKeySequence>
#include <QObject>


bpEditableTreeWidget::bpEditableTreeWidget( QWidget * parent )
:	QTreeWidget(parent),
	m_removeAction(0) {
	m_removeAction = addAction(tr("&Remove"), this, SLOT(emitRemoveRequested()), tr("Ctrl+R"));
	m_removeAction->setStatusTip(tr("Remove the selected entries."));
}


void bpEditableTreeWidget::addAction( QAction * action ) {
	QTreeWidget::addAction(action);
}


QAction * bpEditableTreeWidget::addAction( const QIcon & icon, const QString & text, const QObject * receiver, const char * slot, const QKeySequence & shortcut  ) {
	QAction * ret = new QAction(icon, text, 0);
	if(receiver && slot) connect(ret, SIGNAL(triggered()), receiver, slot);
	if(!shortcut.isEmpty()) ret->setShortcut(shortcut);
	addAction(ret);
	return ret;
}


QAction * bpEditableTreeWidget::addAction( const QIcon & icon, const QString & text ) {
	return addAction(icon, text, 0, 0);
}


QAction * bpEditableTreeWidget::addAction( const QString & text, const QObject * receiver, const char * slot, const QKeySequence & shortcut ) {
	QAction * ret = new QAction(text, 0);
	if(receiver && slot) connect(ret, SIGNAL(triggered()), receiver, slot);
	if(!shortcut.isEmpty()) ret->setShortcut(shortcut);
	addAction(ret);
	return ret;
}


QAction * bpEditableTreeWidget::addAction( const QString & text ) {
	return addAction(text, 0, 0);
}


void bpEditableTreeWidget::contextMenuEvent( QContextMenuEvent * event ) {
	if(selectedItems().size() == 0) m_removeAction->setEnabled(false);
	else m_removeAction->setEnabled(true);

	QMenu menu(this);

	foreach(QAction * a, actions())
		menu.addAction(a);

	menu.exec(event->globalPos());
}


void bpEditableTreeWidget::removeItems( const QModelIndexList & itemIndices ) {
	QList<QTreeWidgetItem *> items;
	QTreeWidgetItem * it;
	
	/* index list contains one entry for each column in a selected item */
	foreach(QModelIndex i, itemIndices) {
		it = itemFromIndex(i);
		if(!items.contains(it))
			items << it;
	}

	int itemCount = items.count();
	
	for(int i = 0; i < itemCount; i++) {
		it = items[i];

		if(it) {
			emit(removingItem(it));
			QTreeWidgetItem * p = it->parent();
			
			if(p) p->removeChild(it);
			else takeTopLevelItem(indexOfTopLevelItem(it));
			
			delete it;
		}
	}
}


void bpEditableTreeWidget::emitRemoveRequested( void ) {
	emit(removeRequested(selectedItems()));
	emit(removeRequested());
}
