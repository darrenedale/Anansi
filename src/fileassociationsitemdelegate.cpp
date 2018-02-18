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

#include <iostream>

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
				case FileAssociationMimeTypeItem::ItemType: {
					auto * editor = new MimeTypeCombo(true, parent);

					//					for(const auto & mimeType : m_config->registeredMimeTypes()) {
					//						editor->addMimeType(mimeType);
					//					}

					return editor;
				}

				case FileAssociationExtensionItem::ItemType: {
					auto * editor = new QComboBox(parent);
					editor->setEditable(true);

					//					for(const auto & ext : m_config->registeredFileExtensions()) {
					//						editor->addItem(ext);
					//					}

					return editor;
				}
			}
		}

		return QStyledItemDelegate::createEditor(parent, option, index);
	}


	void FileAssociationsItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(0 == index.column()) {
			switch(index.data(FileAssociationsWidget::DelegateItemTypeRole).value<int>()) {
				case FileAssociationMimeTypeItem::ItemType: {
					auto * combo = static_cast<MimeTypeCombo *>(editor);
					combo->setCurrentMimeType(index.data(Qt::EditRole).value<QString>());
					return;
				}

				case FileAssociationExtensionItem::ItemType: {
					auto * combo = static_cast<QComboBox *>(editor);
					combo->setCurrentText(index.data(Qt::EditRole).value<QString>());

					return;
				}
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	//	void FileAssociationsItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	//	}


	void FileAssociationsItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(0 == index.column()) {
			switch(index.data(FileAssociationsWidget::DelegateItemTypeRole).value<int>()) {
				case FileAssociationMimeTypeItem::ItemType: {
					auto * combo = static_cast<MimeTypeCombo *>(editor);
					QSignalBlocker blocker(model);
					model->setData(index, index.data(FileAssociationsWidget::DelegateItemDataRole), FileAssociationsWidget::DelegateItemOldDataRole);
					model->setData(index, combo->currentMimeType());
					blocker.unblock();
					model->setData(index, combo->currentMimeType(), FileAssociationsWidget::DelegateItemDataRole);
					return;
				}

				case FileAssociationExtensionItem::ItemType: {
					QSignalBlocker blocker(model);
					auto * combo = static_cast<QComboBox *>(editor);
					model->setData(index, index.data(FileAssociationsWidget::DelegateItemDataRole), FileAssociationsWidget::DelegateItemOldDataRole);
					model->setData(index, combo->currentText());
					blocker.unblock();
					model->setData(index, combo->currentText(), FileAssociationsWidget::DelegateItemDataRole);
					return;
				}
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	FileAssociationsItemDelegate::~FileAssociationsItemDelegate() = default;


}  // namespace EquitWebServer
