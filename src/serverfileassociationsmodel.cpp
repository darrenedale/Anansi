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

/// \file serverfileassociationsmodel.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the ServerFileAssociationsModel class.
///
/// \dep
/// - <iostream>
/// - server.h
/// - mimeicons.h
///
/// \par Changes
/// - (2018-03) First release.

#include "serverfileassociationsmodel.h"

#include <iostream>

#include "server.h"
#include "mimeicons.h"


namespace Anansi {


	ServerFileAssociationsModel::ServerFileAssociationsModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server to observe must not be null");
	}


	QModelIndex ServerFileAssociationsModel::findFileExtension(const QString & ext) const {
		const auto extensions = m_server->configuration().registeredFileExtensions();
		const auto & begin = extensions.cbegin();
		const auto & end = extensions.cend();
		const auto extIt = std::find(begin, end, ext);

		if(extIt == end) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, extIt)), 0, static_cast<quintptr>(0));
	}


	QModelIndex ServerFileAssociationsModel::findMimeType(const QString & mimeType, const QModelIndex & parent) const {
		if(!parent.isValid()) {
			return {};
		}

		if(parent.parent().isValid()) {
			// provided parent is a MIME type item
			return {};
		}

		const auto mimeTypes = m_server->configuration().fileExtensionMimeTypes(parent.data().value<QString>());
		const auto & begin = mimeTypes.cbegin();
		const auto & end = mimeTypes.cend();
		const auto mimeTypeIt = std::find(begin, end, mimeType);

		if(end == mimeTypeIt) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, mimeTypeIt)), 0, static_cast<quintptr>(parent.row() + 1));
	}


	QModelIndex ServerFileAssociationsModel::index(int row, int column, const QModelIndex & parent) const {
		if(0 != column) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid column (" << column << ")\n";
			return {};
		}

		if(0 > row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid row (" << row << ")\n";
			return {};
		}

		if(parent.isValid()) {
			if(0 == parent.internalId()) {
				// extension items have associated MIME types as children
				if(m_server->configuration().fileExtensionMimeTypeCount(parent.data().value<QString>()) <= row) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: row for MIME type item index is out of bounds\n";
					return {};
				}

				// ID is parent row + 1. this leaves ID = 0 to be used as an indicator
				// that an index is an extension item index. the ID for MIME type items
				// has 1 subtracted to find the row index of its parent extension item
				return createIndex(row, column, static_cast<quintptr>(parent.row() + 1));
			}

			// if parent's ID is > 0, it's a MIME type item, which has no children, so just return
			// and invalid index
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: parent index does not have any children\n";
			return {};
		}

		// for anything else, return a top-level extension item index
		if(rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: row for extension item index is out of bounds\n";
			return {};
		}

		return createIndex(row, column, static_cast<quintptr>(0));
	}


	QModelIndex ServerFileAssociationsModel::parent(const QModelIndex & index) const {
		if(!index.isValid()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid index == invalid parent\n";
			return {};
		}

		if(0 == index.internalId()) {
			// this is an extension item index and as such it has no parent
			return {};
		}

		return createIndex(static_cast<int>(index.internalId() - 1), 0, static_cast<quintptr>(0));
	}


	int ServerFileAssociationsModel::rowCount(const QModelIndex & parent) const {
		if(parent.isValid()) {
			if(0 < parent.internalId()) {
				// MIME type items don't have children
				return 0;
			}

			return m_server->configuration().fileExtensionMimeTypeCount(parent.data().value<QString>());
		}

		return m_server->configuration().registeredFileExtensionCount();
	}


	int ServerFileAssociationsModel::columnCount(const QModelIndex &) const {
		return 1;
	}


	QVariant ServerFileAssociationsModel::data(const QModelIndex & index, int role) const {
		if(Qt::DisplayRole != role && Qt::EditRole != role && Qt::DecorationRole != role) {
			return {};
		}

		if(!index.isValid()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		if(0 != index.column()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: index column must be 0\n";
			return {};
		}

		if(0 == index.internalId()) {
			if(Qt::DecorationRole == role) {
				return {};
			}

			int idx = index.row();
			const auto & config = m_server->configuration();

			if(0 > idx || config.registeredFileExtensionCount() <= idx) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extensions index row is not valid\n";
				return {};
			}

			return config.registeredFileExtensions()[idx];
		}

		int idx = static_cast<int>(index.internalId() - 1);
		const auto & config = m_server->configuration();

		if(0 > idx || config.registeredFileExtensionCount() <= idx) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid parent row index\n";
			return {};
		}

		const auto ext = config.registeredFileExtensions()[idx];

		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: empty extension when looking up MIME type index\n";
			return {};
		}

		idx = index.row();

		if(0 > idx || config.fileExtensionMimeTypeCount(ext) <= idx) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: MIME type index row is not valid\n";
			return {};
		}

		const auto mimeTypes = config.fileExtensionMimeTypes(ext);

		if(Qt::DecorationRole == role) {
			return mimeIcon(mimeTypes[static_cast<std::size_t>(idx)]);
		}

		return mimeTypes[static_cast<std::size_t>(idx)];
	}


	Qt::ItemFlags ServerFileAssociationsModel::flags(const QModelIndex & index) const {
		auto ret = QAbstractItemModel::flags(index);

		if(index.isValid()) {
			ret |= Qt::ItemIsEditable;

			if(index.parent().isValid()) {
				// it's a MIME type item, which never has any children
				ret |= Qt::ItemNeverHasChildren;
			}
		}

		return ret;
	}


	QVariant ServerFileAssociationsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if(Qt::DisplayRole != role) {
			return QAbstractItemModel::headerData(section, orientation, role);
		}

		if(0 == section && Qt::Horizontal == orientation) {
			return tr("MIME type associations");
		}

		return {};
	}


	bool ServerFileAssociationsModel::setData(const QModelIndex & index, const QVariant & value, int role) {
		if(!index.isValid()) {
			return false;
		}

		if(Qt::EditRole != role) {
			return QAbstractItemModel::setData(index, value, role);
		}

		const auto parent = index.parent();

		if(parent.isValid()) {
			// it's a MIME type item
			// this call does all the validation necessary
			const auto ext = parent.data().value<QString>();
			const auto oldMime = index.data().value<QString>();
			const auto newMime = value.value<QString>();

			if(oldMime == newMime) {
				return true;
			}

			auto & config = m_server->configuration();

			if(config.fileExtensionHasMimeType(ext, newMime)) {
				return false;
			}

			if(!config.changeFileExtensionMimeType(ext, oldMime, newMime)) {
				return false;
			}

			Q_EMIT dataChanged(index, index, QVector<int>() << Qt::DisplayRole << Qt::EditRole);
			Q_EMIT extensionMimeTypeChanged(ext, oldMime, newMime);
			return true;
		}

		// it's a file extension item
		// this call does all the validation necessary
		if(!m_server->configuration().changeFileExtension(index.data().value<QString>(), value.value<QString>())) {
			return false;
		}

		const auto oldExt = index.data().value<QString>();
		const auto newExt = value.value<QString>();

		// changing an extension causes the underlying storage map to rehash its key,
		// therefore extensions are likely to be reordered so all indexes will be
		// potentially invalidated
		beginResetModel();
		endResetModel();
		Q_EMIT extensionChanged(oldExt, newExt);
		return true;
	}


	QModelIndex ServerFileAssociationsModel::addFileExtension(QString ext, QString mimeType) {
		auto & config = m_server->configuration();

		if(ext.isEmpty()) {
			ext = tr("newextension");

			if(config.fileExtensionIsRegistered(ext)) {
				int idx = 1;

				do {
					++idx;
					ext = tr("newextension%1").arg(idx);
				} while(config.fileExtensionIsRegistered(ext));
			}
		}
		else if(config.fileExtensionIsRegistered(ext)) {
			return {};
		}

		if(mimeType.isEmpty()) {
			mimeType = tr("application/octet-stream");
		}

		if(!config.addFileExtensionMimeType(ext, mimeType)) {
			return {};
		}

		beginResetModel();
		endResetModel();
		return findFileExtension(ext);
	}


	QModelIndex ServerFileAssociationsModel::addFileExtensionMimeType(QString ext, QString mimeType) {
		if(ext.isEmpty()) {
			return {};
		}

		auto & config = m_server->configuration();

		if(mimeType.isEmpty()) {
			mimeType = QStringLiteral("application/x-subtype");

			if(config.fileExtensionHasMimeType(ext, mimeType)) {
				int idx = 1;

				do {
					++idx;
					ext = QStringLiteral("application/x-subtype-%1").arg(idx);
				} while(config.fileExtensionHasMimeType(ext, mimeType));
			}
		}
		else if(config.fileExtensionHasMimeType(ext, mimeType)) {
			return {};
		}

		if(!config.addFileExtensionMimeType(ext, mimeType)) {
			return {};
		}

		beginResetModel();
		endResetModel();
		return findFileExtensionMimeType(ext, mimeType);
	}


	//	bool ServerFileAssociationsModel::insertRows(int, int count, const QModelIndex & parent) {
	//		if(1 != count) {
	//			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: count of items to insert must be 1\n";
	//			return false;
	//		}

	//		auto & config = m_server->configuration();
	//		QString ext;
	//		QString mimeType;

	//		if(parent.isValid()) {
	//			// insert a new MIME type
	//			ext = parent.data().value<QString>();
	//			mimeType = QStringLiteral("application/x-subtype");

	//			if(config.fileExtensionHasMimeType(ext, mimeType)) {
	//				int idx = 1;

	//				do {
	//					++idx;
	//					ext = QStringLiteral("application/x-subtype-%1").arg(idx);
	//				} while(config.fileExtensionHasMimeType(ext, mimeType));
	//			}
	//		}
	//		else {
	//			// insert a new extension
	//			mimeType = QStringLiteral("application/octet-stream");
	//			ext = tr("newextension");

	//			if(config.fileExtensionIsRegistered(ext)) {
	//				int idx = 1;

	//				do {
	//					++idx;
	//					ext = tr("newextension%1").arg(idx);
	//				} while(config.fileExtensionIsRegistered(ext));
	//			}
	//		}

	//		// can't guarantee location of insertion so have to issue model reset
	//		beginResetModel();
	//		config.addFileExtensionMimeType(ext, mimeType);
	//		endResetModel();
	//		return true;
	//	}


	bool ServerFileAssociationsModel::removeRows(int row, int count, const QModelIndex & parent) {
		if(1 > count) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: count of items to remove must be > 0\n";
			return false;
		}

		if(parent.isValid()) {
			// remove MIME items
			const auto ext = parent.data().value<QString>();
			auto & config = m_server->configuration();
			const int mimeTypeCount = config.fileExtensionMimeTypeCount(ext);

			if(0 > row || mimeTypeCount <= row) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
				return false;
			}

			int endRow = row + count - 1;

			if(mimeTypeCount <= endRow) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
				return false;
			}

			beginRemoveRows(parent, row, endRow);
			const auto mimeTypes = config.fileExtensionMimeTypes(ext);
			auto begin = mimeTypes.cbegin() + row;

			std::for_each(begin, begin + count, [&config, &ext](const auto & mimeType) {
				config.removeFileExtensionMimeType(ext, mimeType);
			});

			endRemoveRows();
			return true;
		}

		// remove extension items
		auto & config = m_server->configuration();
		const int extensionCount = config.registeredFileExtensionCount();

		if(0 > row || extensionCount <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
			return false;
		}

		int endRow = row + count - 1;

		if(extensionCount <= endRow) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
			return false;
		}

		beginRemoveRows(parent, row, endRow);
		const auto extensions = config.registeredFileExtensions();
		auto begin = extensions.cbegin() + row;

		std::for_each(begin, begin + count, [&config](const auto & ext) {
			config.removeFileExtension(ext);
		});

		endRemoveRows();
		return true;
	}


}  // namespace Anansi
