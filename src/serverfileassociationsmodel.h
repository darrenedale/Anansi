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

/// \file serverfileassociationsmodel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the ServerFileAssociationsModel class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SERVERFILEASSOCIATIONSMODEL_H
#define ANANSI_SERVERFILEASSOCIATIONSMODEL_H

#include <QAbstractItemModel>
#include <QString>

namespace Anansi {

	class Server;

	class ServerFileAssociationsModel : public QAbstractItemModel {
		Q_OBJECT

	public:
		ServerFileAssociationsModel(Server * server, QObject * parent = nullptr);

		QModelIndex findFileExtension(const QString & ext) const;
		QModelIndex findMimeType(const QString & mimeType, const QModelIndex & parent) const;

		inline QModelIndex findFileExtensionMimeType(const QString & ext, const QString & mimeType) const {
			return findMimeType(mimeType, findFileExtension(ext));
		}

		QModelIndex addFileExtension(QString ext = {}, QString mimeType = {});
		QModelIndex addFileExtensionMimeType(QString ext, QString mimeType = {});

		virtual QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override;
		virtual QModelIndex parent(const QModelIndex & index) const override;
		virtual int rowCount(const QModelIndex & parent = {}) const override;
		virtual int columnCount(const QModelIndex & parent = {}) const override;
		virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
		virtual Qt::ItemFlags flags(const QModelIndex & index) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
		//		virtual bool insertRows(int row, int count, const QModelIndex & parent = {}) override;
		virtual bool removeRows(int row, int count, const QModelIndex & parent = {}) override;

	Q_SIGNALS:
		void extensionChanged(const QString & oldExt, const QString & newExt);
		void extensionMimeTypeChanged(const QString & ext, const QString & oldMime, const QString & newMime);

	private:
		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_SERVERFILEASSOCIATIONSMODEL_H
