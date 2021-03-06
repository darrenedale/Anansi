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

/// \file fileassociationsmodel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the FileAssociationsModel class for Anansi.
///
/// \dep
/// - <QAbstractItemModel>
/// - <Qt>
/// - <QModelIndex>
/// - <QString>
/// - <QVariant>
///
/// NEXTRELEASE use findHelper() (see IpConnectionPolicyModel)
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_FILEASSOCIATIONSMODEL_H
#define ANANSI_FILEASSOCIATIONSMODEL_H

#include <QAbstractItemModel>
#include <Qt>
#include <QModelIndex>
#include <QString>
#include <QVariant>

namespace Anansi {

	class Server;

	class FileAssociationsModel final : public QAbstractItemModel {
		Q_OBJECT

	public:
		explicit FileAssociationsModel(Server * server, QObject * parent = nullptr);
		FileAssociationsModel(const FileAssociationsModel &) = delete;
		FileAssociationsModel(FileAssociationsModel &&) = delete;
		void operator=(const FileAssociationsModel &) = delete;
		void operator=(FileAssociationsModel &&) = delete;

		QModelIndex findFileExtension(const QString & ext) const;
		QModelIndex findMediaType(const QString & mediaType, const QModelIndex & parent) const;

		inline QModelIndex findFileExtensionMediaType(const QString & ext, const QString & mediaType) const {
			return findMediaType(mediaType, findFileExtension(ext));
		}

		QModelIndex addFileExtension(QString ext = {}, QString mediaType = {});
		QModelIndex addFileExtensionMediaType(QString ext, QString mediaType = {});
		bool removeFileExtension(const QString & ext = {});
		bool removeFileExtensionMediaType(const QString & ext, const QString & mediaType = {});

		void clear();

		int rowCount(const QModelIndex & parent = {}) const override;
		int columnCount(const QModelIndex & parent = {}) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

		QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override;
		QModelIndex parent(const QModelIndex & idx) const override;

		QVariant data(const QModelIndex & idx, int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex & idx) const override;
		bool setData(const QModelIndex & idx, const QVariant & value, int role = Qt::EditRole) override;

	Q_SIGNALS:
		void extensionChanged(const QString & oldExt, const QString & newExt);
		void extensionMediaTypeChanged(const QString & ext, const QString & oldMediaType, const QString & newMediaType);

	private:
		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_FILEASSOCIATIONSMODEL_H
