#include "selectorpanel.h"

#include <iostream>

#include <QDrag>
#include <QStyledItemDelegate>

namespace Anansi {


	SelectorPanel::SelectorPanel(QWidget * parent)
	: QListWidget(parent) {
		setEditTriggers(QAbstractItemView::NoEditTriggers);
		setDropIndicatorShown(false);
		setDragDropMode(QAbstractItemView::NoDragDrop);
		setDefaultDropAction(Qt::IgnoreAction);
		setTextElideMode(Qt::ElideNone);
		setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
		setMovement(QListView::Static);
		setFlow(QListView::TopToBottom);
		setViewMode(QListView::IconMode);
		setWordWrap(true);
		setSelectionRectVisible(false);
	}


	void SelectorPanel::addItem(QListWidgetItem * item) {
		QListWidget::addItem(item);

		if(isVisible()) {
			recalculateSize();
		}
	}

	void SelectorPanel::insertItem(int row, QListWidgetItem * item) {
		QListWidget::insertItem(row, item);

		if(isVisible()) {
			recalculateSize();
		}
	}


	void SelectorPanel::showEvent(QShowEvent * event) {
		recalculateSize();
		QWidget::showEvent(event);
	}


	void SelectorPanel::recalculateSize() {
		QSize maxSize;
		QStyledItemDelegate infoDelegate;

		for(int i = count() - 1; 0 <= i; --i) {
			auto itemSize = infoDelegate.sizeHint(viewOptions(), indexFromItem(item(i)));

			if(itemSize.width() > maxSize.width()) {
				maxSize.setWidth(itemSize.width());
			}

			if(itemSize.height() > maxSize.height()) {
				maxSize.setHeight(itemSize.height());
			}
		}

		// add some spacing?

		for(int i = count() - 1; 0 <= i; --i) {
			item(i)->setSizeHint(maxSize);
		}

		setGridSize(maxSize);
		auto width = maxSize.width() + rect().width() - contentsRect().width();
		setMinimumWidth(width);
		setMaximumWidth(width);
	}


}  // namespace Anansi
