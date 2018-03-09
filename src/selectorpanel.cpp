/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Anansi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Anansi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the SelectorPanel class.
///
/// \dep
/// - selectorpanel.h
/// - <iostream>
/// - <QDrag>
/// - <QStyledItemDelegate>
///
/// \par Changes
/// - (2018-03) First release.

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
