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

/// \file mediatypeactionsmodel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the MediaTypeActionsModel class for Anansi.
///
/// \dep
/// - <QAbstractItemModel>
/// - <Qt>
/// - <QModelIndex>
/// - <QString>
/// - <QVariant>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MEDIATYPEACTIONSMODEL_H
#define ANANSI_MEDIATYPEACTIONSMODEL_H

#include <QAbstractItemModel>
#include <Qt>
#include <QModelIndex>
#include <QString>
#include <QVariant>

#include "types.h"

namespace Anansi {

	class Server;

	class MediaTypeActionsModel final : public QAbstractItemModel {
		Q_OBJECT

	public:
		static constexpr const int MediaTypeColumnIndex = 0;
		static constexpr const int ActionColumnIndex = 1;
		static constexpr const int CgiColumnIndex = 2;

		explicit MediaTypeActionsModel(Server * server, QObject * parent = nullptr);
		MediaTypeActionsModel(const MediaTypeActionsModel &) = delete;
		MediaTypeActionsModel(MediaTypeActionsModel &&) = delete;
		void operator=(const MediaTypeActionsModel &) = delete;
		void operator=(MediaTypeActionsModel &&) = delete;

		QModelIndex findMediaType(const QString & mediaType) const;
		QModelIndex findMediaTypeAction(const QString & mediaType) const;
		QModelIndex findMediaTypeCgi(const QString & mediaType) const;

		QModelIndex addMediaType(QString mediaType, WebServerAction action, const QString & cgi = {});
		void clear();

		QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override;
		QModelIndex parent(const QModelIndex & idx) const override;
		int rowCount(const QModelIndex & parent = {}) const override;
		int columnCount(const QModelIndex & parent = {}) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		QVariant data(const QModelIndex & idx, int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex & idx) const override;
		bool setData(const QModelIndex & idx, const QVariant & value, int role) override;
		bool removeRows(int row, int count, const QModelIndex & parent) override;

	Q_SIGNALS:
		void actionChanged(const QString & mediaType, WebServerAction action) const;
		void cgiChanged(const QString & mediaType, const QString & cgi) const;

	private:
		template<int ColumnIndex>
		QModelIndex findHelper(const QString & mediaType) const;

		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_MEDIATYPEACTIONSMODEL_H
