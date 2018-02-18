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
/// \todo populate default mime type combo with all known mime types
///
/// \par Changes
/// - (2018-02) first version of this file.
#include "fileassociationswidget.h"
#include "ui_fileassociationswidget.h"

#include <iostream>

#include <QAbstractItemModel>

#include "fileassociationextensionitem.h"
#include "fileassociationmimetypeitem.h"
#include "fileassociationsitemdelegate.h"

namespace EquitWebServer {


	// typedef for the member function used for the value to compare when attempting to find an item
	template<class ObjectT, typename ResultT>
	using MemberFunctionForFindOperand = ResultT (ObjectT::*)() const;

	// helper template to find an item in the QTreeWidget. it iterates over an item's children and
	// compares a provided value (data) to the value returned from a member function (fn) as long
	// as it can be determined that the child item is of type ItemT (this determination is made by
	// checking its type() against ItemTypeId; ItemTypeId is inferred from ItemT unless explicitly
	// provided as a template instantiation argument). The search is recursive - it will continue
	// to search all root's children and their children until the entire subtree is exhausted or
	// a match is found. operator== must be implmented for operants of type DataT and the return
	// type of fn (which is currently also fixed as DataT).
	template<class ItemT, typename DataT, int ItemTypeId = ItemT::ItemType>
	ItemT * findItemHelper(QTreeWidgetItem * root, const DataT & data, MemberFunctionForFindOperand<ItemT, DataT> fn) {
		Q_ASSERT_X(root, __PRETTY_FUNCTION__, "item search root is null");
		Q_ASSERT_X(fn, __PRETTY_FUNCTION__, "item data operand member function is null");

		for(int idx = root->childCount() - 1; idx >= 0; --idx) {
			auto * item = root->child(idx);

			if(item->type() != ItemTypeId) {
				continue;
			}

			ItemT * realItem = static_cast<ItemT *>(item);

			if(std::invoke(fn, realItem) == data) {
				return realItem;
			}

			if(realItem = findItemHelper<ItemT, DataT>(item, data, fn); realItem) {
				return realItem;
			}
		}

		return nullptr;
	}


	FileAssociationsWidget::FileAssociationsWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(new Ui::FileAssociationsWidget) {
		m_ui->setupUi(this);

		m_ui->defaultMimeType->setCustomMimeTypesAllowed(true);

		connect(m_ui->addExtension, &QToolButton::clicked, [this]() {
			QString ext = tr("newextension");

			if(hasExtension(ext)) {
				int idx = 1;

				do {
					++idx;
					ext = tr("newextension%1").arg(idx);
				} while(hasExtension(ext));
			}

			addExtension(ext);
			setCurrentExtension(ext);
			m_ui->mimeTypes->editItem(m_ui->mimeTypes->currentItem());
		});

		connect(m_ui->removeEntry, &QToolButton::clicked, [this]() {
			for(auto * item : m_ui->mimeTypes->selectedItems()) {
				auto * parent = item->parent();
				parent->removeChild(item);

				if(item->type() == FileAssociationMimeTypeItem::ItemType) {
					Q_ASSERT_X(parent->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found MIME type item in file associations tree whose parent is not an extension item");
					Q_EMIT extensionMimeTypeRemoved(static_cast<FileAssociationExtensionItem *>(parent)->extension(), static_cast<FileAssociationMimeTypeItem *>(item)->mimeType());
				}
				else if(item->type() == FileAssociationExtensionItem::ItemType) {
					Q_EMIT extensionRemoved(static_cast<FileAssociationExtensionItem *>(item)->extension());
				}

				delete item;
			}
		});

		connect(m_ui->defaultMimeType, &MimeTypeCombo::currentMimeTypeChanged, this, &FileAssociationsWidget::defaultMimeTypeChanged);

		connect(m_ui->mimeTypes, &QTreeWidget::itemActivated, [](QTreeWidgetItem * item, int column) {
			if(0 != column) {
				return;
			}

			switch(item->type()) {
				case FileAssociationExtensionItem::ItemType:
					[[fallthrough]];
				case FileAssociationMimeTypeItem::ItemType:
					item->setData(0, DelegateItemOldDataRole, item->data(0, Qt::EditRole));
			}
		});

		connect(m_ui->mimeTypes, &QTreeWidget::itemChanged, [this](QTreeWidgetItem * item, int column) {
			std::cout << __PRETTY_FUNCTION__ << " received itemChanged() signal\n"
						 << std::flush;
			if(0 != column) {
				std::cout << __PRETTY_FUNCTION__ << "... not interested in column " << column << "\n"
							 << std::flush;
				return;
			}

			if(item->type() == FileAssociationExtensionItem::ItemType) {
				std::cout << __PRETTY_FUNCTION__ << "... it's an extension that's changed\n"
							 << std::flush;
				auto * extItem = static_cast<FileAssociationExtensionItem *>(item);
				std::cout << __PRETTY_FUNCTION__ << "... changed from \"" << qPrintable(item->data(0, DelegateItemOldDataRole).value<QString>()) << "\" to \"" << qPrintable(extItem->extension()) << "\"\n"
							 << std::flush;
				Q_EMIT extensionChanged(item->data(0, DelegateItemOldDataRole).value<QString>(), item->data(0, DelegateItemDataRole).value<QString>());
			}
			else if(item->type() == FileAssociationMimeTypeItem::ItemType) {
				auto * parent = item->parent();
				Q_ASSERT_X(parent->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found MIME type item in file associations tree whose parent is not an extension item");
				auto * mimeItem = static_cast<FileAssociationMimeTypeItem *>(item);
				Q_EMIT extensionMimeTypeChanged(static_cast<FileAssociationExtensionItem *>(parent)->extension(), mimeItem->previousMimeType(), mimeItem->mimeType());
			}
		});

		m_ui->mimeTypes->resizeColumnToContents(0);
		m_ui->mimeTypes->setItemDelegateForColumn(0, new FileAssociationsItemDelegate(this));
	}


	FileAssociationsWidget::~FileAssociationsWidget() = default;


	QString FileAssociationsWidget::defaultMimeType() const {
		return m_ui->defaultMimeType->currentMimeType();
	}


	/// \brief Fetch the extension for the current item.
	///
	/// If the current item is an extension item, the extension it represents is returned.
	/// If it's a MIME type item, the extension with which it is associated is returned.
	/// Otherwise, an empty string is returned.
	///
	/// \return The extension.
	///
	/// \see selectedExtension(), selectedExtensions()
	QString FileAssociationsWidget::currentExtension() const {
		auto * item = m_ui->mimeTypes->currentItem();

		while(item && item->type() != FileAssociationExtensionItem::ItemType) {
			item = item->parent();
		}

		if(!item) {
			return {};
		}

		return static_cast<FileAssociationExtensionItem *>(item)->extension();
	}


	QString FileAssociationsWidget::selectedExtension() const {
		for(const auto * item : m_ui->mimeTypes->selectedItems()) {
			if(item->type() == FileAssociationExtensionItem::ItemType) {
				return static_cast<const FileAssociationExtensionItem *>(item)->extension();
			}

			// only consider at most the first selected item
			break;
		}

		return {};
	}


	std::vector<QString> FileAssociationsWidget::selectedExtensions() const {
		std::vector<QString> ret;

		for(const auto * item : m_ui->mimeTypes->selectedItems()) {
			if(item->type() == FileAssociationExtensionItem::ItemType) {
				ret.push_back(static_cast<const FileAssociationExtensionItem *>(item)->extension());
			}
		}

		return ret;
	}


	/// \brief Fetch the MIME type for the current item.
	///
	/// If the current item is a MIME type item, the MIME type it represents is returned.
	/// Otherwise, an empty string is returned.
	///
	/// \return The MIME type.
	///
	/// \see selectedMimeType(), selectedMimeTypes()
	QString FileAssociationsWidget::currentMimeType() const {
		auto * item = m_ui->mimeTypes->currentItem();

		if(!item || item->type() != FileAssociationMimeTypeItem::ItemType) {
			return {};
		}

		return static_cast<FileAssociationMimeTypeItem *>(item)->mimeType();
	}


	QString FileAssociationsWidget::selectedMimeType() const {
		for(const auto * item : m_ui->mimeTypes->selectedItems()) {
			if(item->type() == FileAssociationMimeTypeItem::ItemType) {
				return static_cast<const FileAssociationMimeTypeItem *>(item)->mimeType();
			}

			// only consider at most the first selected item
			break;
		}

		return {};
	}


	std::vector<QString> FileAssociationsWidget::selectedMimeTypes() const {
		std::vector<QString> ret;

		for(const auto * item : m_ui->mimeTypes->selectedItems()) {
			if(item->type() == FileAssociationMimeTypeItem::ItemType) {
				ret.push_back(static_cast<const FileAssociationMimeTypeItem *>(item)->mimeType());
			}
		}

		return ret;
	}


	void FileAssociationsWidget::clear() {
		for(int idx = m_ui->mimeTypes->topLevelItemCount() - 1; idx >= 0; --idx) {
			auto * item = m_ui->mimeTypes->takeTopLevelItem(idx);
			Q_ASSERT_X(item->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found top-level item that is not an extension type");
			Q_EMIT extensionRemoved(static_cast<FileAssociationExtensionItem *>(item)->extension());
		}
	}


	void FileAssociationsWidget::addExtension(const QString & ext) {
		if(hasExtension(ext)) {
			return;
		}

		m_ui->mimeTypes->addTopLevelItem(new FileAssociationExtensionItem(ext));
		Q_EMIT extensionAdded(ext);
	}


	void FileAssociationsWidget::addExtensionMimeType(const QString & ext, const QString & mimeType) {
		auto * extItem = findExtensionItem(ext);

		if(extItem) {
			if(findItemHelper<FileAssociationMimeTypeItem, QString>(extItem, mimeType, &FileAssociationMimeTypeItem::mimeType)) {
				return;
			}
		}
		else {
			extItem = new FileAssociationExtensionItem(ext);
			m_ui->mimeTypes->addTopLevelItem(extItem);
			Q_EMIT extensionAdded(ext);
		}

		extItem->addChild(new FileAssociationMimeTypeItem(mimeType));
		Q_EMIT extensionMimeTypeAdded(ext, mimeType);
	}


	void FileAssociationsWidget::removeExtension(const QString & ext) {
		auto * item = findExtensionItem(ext);

		if(!item) {
			return;
		}

		item->parent()->removeChild(item);
		Q_EMIT extensionRemoved(ext);
		delete item;
	}


	void FileAssociationsWidget::removeExtensionMimeType(const QString & ext, const QString & mimeType) {
		auto * item = findMimeTypeItem(ext, mimeType);

		if(!item) {
			return;
		}

		item->parent()->removeChild(item);
		Q_EMIT extensionMimeTypeRemoved(ext, mimeType);
		delete item;
	}


	bool FileAssociationsWidget::setCurrentExtension(const QString & ext) {
		auto * curItem = m_ui->mimeTypes->currentItem();
		auto * newItem = findExtensionItem(ext);

		if(curItem != newItem) {
			m_ui->mimeTypes->setCurrentItem(newItem);
			Q_EMIT currentExtensionChanged(ext);
		}

		return nullptr != newItem;
	}


	bool FileAssociationsWidget::setCurrentExtensionMimeType(const QString & ext, const QString & mimeType) {
		auto * curItem = m_ui->mimeTypes->currentItem();
		auto * newItem = findMimeTypeItem(ext, mimeType);

		if(curItem != newItem) {
			bool extChanged = (currentExtension() != ext);
			m_ui->mimeTypes->setCurrentItem(newItem);

			if(extChanged) {
				Q_EMIT currentExtensionChanged(ext);
			}

			Q_EMIT currentExtensionMimeTypeChanged(ext, mimeType);
		}

		return nullptr != newItem;
	}


	void FileAssociationsWidget::setDefaultMimeType(const QString & mimeType) {
		m_ui->defaultMimeType->setCurrentMimeType(mimeType);
	}


	FileAssociationExtensionItem * FileAssociationsWidget::findExtensionItem(const QString & ext) const {
		return findItemHelper<FileAssociationExtensionItem, QString>(m_ui->mimeTypes->invisibleRootItem(), ext, &FileAssociationExtensionItem::extension);
	}


	FileAssociationMimeTypeItem * FileAssociationsWidget::findMimeTypeItem(const QString & ext, const QString & mimeType) const {
		auto * extItem = findItemHelper<FileAssociationExtensionItem, QString>(m_ui->mimeTypes->invisibleRootItem(), ext, &FileAssociationExtensionItem::extension);

		if(!extItem) {
			return nullptr;
		}

		return findItemHelper<FileAssociationMimeTypeItem, QString>(extItem, mimeType, &FileAssociationMimeTypeItem::mimeType);
	}


}  // namespace EquitWebServer
