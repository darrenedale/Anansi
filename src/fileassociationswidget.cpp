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
#include "fileassociationswidget.h"
#include "ui_fileassociationswidget.h"

#include <iostream>

#include <QMenu>
#include <QKeyEvent>

#include "server.h"
#include "fileassociationsitemdelegate.h"
#include "serverfileassociationsmodel.h"


namespace EquitWebServer {


	FileAssociationsWidget::FileAssociationsWidget(QWidget * parent)
	: QWidget(parent),
	  m_model(nullptr),
	  m_ui(new Ui::FileAssociationsWidget),
	  m_server(nullptr) {
		m_ui->setupUi(this);

		m_ui->defaultMimeType->setCustomMimeTypesAllowed(true);
		m_ui->defaultMimeType->addMimeType(QStringLiteral("application/octet-stream"));

		QMenu * addEntryMenu = new QMenu(this);
		addEntryMenu->addAction(m_ui->actionAddExtension);
		addEntryMenu->addAction(m_ui->actionAddMime);
		m_ui->addEntry->setMenu(addEntryMenu);

		auto addExtensionSlot = [this]() {
			auto idx = m_model->addFileExtension();

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << ": failed to add new file extension\n";
				return;
			}

			Q_EMIT extensionAdded(idx.data().value<QString>());
			Q_EMIT extensionMimeTypeAdded(idx.data().value<QString>(), m_model->index(0, 0, idx).data().value<QString>());

			m_ui->fileExtensionMimeTypes->setCurrentIndex(idx);
			m_ui->fileExtensionMimeTypes->scrollTo(idx);
			m_ui->fileExtensionMimeTypes->edit(idx);
		};

		connect(m_ui->addEntry, &QPushButton::clicked, addExtensionSlot);
		connect(m_ui->actionAddExtension, &QAction::triggered, addExtensionSlot);

		connect(m_ui->actionAddMime, &QAction::triggered, [this]() {
			QString ext = currentExtension();

			if(ext.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << ": no current extension, can't add associated MIME type\n";
				return;
			}

			auto idx = m_model->addFileExtensionMimeType(ext);

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << ": failed to add MIME type for extension \"" << qPrintable(ext) << "\"\n";
				return;
			}

			Q_EMIT extensionMimeTypeAdded(idx.parent().data().value<QString>(), idx.data().value<QString>());
			m_ui->fileExtensionMimeTypes->setCurrentIndex(idx);
			m_ui->fileExtensionMimeTypes->scrollTo(idx);
			m_ui->fileExtensionMimeTypes->edit(idx);
		});

		connect(m_ui->removeEntry, &QPushButton::clicked, [this]() {
			// at present, the selection model is always single-select only
			const auto selection = m_ui->fileExtensionMimeTypes->selectionModel()->selectedIndexes();

			if(0 == selection.size()) {
				return;
			}

			const auto & idx = selection[0];
			const auto parent = idx.parent();
			const auto removedData = idx.data().value<QString>();

			if(!m_model->removeRow(idx.row(), parent)) {
				return;
			}

			if(parent.isValid()) {
				Q_EMIT extensionMimeTypeRemoved(parent.data().value<QString>(), removedData);
			}
			else {
				Q_EMIT extensionRemoved(removedData);
			}
			// if selection model changes to multi-select, use this as a basis for iterating the selection
			// and building a set of contiguous ranges of rows (per-parent)
			//			std::cout << "Selected rows:\n";

			//			for(const auto & index : m_ui->fileExtensionMimeTypes->selectionModel()->selectedRows()) {
			//				if(index.parent().isValid()) {
			//					std::cout << "   MIME type \"" << qPrintable(index.data().value<QString>()) << "\" for \"" << qPrintable(index.parent().data().value<QString>()) << "\" (" << index.row() << ", " << index.column() << ")\n";
			//				}
			//				else {
			//					std::cout << "   Extension type \"" << qPrintable(index.data().value<QString>()) << "\" (" << index.row() << ", " << index.column() << ")\n";
			//				}
			//			}

			//			std::cout << std::flush;

		});

		connect(m_ui->defaultMimeType, &MimeTypeCombo::currentMimeTypeChanged, [this](const QString & mimeType) {
			// can be null while setting up UI
			if(!m_server) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: server not yet set\n"
							 << std::flush;
				return;
			}

			m_server->configuration().setDefaultMimeType(mimeType);
			Q_EMIT defaultMimeTypeChanged(mimeType);
		});

		m_ui->fileExtensionMimeTypes->setHeaderHidden(false);
		m_ui->fileExtensionMimeTypes->setItemDelegateForColumn(0, new FileAssociationsItemDelegate(this));
	}


	FileAssociationsWidget::FileAssociationsWidget(Server * server, QWidget * parent)
	: FileAssociationsWidget(parent) {
		setServer(server);
	}


	void FileAssociationsWidget::setServer(Server * server) {
		std::array<QSignalBlocker, 2> blocks = {{QSignalBlocker(m_ui->defaultMimeType), QSignalBlocker(m_ui->fileExtensionMimeTypes)}};
		m_server = server;

		if(!server) {
			m_model.reset(nullptr);
			m_ui->defaultMimeType->setCurrentMimeType(QStringLiteral("application/octet-stream"));
		}
		else {
			m_model = std::make_unique<ServerFileAssociationsModel>(server);
			m_ui->defaultMimeType->clear();
			m_ui->defaultMimeType->addMimeType("application/octet-stream");

			for(const auto & mimeType : server->configuration().allKnownMimeTypes()) {
				m_ui->defaultMimeType->addMimeType(mimeType);
			}

			m_ui->defaultMimeType->setCurrentMimeType(server->configuration().defaultMimeType());

			connect(m_model.get(), &ServerFileAssociationsModel::extensionChanged, this, &FileAssociationsWidget::extensionChanged);
			connect(m_model.get(), &ServerFileAssociationsModel::extensionMimeTypeChanged, this, &FileAssociationsWidget::extensionMimeTypeChanged);
		}

		auto * selectionModel = m_ui->fileExtensionMimeTypes->selectionModel();

		if(selectionModel) {
			selectionModel->disconnect(this);
		}

		m_ui->fileExtensionMimeTypes->setModel(m_model.get());
		selectionModel = m_ui->fileExtensionMimeTypes->selectionModel();

		if(selectionModel) {
			connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &FileAssociationsWidget::onFileExtensionsSelectionChanged, Qt::UniqueConnection);
		}
	}


	bool FileAssociationsWidget::hasExtension(const QString & ext) const {
		return m_model && m_model->findFileExtension(ext).isValid();
	}


	bool FileAssociationsWidget::extensionHasMimeType(const QString & ext, const QString & mimeType) const {
		return m_model && m_model->findFileExtensionMimeType(ext, mimeType).isValid();
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


	bool FileAssociationsWidget::addExtension(const QString & ext) {
		if(!m_model) {
			return false;
		}

		auto idx = m_model->addFileExtension(ext);

		if(!idx.isValid()) {
			return false;
		}

		Q_EMIT extensionAdded(ext);
		return true;
	}


	bool FileAssociationsWidget::addExtensionMimeType(const QString & ext, const QString & mimeType) {
		if(!m_model) {
			return false;
		}

		auto idx = m_model->addFileExtensionMimeType(ext, mimeType);

		if(!idx.isValid()) {
			return false;
		}

		Q_EMIT extensionMimeTypeAdded(ext, mimeType);
		return true;
	}


	void FileAssociationsWidget::removeExtension(const QString & ext) {
		if(!m_model) {
			return;
		}

		const auto idx = m_model->findFileExtension(ext);

		if(!idx.isValid()) {
			return;
		}

		if(!m_model->removeRow(idx.row(), {})) {
			return;
		}

		Q_EMIT extensionRemoved(ext);
	}


	void FileAssociationsWidget::removeExtensionMimeType(const QString & ext, const QString & mimeType) {
		if(!m_model) {
			return;
		}

		const auto idx = m_model->findFileExtensionMimeType(ext, mimeType);

		if(!idx.isValid()) {
			return;
		}

		if(!m_model->removeRow(idx.row(), idx.parent())) {
			return;
		}

		Q_EMIT extensionMimeTypeRemoved(ext, mimeType);
	}


	bool FileAssociationsWidget::setCurrentExtension(const QString & ext) {
		if(!m_model) {
			return false;
		}

		auto curIdx = m_ui->fileExtensionMimeTypes->currentIndex();
		auto newIdx = m_model->findFileExtension(ext);

		if(curIdx != newIdx) {
			m_ui->fileExtensionMimeTypes->setCurrentIndex(newIdx);
			Q_EMIT currentExtensionChanged(ext);
		}

		return newIdx.isValid();
	}


	bool FileAssociationsWidget::setCurrentExtensionMimeType(const QString & ext, const QString & mimeType) {
		if(!m_model) {
			return false;
		}

		auto curIdx = m_ui->fileExtensionMimeTypes->currentIndex();
		auto newIdx = m_model->findFileExtension(ext);

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


	void FileAssociationsWidget::onFileExtensionsSelectionChanged() {
		auto * selectionModel = m_ui->fileExtensionMimeTypes->selectionModel();
		m_ui->removeEntry->setEnabled(selectionModel && !selectionModel->selectedIndexes().isEmpty());
	}


}  // namespace EquitWebServer
