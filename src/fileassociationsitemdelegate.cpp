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
/// \todo populate mime editor combo with all known mime types
///
/// \par Changes
/// - (2018-02) first version of this file.

#include "fileassociationsitemdelegate.h"

#include <QLineEdit>

#include "fileassociationswidget.h"
#include "fileassociationmimetypeitem.h"
#include "fileassociationextensionitem.h"
#include "mimetypecombo.h"


namespace EquitWebServer {


	FileAssociationsItemDelegate::FileAssociationsItemDelegate(FileAssociationsWidget * parent)
	: QStyledItemDelegate(parent) {
	}


	QWidget * FileAssociationsItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
		if(0 == index.column()) {
			switch(index.data(FileAssociationsWidget::DelegateItemTypeRole).value<int>()) {
				case FileAssociationMimeTypeItem::ItemType:
					return new MimeTypeCombo(true, parent);

				case FileAssociationExtensionItem::ItemType:
					return new QLineEdit(parent);
			}
		}

		return QStyledItemDelegate::createEditor(parent, option, index);
	}


	void FileAssociationsItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(0 == index.column()) {
			switch(index.data(FileAssociationsWidget::DelegateItemTypeRole).value<int>()) {
				case FileAssociationMimeTypeItem::ItemType: {
					auto * combo = static_cast<MimeTypeCombo *>(editor);
					combo->setCurrentMimeType(index.data(FileAssociationsWidget::DelegateItemDataRole).value<QString>());
					return;
				}

				case FileAssociationExtensionItem::ItemType: {
					auto * lineEdit = static_cast<QLineEdit *>(editor);
					lineEdit->setText(index.data(FileAssociationsWidget::DelegateItemDataRole).value<QString>());
					return;
				}
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	void FileAssociationsItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(0 == index.column()) {
			switch(index.data(FileAssociationsWidget::DelegateItemTypeRole).value<int>()) {
				case FileAssociationMimeTypeItem::ItemType: {
					auto * combo = static_cast<MimeTypeCombo *>(editor);
					model->setData(index, combo->currentMimeType());
					return;
				}

				case FileAssociationExtensionItem::ItemType: {
					auto * lineEdit = static_cast<QLineEdit *>(editor);
					model->setData(index, lineEdit->text());
					return;
				}
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	FileAssociationsItemDelegate::~FileAssociationsItemDelegate() = default;


}  // namespace EquitWebServer
