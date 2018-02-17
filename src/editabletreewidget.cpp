/** \file EditableTreeWidget.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the bpEditableTreeWidget class for EquitWebServer
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#include "editabletreewidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QKeySequence>
#include <QObject>


namespace EquitWebServer {


	EditableTreeWidget::EditableTreeWidget(QWidget * parent)
	: QTreeWidget(parent),
	  m_removeAction(new QAction(QIcon::fromTheme("list-remove"), tr("&Remove"))) {
		m_removeAction->setStatusTip(tr("Remove the selected entries."));

		connect(m_removeAction, &QAction::triggered, [this]() {
			Q_EMIT removeRequested(selectedItems());
			Q_EMIT removeRequested();
		});
	}


	void EditableTreeWidget::contextMenuEvent(QContextMenuEvent * event) {
		if(0 == selectedItems().size()) {
			m_removeAction->setEnabled(false);
		}
		else {
			m_removeAction->setEnabled(true);
		}

		QMenu menu(this);

		for(auto * a : actions()) {
			menu.addAction(a);
		}

		menu.exec(event->globalPos());
	}


	void EditableTreeWidget::removeItems(const QModelIndexList & itemIndices) {
		QList<QTreeWidgetItem *> items;
		QTreeWidgetItem * it;

		/* index list contains one entry for each column in a selected item */
		for(const QModelIndex & i : itemIndices) {
			it = itemFromIndex(i);

			if(!items.contains(it)) {
				items << it;
			}
		}

		int itemCount = items.count();

		for(int i = 0; i < itemCount; i++) {
			it = items[i];

			if(it) {
				emit(removingItem(it));
				QTreeWidgetItem * p = it->parent();

				if(p) {
					p->removeChild(it);
				}
				else {
					takeTopLevelItem(indexOfTopLevelItem(it));
				}

				delete it;
			}
		}
	}


}  // namespace EquitWebServer
