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

/// \file fileassociationsmodel.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the FileAssociationsModel class.
///
/// \dep
/// - fileassociationsmodel.h
/// - <iostream>
/// - <Qt>
/// - <QVector>
/// - assert.h
/// - server.h
/// - mediatypeicons.h
///
/// NEXTRELEASE API for direct-removal of extensions/media types rather than
/// using removeRows() which is cumbersome and feels error prone
///
/// \par Changes
/// - (2018-03) First release.

#include "fileassociationsmodel.h"

#include <iostream>

#include <Qt>
#include <QVector>

#include "assert.h"
#include "server.h"
#include "mediatypeicons.h"


namespace Anansi {


	FileAssociationsModel::FileAssociationsModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		eqAssert(m_server, "server to observe must not be null");
	}


	QModelIndex FileAssociationsModel::findFileExtension(const QString & ext) const {
		const auto extensions = m_server->configuration().registeredFileExtensions();
		const auto & begin = extensions.cbegin();
		const auto & end = extensions.cend();
		const auto extIt = std::find(begin, end, ext);

		if(extIt == end) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, extIt)), 0, static_cast<quintptr>(0));
	}


	QModelIndex FileAssociationsModel::findMediaType(const QString & mediaType, const QModelIndex & parent) const {
		if(!parent.isValid()) {
			return {};
		}

		if(parent.parent().isValid()) {
			// the provided parent is a media type item
			return {};
		}

		const auto mediaTypes = m_server->configuration().fileExtensionMediaTypes(parent.data().value<QString>());
		const auto & begin = mediaTypes.cbegin();
		const auto & end = mediaTypes.cend();
		const auto mediaTypeIt = std::find(begin, end, mediaType);

		if(end == mediaTypeIt) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, mediaTypeIt)), 0, static_cast<quintptr>(parent.row() + 1));
	}


	QModelIndex FileAssociationsModel::index(int row, int column, const QModelIndex & parent) const {
		if(0 != column) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid column (" << column << ")\n";
			return {};
		}

		if(0 > row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid row (" << row << ")\n";
			return {};
		}

		if(parent.isValid()) {
			if(0 == parent.internalId()) {
				// extension items have associated media types as children
				if(m_server->configuration().fileExtensionMediaTypeCount(parent.data().value<QString>()) <= row) {
					std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: row for media type item index is out of bounds\n";
					return {};
				}

				// ID is parent row + 1. this leaves ID = 0 to be used as an indicator
				// that an index is an extension item index. the ID for media type items
				// has 1 subtracted to find the row index of its parent extension item
				return createIndex(row, column, static_cast<quintptr>(parent.row() + 1));
			}

			// if parent's ID is > 0, it's a media type item, which has no children, so
			// just return and invalid index
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: parent index does not have any children\n";
			return {};
		}

		// for anything else, return a top-level extension item index
		if(rowCount() <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: row for extension item index is out of bounds\n";
			return {};
		}

		return createIndex(row, column, static_cast<quintptr>(0));
	}


	QModelIndex FileAssociationsModel::parent(const QModelIndex & idx) const {
		if(!idx.isValid()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid index == invalid parent\n";
			return {};
		}

		if(0 == idx.internalId()) {
			// this is an extension item index and as such it has no parent
			return {};
		}

		return createIndex(static_cast<int>(idx.internalId() - 1), 0, static_cast<quintptr>(0));
	}


	int FileAssociationsModel::rowCount(const QModelIndex & parent) const {
		if(parent.isValid()) {
			if(0 < parent.internalId()) {
				// media type items don't have children
				return 0;
			}

			return m_server->configuration().fileExtensionMediaTypeCount(parent.data().value<QString>());
		}

		return m_server->configuration().registeredFileExtensionCount();
	}


	int FileAssociationsModel::columnCount(const QModelIndex &) const {
		return 1;
	}


	QVariant FileAssociationsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if(Qt::DisplayRole != role) {
			return QAbstractItemModel::headerData(section, orientation, role);
		}

		if(0 == section) {
			return tr("Media type associations");
		}

		return {};
	}


	QVariant FileAssociationsModel::data(const QModelIndex & idx, int role) const {
		if(Qt::DisplayRole != role && Qt::EditRole != role && Qt::DecorationRole != role) {
			return {};
		}

		if(!idx.isValid()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		if(0 != idx.column()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: index column must be 0\n";
			return {};
		}

		if(0 == idx.internalId()) {
			if(Qt::DecorationRole == role) {
				return {};
			}

			int extIdx = idx.row();
			const auto & config = m_server->configuration();

			if(0 > extIdx || config.registeredFileExtensionCount() <= extIdx) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: extensions index row is not valid\n";
				return {};
			}

			return config.registeredFileExtensions()[static_cast<std::size_t>(extIdx)];
		}

		int extIdx = static_cast<int>(idx.internalId() - 1);
		const auto & config = m_server->configuration();

		if(0 > extIdx || config.registeredFileExtensionCount() <= extIdx) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid parent row index\n";
			return {};
		}

		const auto ext = config.registeredFileExtensions()[static_cast<std::size_t>(extIdx)];

		if(ext.isEmpty()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: empty extension when looking up media type index\n";
			return {};
		}

		int mediaTypeIdx = idx.row();

		if(0 > mediaTypeIdx || config.fileExtensionMediaTypeCount(ext) <= mediaTypeIdx) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: media type index row is not valid\n";
			return {};
		}

		const auto mediaTypes = config.fileExtensionMediaTypes(ext);

		if(Qt::DecorationRole == role) {
			return mediaTypeIcon(mediaTypes[static_cast<std::size_t>(mediaTypeIdx)]);
		}

		return mediaTypes[static_cast<std::size_t>(mediaTypeIdx)];
	}


	Qt::ItemFlags FileAssociationsModel::flags(const QModelIndex & idx) const {
		auto ret = QAbstractItemModel::flags(idx);

		if(idx.isValid()) {
			ret |= Qt::ItemIsEditable;

			if(idx.parent().isValid()) {
				// it's a media type item, which never has any children
				ret |= Qt::ItemNeverHasChildren;
			}
		}

		return ret;
	}


	bool FileAssociationsModel::setData(const QModelIndex & idx, const QVariant & value, int role) {
		if(!idx.isValid()) {
			return false;
		}

		if(Qt::EditRole != role) {
			return QAbstractItemModel::setData(idx, value, role);
		}

		const auto parent = idx.parent();

		if(parent.isValid()) {
			// it's a media type item
			// this call does all the validation necessary
			const auto ext = parent.data().value<QString>();
			const auto oldMediaType = idx.data().value<QString>();
			const auto newMediaType = value.value<QString>();

			if(oldMediaType == newMediaType) {
				return true;
			}

			auto & config = m_server->configuration();

			if(config.fileExtensionHasMediaType(ext, newMediaType)) {
				return false;
			}

			if(!config.changeFileExtensionMediaType(ext, oldMediaType, newMediaType)) {
				return false;
			}

			Q_EMIT dataChanged(idx, idx, QVector<int>() << Qt::DisplayRole << Qt::EditRole);
			Q_EMIT extensionMediaTypeChanged(ext, oldMediaType, newMediaType);
			return true;
		}

		// it's a file extension item
		// this call does all the validation necessary
		if(!m_server->configuration().changeFileExtension(idx.data().value<QString>(), value.value<QString>())) {
			return false;
		}

		const auto oldExt = idx.data().value<QString>();
		const auto newExt = value.value<QString>();

		// changing an extension causes the underlying storage map to rehash its key,
		// therefore extensions are likely to be reordered so all indexes will be
		// potentially invalidated
		beginResetModel();
		endResetModel();
		Q_EMIT extensionChanged(oldExt, newExt);
		return true;
	}


	QModelIndex FileAssociationsModel::addFileExtension(QString ext, QString mediaType) {
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

		if(mediaType.isEmpty()) {
			mediaType = QStringLiteral("application/octet-stream");
		}

		if(!config.addFileExtensionMediaType(ext, mediaType)) {
			return {};
		}

		beginResetModel();
		endResetModel();
		return findFileExtension(ext);
	}


	QModelIndex FileAssociationsModel::addFileExtensionMediaType(QString ext, QString mediaType) {
		if(ext.isEmpty()) {
			return {};
		}

		auto & config = m_server->configuration();

		if(mediaType.isEmpty()) {
			mediaType = QStringLiteral("application/x-subtype");

			if(config.fileExtensionHasMediaType(ext, mediaType)) {
				int idx = 1;

				do {
					++idx;
					ext = QStringLiteral("application/x-subtype-%1").arg(idx);
				} while(config.fileExtensionHasMediaType(ext, mediaType));
			}
		}
		else if(config.fileExtensionHasMediaType(ext, mediaType)) {
			return {};
		}

		if(!config.addFileExtensionMediaType(ext, mediaType)) {
			return {};
		}

		beginResetModel();
		endResetModel();
		return findFileExtensionMediaType(ext, mediaType);
	}


	bool FileAssociationsModel::removeRows(int row, int count, const QModelIndex & parent) {
		if(1 > count) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: count of items to remove must be > 0\n";
			return false;
		}

		if(parent.isValid()) {
			// remove media type items
			const auto ext = parent.data().value<QString>();
			auto & config = m_server->configuration();
			const int mediaTypeCount = config.fileExtensionMediaTypeCount(ext);

			if(0 > row || mediaTypeCount <= row) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
				return false;
			}

			int endRow = row + count - 1;

			if(mediaTypeCount <= endRow) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
				return false;
			}

			beginRemoveRows(parent, row, endRow);
			const auto mediaTypes = config.fileExtensionMediaTypes(ext);
			auto begin = mediaTypes.cbegin() + row;

			std::for_each(begin, begin + count, [&config, &ext](const auto & mediaType) {
				config.removeFileExtensionMediaType(ext, mediaType);
			});

			endRemoveRows();
			return true;
		}

		// remove extension items
		auto & config = m_server->configuration();
		const int extensionCount = config.registeredFileExtensionCount();

		if(0 > row || extensionCount <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
			return false;
		}

		int endRow = row + count - 1;

		if(extensionCount <= endRow) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
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


	void FileAssociationsModel::clear() {
		beginResetModel();
		m_server->configuration().clearAllFileExtensions();
		endResetModel();
	}


}  // namespace Anansi
