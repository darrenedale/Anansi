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

/// \file serveripconnectionpolicymodel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the ServerIpConnectionPolicyModel class for Anansi.
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

#ifndef ANANSI_SERVERIPCONNECTIONPOLICYMODEL_H
#define ANANSI_SERVERIPCONNECTIONPOLICYMODEL_H

#include <QAbstractItemModel>
#include <Qt>
#include <QModelIndex>
#include <QString>
#include <QVariant>

#include "types.h"

namespace Anansi {

	class Server;

	class ServerIpConnectionPolicyModel final : public QAbstractItemModel {
		Q_OBJECT

	public:
		static constexpr const int IpAddressColumnIndex = 0;
		static constexpr const int PolicyColumnIndex = 1;

		explicit ServerIpConnectionPolicyModel(Server * server, QObject * parent = nullptr);
		ServerIpConnectionPolicyModel(const ServerIpConnectionPolicyModel &) = delete;
		ServerIpConnectionPolicyModel(ServerIpConnectionPolicyModel &&) = delete;
		void operator=(const ServerIpConnectionPolicyModel &) = delete;
		void operator=(ServerIpConnectionPolicyModel &&) = delete;

		QModelIndex findIpAddress(const QString & addr) const;
		QModelIndex findIpAddressPolicy(const QString & addr) const;

		QModelIndex addIpAddress(QString addr, ConnectionPolicy policy);

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
		void policyChanged(const QString & addr, ConnectionPolicy policy) const;

	private:
		template<int ColumnIndex>
		QModelIndex findHelper(const QString & addr) const;

		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_SERVERIPCONNECTIONPOLICYMODEL_H
