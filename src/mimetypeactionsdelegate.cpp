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

/// \file mimetypeactionsdelegate.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MimeActionsItemDelegate class.
///
/// \dep
/// - <iostream>
/// - <QLineEdit>
/// - configuration.h
/// - filenamewidget.h
/// - mimeactionswidget.h
/// - webserveractioncombo.h
/// - servermimeactionsmodel.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mimetypeactionsdelegate.h"

#include <iostream>
#include <QLineEdit>

#include "configuration.h"
#include "filenamewidget.h"
#include "mimeactionswidget.h"
#include "webserveractioncombo.h"
#include "servermimeactionsmodel.h"
#include "qtmetatypes.h"


namespace Anansi {


	MimeTypeActionsDelegate::MimeTypeActionsDelegate(MimeActionsWidget * parent)
	: QStyledItemDelegate(parent),
	  m_parent(parent) {
	}


	QWidget * MimeTypeActionsDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const {
		if(!index.isValid()) {
			return nullptr;
		}

		switch(index.column()) {
			case ServerMimeActionsModel::MimeTypeColumnIndex:
				return nullptr;

			case ServerMimeActionsModel::ActionColumnIndex:
				return new WebServerActionCombo(parent);

			case ServerMimeActionsModel::CgiColumnIndex:
				return new FileNameWidget(parent);
		}

		return nullptr;
	}


	void MimeTypeActionsDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		switch(index.column()) {
			case ServerMimeActionsModel::ActionColumnIndex: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
				combo->setWebServerAction(index.data(Qt::EditRole).value<WebServerAction>());
				return;
			}

			case ServerMimeActionsModel::CgiColumnIndex: {
				auto * fileNameWidget = qobject_cast<FileNameWidget *>(editor);
				Q_ASSERT_X(fileNameWidget, __PRETTY_FUNCTION__, "expected editor to be a FileNameWidget");
				fileNameWidget->setFileName(index.data(Qt::EditRole).value<QString>());
				return;
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	void MimeTypeActionsDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		switch(index.column()) {
			case ServerMimeActionsModel::ActionColumnIndex: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
				model->setData(index, QVariant::fromValue(combo->webServerAction()));
				break;
			}

			case ServerMimeActionsModel::CgiColumnIndex: {
				auto * fileName = qobject_cast<FileNameWidget *>(editor);
				Q_ASSERT_X(fileName, __PRETTY_FUNCTION__, "expected editor to be a FileNameWidget");
				model->setData(index, fileName->fileName());
				break;
			}
		}
	}


	MimeTypeActionsDelegate::~MimeTypeActionsDelegate() = default;


}  // namespace Anansi
