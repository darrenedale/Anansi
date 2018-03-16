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

/// \file mediatypeactionsdelegate.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MediaTypeActionsDelegate class.
///
/// \dep
/// - mediatypeactionsdelegate.h
/// - <QAbstractItemModel>
/// - <QModelIndex>
/// - assert.h
/// - types.h
/// - qtmetatypes.h
/// - mediatypeactionswidget.h
/// - filenamewidget.h
/// - webserveractioncombo.h
/// - mediatypeactionsmodel.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mediatypeactionsdelegate.h"

#include <QAbstractItemModel>
#include <QModelIndex>

#include "assert.h"
#include "types.h"
#include "qtmetatypes.h"
#include "mediatypeactionswidget.h"
#include "filenamewidget.h"
#include "webserveractioncombo.h"
#include "mediatypeactionsmodel.h"


namespace Anansi {


	MediaTypeActionsDelegate::MediaTypeActionsDelegate(MediaTypeActionsWidget * parent)
	: QStyledItemDelegate(parent),
	  m_parent(parent) {
	}


	QWidget * MediaTypeActionsDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & idx) const {
		if(!idx.isValid()) {
			return nullptr;
		}

		switch(idx.column()) {
			case MediaTypeActionsModel::MediaTypeColumnIndex:
				return nullptr;

			case MediaTypeActionsModel::ActionColumnIndex:
				return new WebServerActionCombo(parent);

			case MediaTypeActionsModel::CgiColumnIndex:
				return new FileNameWidget(parent);
		}

		return nullptr;
	}


	void MediaTypeActionsDelegate::setEditorData(QWidget * editor, const QModelIndex & idx) const {
		if(!idx.isValid()) {
			return;
		}

		switch(idx.column()) {
			case MediaTypeActionsModel::ActionColumnIndex: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				eqAssert(combo, "expected editor to be a WebServerActionCombo (it's a " << qPrintable(editor->metaObject()->className()) << ")");
				combo->setWebServerAction(idx.data(Qt::EditRole).value<WebServerAction>());
				return;
			}

			case MediaTypeActionsModel::CgiColumnIndex: {
				auto * fileNameWidget = qobject_cast<FileNameWidget *>(editor);
				eqAssert(fileNameWidget, "expected editor to be a FileNameWidget (it's a " << qPrintable(editor->metaObject()->className()) << ")");
				fileNameWidget->setFileName(idx.data(Qt::EditRole).value<QString>());
				return;
			}
		}

		QStyledItemDelegate::setEditorData(editor, idx);
	}


	void MediaTypeActionsDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & idx) const {
		if(!idx.isValid()) {
			return;
		}

		switch(idx.column()) {
			case MediaTypeActionsModel::ActionColumnIndex: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				eqAssert(combo, "expected editor to be a WebServerActionCombo (it's a " << qPrintable(editor->metaObject()->className()) << ")");
				model->setData(idx, QVariant::fromValue(combo->webServerAction()));
				break;
			}

			case MediaTypeActionsModel::CgiColumnIndex: {
				auto * fileNameWidget = qobject_cast<FileNameWidget *>(editor);
				eqAssert(fileNameWidget, "expected editor to be a FileNameWidget (it's a " << qPrintable(editor->metaObject()->className()) << ")");
				model->setData(idx, fileNameWidget->fileName());
				break;
			}
		}
	}


}  // namespace Anansi
