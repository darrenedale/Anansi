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

/// \file fileassociationswidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the FileAssociationsWidget class.
///
/// \dep
/// - fileassociationswidget.h
/// - fileassociationswidget.ui
/// - <array>
/// - <vector>
/// - <iostream>
/// - <QMenu>
/// - <QKeyEvent>
/// - <QSignalBlocker>
/// - <QItemSelectionModel>
/// - <QModelIndex>
/// - <QModelIndexList>
/// - macros.h
/// - server.h
/// - fileassociationsitemdelegate.h
/// - fileassociationsmodel.h
///
/// NEXTRELEASE refactor item delegate so that it doesn't need to keep
/// reference to parent for list of media types, then make it a std::unique_ptr
/// member
///
/// \par Changes
/// - (2018-03) First release.

#include "fileassociationswidget.h"
#include "ui_fileassociationswidget.h"

#include <array>
#include <vector>
#include <iostream>

#include <QMenu>
#include <QKeyEvent>
#include <QSignalBlocker>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>

#include "macros.h"
#include "server.h"
#include "fileassociationsitemdelegate.h"
#include "fileassociationsmodel.h"


namespace Anansi {


	FileAssociationsWidget::FileAssociationsWidget(QWidget * parent)
	: QWidget(parent),
	  m_model(nullptr),
	  m_ui(std::make_unique<Ui::FileAssociationsWidget>()),
	  m_addEntryMenu(std::make_unique<QMenu>()),
	  m_server(nullptr) {
		m_ui->setupUi(this);

		m_ui->defaultMediaType->setCustomMediaTypesAllowed(true);
		m_ui->defaultMediaType->addMediaType(QStringLiteral("application/octet-stream"));

		m_addEntryMenu->addAction(m_ui->actionAddExtension);
		m_addEntryMenu->addAction(m_ui->actionAddMediaType);
		m_ui->addEntry->setMenu(m_addEntryMenu.get());

		auto addExtensionSlot = [this]() {
			auto idx = m_model->addFileExtension();

			if(!idx.isValid()) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to add new file extension\n";
				return;
			}

			Q_EMIT extensionAdded(idx.data().value<QString>());
			Q_EMIT extensionMediaTypeAdded(idx.data().value<QString>(), m_model->index(0, 0, idx).data().value<QString>());

			m_ui->fileExtensionMediaTypes->setCurrentIndex(idx);
			m_ui->fileExtensionMediaTypes->scrollTo(idx);
			m_ui->fileExtensionMediaTypes->edit(idx);
		};

		connect(m_ui->addEntry, &QPushButton::clicked, addExtensionSlot);
		connect(m_ui->actionAddExtension, &QAction::triggered, addExtensionSlot);

		connect(m_ui->actionAddMediaType, &QAction::triggered, [this]() {
			QString ext = currentExtension();

			if(ext.isEmpty()) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: no current extension, can't add associated media type\n";
				return;
			}

			auto idx = m_model->addFileExtensionMediaType(ext);

			if(!idx.isValid()) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to add media type for extension \"" << qPrintable(ext) << "\"\n";
				return;
			}

			Q_EMIT extensionMediaTypeAdded(idx.parent().data().value<QString>(), idx.data().value<QString>());
			m_ui->fileExtensionMediaTypes->setCurrentIndex(idx);
			m_ui->fileExtensionMediaTypes->scrollTo(idx);
			m_ui->fileExtensionMediaTypes->edit(idx);
		});

		connect(m_ui->removeEntry, &QPushButton::clicked, [this]() {
			// at present, the selection model is always single-select only
			const auto selection = m_ui->fileExtensionMediaTypes->selectionModel()->selectedIndexes();

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
				Q_EMIT extensionMediaTypeRemoved(parent.data().value<QString>(), removedData);
			}
			else {
				Q_EMIT extensionRemoved(removedData);
			}
			// if selection model changes to multi-select, use this as a basis for iterating the selection
			// and building a set of contiguous ranges of rows (per-parent)
			//			std::cout << "Selected rows:\n";

			//			for(const auto & index : m_ui->fileExtensionMediaTypes->selectionModel()->selectedRows()) {
			//				if(index.parent().isValid()) {
			//					std::cout << "   media type \"" << qPrintable(index.data().value<QString>()) << "\" for \"" << qPrintable(index.parent().data().value<QString>()) << "\" (" << index.row() << ", " << index.column() << ")\n";
			//				}
			//				else {
			//					std::cout << "   Extension type \"" << qPrintable(index.data().value<QString>()) << "\" (" << index.row() << ", " << index.column() << ")\n";
			//				}
			//			}

			//			std::cout << std::flush;

		});

		connect(m_ui->defaultMediaType, &MediaTypeCombo::currentMediaTypeChanged, [this](const QString & mediaType) {
			// can be null while setting up UI
			if(!m_server) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: server not yet set\n"
							 << std::flush;
				return;
			}

			m_server->configuration().setDefaultMediaType(mediaType);
			Q_EMIT defaultMediaTypeChanged(mediaType);
		});

		m_ui->fileExtensionMediaTypes->setHeaderHidden(false);
		m_ui->fileExtensionMediaTypes->setItemDelegateForColumn(0, new FileAssociationsItemDelegate(this));
	}


	FileAssociationsWidget::FileAssociationsWidget(Server * server, QWidget * parent)
	: FileAssociationsWidget(parent) {
		setServer(server);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	FileAssociationsWidget::~FileAssociationsWidget() = default;


	void FileAssociationsWidget::setServer(Server * server) {
		std::array<QSignalBlocker, 2> blocks = {{QSignalBlocker(m_ui->defaultMediaType), QSignalBlocker(m_ui->fileExtensionMediaTypes)}};
		m_server = server;

		if(!server) {
			m_model.reset(nullptr);
			m_ui->defaultMediaType->setCurrentMediaType(QStringLiteral("application/octet-stream"));
		}
		else {
			m_model = std::make_unique<FileAssociationsModel>(server);
			m_ui->defaultMediaType->clear();
			m_ui->defaultMediaType->addMediaType(QStringLiteral("application/octet-stream"));

			for(const auto & mediaType : server->configuration().allKnownMediaTypes()) {
				m_ui->defaultMediaType->addMediaType(mediaType);
			}

			m_ui->defaultMediaType->setCurrentMediaType(server->configuration().defaultMediaType());

			connect(m_model.get(), &FileAssociationsModel::extensionChanged, this, &FileAssociationsWidget::extensionChanged);
			connect(m_model.get(), &FileAssociationsModel::extensionMediaTypeChanged, this, &FileAssociationsWidget::extensionMediaTypeChanged);
		}

		auto * selectionModel = m_ui->fileExtensionMediaTypes->selectionModel();

		if(selectionModel) {
			selectionModel->disconnect(this);
		}

		m_ui->fileExtensionMediaTypes->setModel(m_model.get());
		selectionModel = m_ui->fileExtensionMediaTypes->selectionModel();

		if(selectionModel) {
			connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &FileAssociationsWidget::onFileExtensionsSelectionChanged, Qt::UniqueConnection);
		}
	}


	bool FileAssociationsWidget::hasExtension(const QString & ext) const {
		return m_model && m_model->findFileExtension(ext).isValid();
	}


	bool FileAssociationsWidget::extensionHasMediaType(const QString & ext, const QString & mediaType) const {
		return m_model && m_model->findFileExtensionMediaType(ext, mediaType).isValid();
	}


	std::vector<QString> FileAssociationsWidget::availableMediaTypes() const {
		return m_ui->defaultMediaType->availableMediaTypes();
	}


	QString FileAssociationsWidget::defaultMediaType() const {
		return m_ui->defaultMediaType->currentMediaType();
	}


	QString FileAssociationsWidget::currentExtension() const {
		auto idx = m_ui->fileExtensionMediaTypes->currentIndex();

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
		for(const auto & idx : m_ui->fileExtensionMediaTypes->selectionModel()->selectedIndexes()) {
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

		for(const auto & idx : m_ui->fileExtensionMediaTypes->selectionModel()->selectedIndexes()) {
			if(idx.isValid() && !idx.parent().isValid()) {
				// valid index with no parent == extension item
				ret.push_back(idx.data().value<QString>());
			}
		}

		return ret;
	}


	QString FileAssociationsWidget::currentMediaType() const {
		auto idx = m_ui->fileExtensionMediaTypes->currentIndex();

		if(idx.isValid() && idx.parent().isValid()) {
			// is valid and has a parent == media type item
			return idx.data().value<QString>();
		}

		return {};
	}


	QString FileAssociationsWidget::selectedMediaType() const {
		for(const auto & idx : m_ui->fileExtensionMediaTypes->selectionModel()->selectedIndexes()) {
			if(idx.isValid() && idx.parent().isValid()) {
				// valid index with valid parent == media type item
				return idx.data().value<QString>();
			}

			// only consider at most the first selected item
			break;
		}

		return {};
	}


	std::vector<QString> FileAssociationsWidget::selectedMediaTypes() const {
		std::vector<QString> ret;

		for(const auto & idx : m_ui->fileExtensionMediaTypes->selectionModel()->selectedIndexes()) {
			if(idx.isValid() && idx.parent().isValid()) {
				// valid index with valid parent == media type item
				ret.push_back(idx.data().value<QString>());
			}
		}
		return ret;
	}


	void FileAssociationsWidget::clear() {
		if(!m_model) {
			return;
		}

		m_model->clear();
	}


	void FileAssociationsWidget::addAvailableMediaType(const QString & mediaType) {
		// NEXTRELEASE this should also be added to the item delegate
		m_ui->defaultMediaType->addMediaType(mediaType);
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


	bool FileAssociationsWidget::addExtensionMediaType(const QString & ext, const QString & mediaType) {
		if(!m_model) {
			return false;
		}

		auto idx = m_model->addFileExtensionMediaType(ext, mediaType);

		if(!idx.isValid()) {
			return false;
		}

		Q_EMIT extensionMediaTypeAdded(ext, mediaType);
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


	void FileAssociationsWidget::removeExtensionMediaType(const QString & ext, const QString & mediaType) {
		if(!m_model) {
			return;
		}

		const auto idx = m_model->findFileExtensionMediaType(ext, mediaType);

		if(!idx.isValid()) {
			return;
		}

		if(!m_model->removeRow(idx.row(), idx.parent())) {
			return;
		}

		Q_EMIT extensionMediaTypeRemoved(ext, mediaType);
	}


	bool FileAssociationsWidget::setCurrentExtension(const QString & ext) {
		if(!m_model) {
			return false;
		}

		auto curIdx = m_ui->fileExtensionMediaTypes->currentIndex();
		auto newIdx = m_model->findFileExtension(ext);

		if(curIdx != newIdx) {
			m_ui->fileExtensionMediaTypes->setCurrentIndex(newIdx);
			Q_EMIT currentExtensionChanged(ext);
		}

		return newIdx.isValid();
	}


	bool FileAssociationsWidget::setCurrentExtensionMediaType(const QString & ext, const QString & mediaType) {
		if(!m_model) {
			return false;
		}

		auto curIdx = m_ui->fileExtensionMediaTypes->currentIndex();
		auto newIdx = m_model->findFileExtension(ext);

		if(curIdx != newIdx) {
			bool extChanged = (currentExtension() != ext);
			m_ui->fileExtensionMediaTypes->setCurrentIndex(newIdx);

			if(extChanged) {
				Q_EMIT currentExtensionChanged(ext);
			}

			Q_EMIT currentExtensionMediaTypeChanged(ext, mediaType);
		}

		return newIdx.isValid();
	}


	void FileAssociationsWidget::setDefaultMediaType(const QString & mediaType) {
		m_ui->defaultMediaType->setCurrentMediaType(mediaType);
	}


	void FileAssociationsWidget::onFileExtensionsSelectionChanged() {
		auto * selectionModel = m_ui->fileExtensionMediaTypes->selectionModel();
		m_ui->removeEntry->setEnabled(selectionModel && !selectionModel->selectedIndexes().isEmpty());
	}


}  // namespace Anansi
