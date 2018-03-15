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

/// \file servermimeactionsmodel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the ServerMimeActionsModel class for Anansi.
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

#ifndef ANANSI_SERVERMIMEACTIONSMODEL_H
#define ANANSI_SERVERMIMEACTIONSMODEL_H

#include <QAbstractItemModel>
#include <Qt>
#include <QModelIndex>
#include <QString>
#include <QVariant>

#include "types.h"

namespace Anansi {

	class Server;

	class ServerMimeActionsModel final : public QAbstractItemModel {
		Q_OBJECT

	public:
		static constexpr const int MimeTypeColumnIndex = 0;
		static constexpr const int ActionColumnIndex = 1;
		static constexpr const int CgiColumnIndex = 2;

		explicit ServerMimeActionsModel(Server * server, QObject * parent = nullptr);
		ServerMimeActionsModel(const ServerMimeActionsModel &) = delete;
		ServerMimeActionsModel(ServerMimeActionsModel &&) = delete;
		void operator=(const ServerMimeActionsModel &) = delete;
		void operator=(ServerMimeActionsModel &&) = delete;

		QModelIndex findMimeType(const QString & mime) const;
		QModelIndex findMimeTypeAction(const QString & mime) const;
		QModelIndex findMimeTypeCgi(const QString & mime) const;

		QModelIndex addMimeType(QString mime, WebServerAction action, const QString & cgi = {});
		void clear();

		virtual QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override;
		virtual QModelIndex parent(const QModelIndex & idx) const override;
		virtual int rowCount(const QModelIndex & parent = {}) const override;
		virtual int columnCount(const QModelIndex & parent = {}) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		virtual QVariant data(const QModelIndex & idx, int role = Qt::DisplayRole) const override;
		virtual Qt::ItemFlags flags(const QModelIndex & idx) const override;
		virtual bool setData(const QModelIndex & idx, const QVariant & value, int role) override;
		virtual bool removeRows(int row, int count, const QModelIndex & parent) override;

	Q_SIGNALS:
		void actionChanged(const QString & mime, WebServerAction action) const;
		void cgiChanged(const QString & mime, const QString & cgi) const;

	private:
		template<int ColumnIndex>
		QModelIndex findHelper(const QString & mime) const;

		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_SERVERMIMEACTIONSMODEL_H
