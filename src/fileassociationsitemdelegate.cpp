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
/// - <QString>
/// - <QLineEdit>
/// - <QAbstractItemModel>
/// - <QModelIndex>
/// - assert.h
/// - fileassociationswidget.h
/// - mediatypecombo.h
/// - notifications.h
///
/// \par Changes
/// - (2018-03) First release.

#include "fileassociationsitemdelegate.h"

#include <QString>
#include <QLineEdit>
#include <QAbstractItemModel>
#include <QModelIndex>

#include "eqassert.h"
#include "fileassociationswidget.h"
#include "mediatypecombo.h"
#include "notifications.h"


namespace Anansi {


	FileAssociationsItemDelegate::FileAssociationsItemDelegate(QObject * parent)
	: QStyledItemDelegate(parent) {
	}


	void FileAssociationsItemDelegate::addMediaType(const QString & mediaType) {
		const auto & end = m_mediaTypes.cend();

		if(end != std::find(m_mediaTypes.cbegin(), end, mediaType)) {
			return;
		}

		m_mediaTypes.push_back(mediaType);
	}


	void FileAssociationsItemDelegate::removeMediaType(const QString & mediaType) {
		const auto & end = m_mediaTypes.cend();
		const auto mediaTypeIt = std::find(m_mediaTypes.cbegin(), end, mediaType);

		if(end == mediaTypeIt) {
			return;
		}

		m_mediaTypes.erase(mediaTypeIt);
	}


	QWidget * FileAssociationsItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const {
		if(!index.isValid()) {
			return nullptr;
		}

		if(index.parent().isValid()) {
			auto * editor = new MediaTypeCombo(true, parent);

			for(const auto & mediaType : m_mediaTypes) {
				editor->addMediaType(mediaType);
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
			auto * combo = qobject_cast<MediaTypeCombo *>(editor);
			eqAssert(combo, "expected delegate editor to be a MediaTypeCombo (it's a " << qPrintable(editor->metaObject()->className()) << ")");
			combo->setCurrentText(index.data().value<QString>());
			combo->lineEdit()->selectAll();
		}
		else {
			auto * lineEdit = qobject_cast<QLineEdit *>(editor);
			eqAssert(lineEdit, "expected delegate editor to be a QLineEdit (it's a " << qPrintable(editor->metaObject()->className()) << ")");
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
			auto * combo = qobject_cast<MediaTypeCombo *>(editor);
			eqAssert(combo, "expected delegate editor to be a MediaTypeCombo (it's a " << qPrintable(editor->metaObject()->className()) << ")");

			if(!model->setData(index, combo->currentText())) {
				showNotification(combo, tr("<p>The file extension %1 could not have the media type %2 added.</p><p><small>Perhaps the file extension has already had that media type assigned?</small></p>").arg(model->data(parentIndex).value<QString>(), combo->currentText()), NotificationType::Warning);
			}
		}
		else {
			auto * lineEdit = qobject_cast<QLineEdit *>(editor);
			eqAssert(lineEdit, "expected delegate editor to be a QLineEdit (it's a " << qPrintable(editor->metaObject()->className()) << ")");

			if(!model->setData(index, lineEdit->text())) {
				showNotification(lineEdit, tr("<p>The file extension could not be set to %1.</p><p><small>Perhaps that file extension is already used elsewhere?</small></p>").arg(lineEdit->text()), NotificationType::Warning);
			}
		}
	}


}  // namespace Anansi
