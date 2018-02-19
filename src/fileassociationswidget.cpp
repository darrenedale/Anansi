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
#include <QMenu>
#include <QKeyEvent>

//#include "fileassociationextensionitem.h"
//#include "fileassociationmimetypeitem.h"
//#include "fileassociationsitemdelegate.h"
#include "serverfileassociationsmodel.h"


namespace EquitWebServer {


	// typedef for the member function used for the value to compare when attempting to find an item
	//	template<class ObjectT, typename ResultT>
	//	using MemberFunctionForFindOperand = ResultT (ObjectT::*)() const;

	//	// helper template to find an item in the QTreeWidget. it iterates over an item's children and
	//	// compares a provided value (data) to the value returned from a member function (fn) as long
	//	// as it can be determined that the child item is of type ItemT (this determination is made by
	//	// checking its type() against ItemTypeId; ItemTypeId is inferred from ItemT unless explicitly
	//	// provided as a template instantiation argument). The search is recursive - it will continue
	//	// to search all root's children and their children until the entire subtree is exhausted or
	//	// a match is found. operator== must be implmented for operants of type DataT and the return
	//	// type of fn (which is currently also fixed as DataT).
	//	template<class ItemT, typename DataT, int ItemTypeId = ItemT::ItemType>
	//	ItemT * findItemHelper(QTreeWidgetItem * root, const DataT & data, MemberFunctionForFindOperand<ItemT, DataT> fn) {
	//		Q_ASSERT_X(root, __PRETTY_FUNCTION__, "item search root is null");
	//		Q_ASSERT_X(fn, __PRETTY_FUNCTION__, "item data operand member function is null");

	//		for(int idx = root->childCount() - 1; idx >= 0; --idx) {
	//			auto * item = root->child(idx);

	//			if(item->type() != ItemTypeId) {
	//				continue;
	//			}

	//			ItemT * realItem = static_cast<ItemT *>(item);

	//			if(std::invoke(fn, realItem) == data) {
	//				return realItem;
	//			}

	//			if(realItem = findItemHelper<ItemT, DataT>(item, data, fn); realItem) {
	//				return realItem;
	//			}
	//		}

	//		return nullptr;
	//	}


	FileAssociationsWidget::FileAssociationsWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(new Ui::FileAssociationsWidget) {
		m_ui->setupUi(this);

		m_ui->defaultMimeType->setCustomMimeTypesAllowed(true);
		m_ui->defaultMimeType->addMimeType(QStringLiteral("application/octet-stream"));

		QMenu * addEntryMenu = new QMenu(this);
		addEntryMenu->addAction(m_ui->actionAddExtension);
		addEntryMenu->addAction(m_ui->actionAddMime);
		m_ui->addEntry->setMenu(addEntryMenu);

		//		auto addExtensionSlot = [this]() {
		//			QString ext = tr("newextension");

		//			if(hasExtension(ext)) {
		//				int idx = 1;

		//				do {
		//					++idx;
		//					ext = tr("newextension%1").arg(idx);
		//				} while(hasExtension(ext));
		//			}

		//			addExtension(ext);
		//			setCurrentExtension(ext);
		//			m_ui->fileExtensionMimeTypes->editItem(m_ui->fileExtensionMimeTypes->currentItem());
		//		};

		//		connect(m_ui->addEntry, &QPushButton::clicked, addExtensionSlot);
		//		connect(m_ui->actionAddExtension, &QAction::triggered, addExtensionSlot);

		//		connect(m_ui->actionAddMime, &QAction::triggered, [this]() {
		//			QString ext = currentExtension();

		//			if(ext.isEmpty()) {
		//				std::cerr << __PRETTY_FUNCTION__ << ": no current extension, can't add associated MIME type\n";
		//				return;
		//			}

		//			QString mimeType = tr("application/x-mysubtype");

		//			if(extensionHasMimeType(ext, mimeType)) {
		//				int idx = 1;

		//				do {
		//					++idx;
		//					ext = tr("application/x-mysubtype-%1").arg(idx);
		//				} while(extensionHasMimeType(ext, mimeType));
		//			}

		//			addExtensionMimeType(ext, mimeType);
		//			setCurrentExtensionMimeType(ext, mimeType);
		//			m_ui->fileExtensionMimeTypes->editItem(m_ui->fileExtensionMimeTypes->currentItem());
		//		});

		//		connect(m_ui->removeEntry, &QPushButton::clicked, [this]() {
		//			std::vector<QTreeWidgetItem *> alreadyRemoved;

		//			for(auto * item : m_ui->fileExtensionMimeTypes->selectedItems()) {
		//				if(alreadyRemoved.cend() != std::find(alreadyRemoved.cbegin(), alreadyRemoved.cend(), item)) {
		//					continue;
		//				}

		//				std::cerr << "removing item " << qPrintable(item->text(0)) << " \n"
		//							 << std::flush;
		//				auto * parent = item->parent();

		//				if(item->type() == FileAssociationMimeTypeItem::ItemType) {
		//					Q_ASSERT_X(parent->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found MIME type item in file associations tree whose parent is not an extension item");
		//					Q_EMIT extensionMimeTypeRemoved(static_cast<FileAssociationExtensionItem *>(parent)->extension(), static_cast<FileAssociationMimeTypeItem *>(item)->mimeType());
		//				}
		//				else if(item->type() == FileAssociationExtensionItem::ItemType) {
		//					Q_EMIT extensionRemoved(static_cast<FileAssociationExtensionItem *>(item)->extension());
		//				}

		//				delete item;
		//			}
		//		});

		//		connect(m_ui->defaultMimeType, &MimeTypeCombo::currentMimeTypeChanged, this, &FileAssociationsWidget::defaultMimeTypeChanged);

		//		connect(m_ui->fileExtensionMimeTypes, &QTreeWidget::itemActivated, [](QTreeWidgetItem * item, int column) {
		//			if(0 != column) {
		//				return;
		//			}

		//			switch(item->type()) {
		//				case FileAssociationExtensionItem::ItemType:
		//					[[fallthrough]];
		//				case FileAssociationMimeTypeItem::ItemType:
		//					item->setData(0, DelegateItemOldDataRole, item->data(0, Qt::EditRole));
		//			}
		//		});

		//		connect(m_ui->fileExtensionMimeTypes, &QTreeWidget::itemChanged, [this](QTreeWidgetItem * item, int column) {
		//			std::cout << __PRETTY_FUNCTION__ << " received itemChanged() signal\n"
		//						 << std::flush;
		//			if(0 != column) {
		//				std::cout << __PRETTY_FUNCTION__ << "... not interested in column " << column << "\n"
		//							 << std::flush;
		//				return;
		//			}

		//			if(item->type() == FileAssociationExtensionItem::ItemType) {
		//				//				auto * extItem = static_cast<FileAssociationExtensionItem *>(item);
		//				Q_EMIT extensionChanged(item->data(0, DelegateItemOldDataRole).value<QString>(), item->data(0, DelegateItemDataRole).value<QString>());
		//			}
		//			else if(item->type() == FileAssociationMimeTypeItem::ItemType) {
		//				std::cout << __PRETTY_FUNCTION__ << "... it's a MIME type that's changed\n"
		//							 << std::flush;
		//				auto * parent = item->parent();
		//				Q_ASSERT_X(parent->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found MIME type item in file associations tree whose parent is not an extension item");
		//				auto * mimeItem = static_cast<FileAssociationMimeTypeItem *>(item);
		//				std::cout << __PRETTY_FUNCTION__ << "... changed from \"" << qPrintable(mimeItem->data(0, DelegateItemOldDataRole).value<QString>()) << "\" to \"" << qPrintable(mimeItem->data(0, DelegateItemDataRole).value<QString>()) << "\" for extension \"" << qPrintable(static_cast<FileAssociationExtensionItem *>(parent)->extension()) << "\"\n"
		//							 << std::flush;
		//				Q_EMIT extensionMimeTypeChanged(static_cast<FileAssociationExtensionItem *>(parent)->extension(), mimeItem->previousMimeType(), mimeItem->mimeType());
		//			}
		//		});

		m_ui->fileExtensionMimeTypes->setColumnWidth(0, 100);
		m_ui->fileExtensionMimeTypes->setHeaderHidden(false);
		//		m_ui->fileExtensionMimeTypes->resizeColumnToContents(0);
		//		m_ui->fileExtensionMimeTypes->setItemDelegateForColumn(0, new FileAssociationsItemDelegate(this));
	}


	FileAssociationsWidget::FileAssociationsWidget(Server * server, QWidget * parent)
	: FileAssociationsWidget(parent) {
		setServer(server);
	}


	void FileAssociationsWidget::setServer(Server * server) {
		auto * oldSelectionModel = m_ui->fileExtensionMimeTypes->selectionModel();
		auto * oldModel = m_ui->fileExtensionMimeTypes->model();

		if(!server) {
			m_ui->fileExtensionMimeTypes->setModel(nullptr);
		}
		else {
			m_ui->fileExtensionMimeTypes->setModel(new ServerFileAssociationsModel(server, this));
		}

		delete oldModel;
		delete oldSelectionModel;
	}


	bool FileAssociationsWidget::hasExtension(const QString & ext) const {
		return findExtensionIndex(ext).isValid();
	}


	bool FileAssociationsWidget::extensionHasMimeType(const QString & ext, const QString & mimeType) const {
		return findMimeTypeIndex(ext, mimeType).isValid();
	}


	std::vector<QString> FileAssociationsWidget::availableMimeTypes() const {
		return m_ui->defaultMimeType->availableMimeTypes();
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
		auto idx = m_ui->fileExtensionMimeTypes->currentIndex();

		if(!idx.isValid()) {
			return {};
		}

		auto parentIdx = idx.parent();

		if(parentIdx.isValid()) {
			return parentIdx.data().value<QString>();
		}

		return idx.data().value<QString>();
	}


	QString FileAssociationsWidget::selectedExtension() const {
		for(const auto & idx : m_ui->fileExtensionMimeTypes->selectionModel()->selectedIndexes()) {
			if(idx.isValid() && !idx.parent().isValid()) {
				// valid index with no parent == extension item
				return idx.data().value<QString>();
			}

			// only consider at most the first selected item
			break;
		}

		return {};
	}


	std::vector<QString> FileAssociationsWidget::selectedExtensions() const {
		std::vector<QString> ret;

		for(const auto & idx : m_ui->fileExtensionMimeTypes->selectionModel()->selectedIndexes()) {
			if(idx.isValid() && !idx.parent().isValid()) {
				// valid index with no parent == extension item
				ret.push_back(idx.data().value<QString>());
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
		auto idx = m_ui->fileExtensionMimeTypes->currentIndex();

		if(idx.isValid() && idx.parent().isValid()) {
			// is valid and has a parent == MIME type item
			return idx.data().value<QString>();
		}

		return {};
	}


	QString FileAssociationsWidget::selectedMimeType() const {
		for(const auto & idx : m_ui->fileExtensionMimeTypes->selectionModel()->selectedIndexes()) {
			if(idx.isValid() && idx.parent().isValid()) {
				// valid index with valid parent == MIME type item
				return idx.data().value<QString>();
			}

			// only consider at most the first selected item
			break;
		}

		return {};
	}


	std::vector<QString> FileAssociationsWidget::selectedMimeTypes() const {
		std::vector<QString> ret;

		for(const auto & idx : m_ui->fileExtensionMimeTypes->selectionModel()->selectedIndexes()) {
			if(idx.isValid() && idx.parent().isValid()) {
				// valid index with valid parent == MIME type item
				ret.push_back(idx.data().value<QString>());
			}
		}

		return ret;
	}


	void FileAssociationsWidget::clear() {
		//		m_ui->fileExtensionMimeTypes->re
		//		for(int idx = m_ui->fileExtensionMimeTypes->topLevelItemCount() - 1; idx >= 0; --idx) {
		//			auto * item = m_ui->fileExtensionMimeTypes->takeTopLevelItem(idx);
		//			Q_ASSERT_X(item->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found top-level item that is not an extension type");
		//			Q_EMIT extensionRemoved(static_cast<FileAssociationExtensionItem *>(item)->extension());
		//		}
	}


	void FileAssociationsWidget::addAvailableMimeType(const QString & mimeType) {
		m_ui->defaultMimeType->addMimeType(mimeType);
	}


	void FileAssociationsWidget::addExtension(const QString & ext) {
		//		if(hasExtension(ext)) {
		//			return;
		//		}

		//		m_ui->fileExtensionMimeTypes->addTopLevelItem(new FileAssociationExtensionItem(ext));
		//		Q_EMIT extensionAdded(ext);
	}


	void FileAssociationsWidget::addExtensionMimeType(const QString & ext, const QString & mimeType) {
		//		auto * extItem = findExtensionItem(ext);

		//		if(extItem) {
		//			if(findItemHelper<FileAssociationMimeTypeItem, QString>(extItem, mimeType, &FileAssociationMimeTypeItem::mimeType)) {
		//				return;
		//			}
		//		}
		//		else {
		//			extItem = new FileAssociationExtensionItem(ext);
		//			m_ui->fileExtensionMimeTypes->addTopLevelItem(extItem);
		//			Q_EMIT extensionAdded(ext);
		//		}

		//		extItem->addChild(new FileAssociationMimeTypeItem(mimeType));
		//		Q_EMIT extensionMimeTypeAdded(ext, mimeType);
	}


	void FileAssociationsWidget::removeExtension(const QString & ext) {
		//		auto * item = findExtensionItem(ext);

		//		if(!item) {
		//			return;
		//		}

		//		item->parent()->removeChild(item);
		//		Q_EMIT extensionRemoved(ext);
		//		delete item;
	}


	void FileAssociationsWidget::removeExtensionMimeType(const QString & ext, const QString & mimeType) {
		//		auto * item = findMimeTypeItem(ext, mimeType);

		//		if(!item) {
		//			return;
		//		}

		//		item->parent()->removeChild(item);
		//		Q_EMIT extensionMimeTypeRemoved(ext, mimeType);
		//		delete item;
	}


	bool FileAssociationsWidget::setCurrentExtension(const QString & ext) {
		auto curIdx = m_ui->fileExtensionMimeTypes->currentIndex();
		auto newIdx = findExtensionIndex(ext);

		if(curIdx != newIdx) {
			m_ui->fileExtensionMimeTypes->setCurrentIndex(newIdx);
			Q_EMIT currentExtensionChanged(ext);
		}

		return newIdx.isValid();
	}


	bool FileAssociationsWidget::setCurrentExtensionMimeType(const QString & ext, const QString & mimeType) {
		auto curIdx = m_ui->fileExtensionMimeTypes->currentIndex();
		auto newIdx = findExtensionIndex(ext);

		if(curIdx != newIdx) {
			bool extChanged = (currentExtension() != ext);
			m_ui->fileExtensionMimeTypes->setCurrentIndex(newIdx);

			if(extChanged) {
				Q_EMIT currentExtensionChanged(ext);
			}

			Q_EMIT currentExtensionMimeTypeChanged(ext, mimeType);
		}

		return newIdx.isValid();
	}


	void FileAssociationsWidget::setDefaultMimeType(const QString & mimeType) {
		m_ui->defaultMimeType->setCurrentMimeType(mimeType);
	}


	QModelIndex FileAssociationsWidget::findExtensionIndex(const QString & ext) const {
		auto * model = m_ui->fileExtensionMimeTypes->model();

		for(int row = m_ui->fileExtensionMimeTypes->model()->rowCount(); row >= 0; --row) {
			auto idx = model->index(row, 0);

			if(idx.data().value<QString>() == ext) {
				return idx;
			}
		}

		return {};
	}


	QModelIndex FileAssociationsWidget::findMimeTypeIndex(const QString & ext, const QString & mimeType) const {
		auto extIndex = findExtensionIndex(ext);

		if(!extIndex.isValid()) {
			return extIndex;
		}

		auto * model = m_ui->fileExtensionMimeTypes->model();

		for(int row = m_ui->fileExtensionMimeTypes->model()->rowCount(extIndex); row >= 0; --row) {
			auto idx = model->index(row, 0, extIndex);

			if(idx.data().value<QString>() == mimeType) {
				return idx;
			}
		}

		return {};
	}


}  // namespace EquitWebServer
