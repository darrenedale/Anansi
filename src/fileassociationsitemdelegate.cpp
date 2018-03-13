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

/// \file fileassociationsitemdelegate.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the FileAssociationsItemDelegate class.
///
/// \dep
/// - fileassociationsitemdelegate.h
/// - <iostream>
/// - <QLineEdit>
/// - <QMessageBox>
/// - fileassociationswidget.h
/// - mimecombo.h
/// - window.h
///
/// \par Changes
/// - (2018-03) First release.

#include "fileassociationsitemdelegate.h"

#include <iostream>

#include <QLineEdit>
#include <QMessageBox>

#include "fileassociationswidget.h"
#include "mimecombo.h"
#include "windowbase.h"
#include "notifications.h"


namespace Anansi {


	FileAssociationsItemDelegate::FileAssociationsItemDelegate(FileAssociationsWidget * parent)
	: QStyledItemDelegate(parent),
	  m_parent(parent) {
	}


	QWidget * FileAssociationsItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const {
		if(!index.isValid()) {
			return nullptr;
		}

		if(index.parent().isValid()) {
			auto * editor = new MimeCombo(true, parent);

			if(m_parent) {
				for(const auto & mimeType : m_parent->availableMimeTypes()) {
					editor->addMimeType(mimeType);
				}
			}

			editor->setCurrentText(index.data().value<QString>());
			return editor;
		}
		else {
			auto * editor = new QLineEdit(parent);
			editor->setText(index.data().value<QString>());
			return editor;
		}
	}


	void FileAssociationsItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		if(index.parent().isValid()) {
			auto * combo = qobject_cast<MimeCombo *>(editor);
			Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected delegate editor to be a MimeTypeCombo");
			combo->setCurrentText(index.data().value<QString>());
			combo->lineEdit()->selectAll();
		}
		else {
			auto * lineEdit = qobject_cast<QLineEdit *>(editor);
			Q_ASSERT_X(lineEdit, __PRETTY_FUNCTION__, "expected delegate editor to be a QLineEdit");
			lineEdit->setText(index.data().value<QString>());
			lineEdit->selectAll();
		}
	}


	void FileAssociationsItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		const auto parentIndex = index.parent();

		if(parentIndex.isValid()) {
			auto * combo = qobject_cast<MimeCombo *>(editor);
			Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected delegate editor to be a MimeTypeCombo");

			if(!model->setData(index, combo->currentText())) {
				showNotification(m_parent, tr("<p>The file extension %1 could have the MIME type %2 added.</p><p><small>Perhaps the file extension already has that MIME type?</small></p>").arg(model->data(parentIndex).value<QString>(), combo->currentText()), NotificationType::Warning);
			}
		}
		else {
			auto * lineEdit = qobject_cast<QLineEdit *>(editor);
			Q_ASSERT_X(lineEdit, __PRETTY_FUNCTION__, "expected delegate editor to be a QLineEdit");

			if(!model->setData(index, lineEdit->text())) {
				showNotification(m_parent, tr("<p>The file extension could not be set to %1.</p><p><small>Perhaps that file extension is already used elsewhere?</small></p>").arg(lineEdit->text()), NotificationType::Warning);
			}
		}
	}


}  // namespace Anansi
