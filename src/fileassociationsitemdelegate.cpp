/// \file fileassociationswidget.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the FileAssociationsWidget class.
///
/// \dep
/// - fileassociationswidget.h
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
#include "mimetypecombo.h"


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
			auto * editor = new MimeTypeCombo(true, parent);

			if(m_parent) {
				for(const auto & mimeType : m_parent->availableMimeTypes()) {
					editor->addMimeType(mimeType);
				}
			}

			editor->setCurrentText(index.data().value<QString>());
			return editor;
		}

		auto * editor = new QComboBox(parent);
		editor->setEditable(true);

		//					for(const auto & ext : m_config->registeredFileExtensions()) {
		//						editor->addItem(ext);
		//					}

		editor->setCurrentText(index.data().value<QString>());
		return editor;
	}


	void FileAssociationsItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		if(index.parent().isValid()) {
			auto * combo = static_cast<MimeTypeCombo *>(editor);
			combo->setCurrentText(index.data().value<QString>());
			combo->lineEdit()->selectAll();
			return;
		}

		auto * combo = static_cast<QComboBox *>(editor);
		combo->setCurrentText(index.data().value<QString>());
		combo->lineEdit()->selectAll();
	}


	void FileAssociationsItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		if(index.parent().isValid()) {
			// TODO pressing [return] in the MIME type editor is clearing its content before we reach here for the first
			// MIME type added to a newly-created filename extension entry
			auto * combo = qobject_cast<MimeTypeCombo *>(editor);

			if(!combo) {
				return;
			}

			model->setData(index, combo->currentText());
			return;
		}

		// TODO check for duplicate extension
		auto * combo = qobject_cast<QComboBox *>(editor);

		if(!combo) {
			return;
		}

		model->setData(index, combo->currentText());
		return;
	}


	FileAssociationsItemDelegate::~FileAssociationsItemDelegate() = default;


}  // namespace EquitWebServer
