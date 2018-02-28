/// \file fileassociationsitemdelegate.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the FileAssociationsItemDelegate class.
///
/// \dep
/// - fileassociationsitemdelegate.h
/// - fileassociationswidget.ui
/// - fileassociationextensionitem.h
/// - fileassociationmimetypeitem.h
///
/// \par Changes
/// - (2018-02) first version of this file.

#include "fileassociationsitemdelegate.h"

#include <iostream>

#include <QLineEdit>
#include <QDebug>

#include "fileassociationswidget.h"
#include "mimecombo.h"


namespace EquitWebServer {


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

		if(index.parent().isValid()) {
			auto * combo = qobject_cast<MimeCombo *>(editor);
			Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected delegate editor to be a MimeTypeCombo");

			if(!model->setData(index, combo->currentText())) {
				// TODO trigger display of error message - MIME type already set?
			}
		}
		else {
			// TODO check for duplicate extension
			auto * lineEdit = qobject_cast<QLineEdit *>(editor);
			Q_ASSERT_X(lineEdit, __PRETTY_FUNCTION__, "expected delegate editor to be a QLineEdit");

			if(!model->setData(index, lineEdit->text())) {
				// TODO trigger display of error message - duplicate extension?
			}
		}
	}


	FileAssociationsItemDelegate::~FileAssociationsItemDelegate() = default;


}  // namespace EquitWebServer
