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

/// \file ipconnectionpolicymodel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the IpConnectionPolicyModel class for Anansi.
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

#ifndef ANANSI_IPCONNECTIONPOLICYMODEL_H
#define ANANSI_IPCONNECTIONPOLICYMODEL_H

#include <QAbstractItemModel>
#include <Qt>
#include <QModelIndex>
#include <QString>
#include <QVariant>

#include "types.h"

namespace Anansi {

	class Server;

	class IpConnectionPolicyModel final : public QAbstractItemModel {
		Q_OBJECT

	public:
		static constexpr const int IpAddressColumnIndex = 0;
		static constexpr const int PolicyColumnIndex = 1;

		explicit IpConnectionPolicyModel(Server * server, QObject * parent = nullptr);
		IpConnectionPolicyModel(const IpConnectionPolicyModel &) = delete;
		IpConnectionPolicyModel(IpConnectionPolicyModel &&) = delete;
		void operator=(const IpConnectionPolicyModel &) = delete;
		void operator=(IpConnectionPolicyModel &&) = delete;

		QModelIndex findIpAddress(const QString & addr) const;
		QModelIndex findIpAddressPolicy(const QString & addr) const;

		QModelIndex addIpAddress(const QString & addr, ConnectionPolicy policy);

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
		void policyChanged(const QString & addr, ConnectionPolicy policy) const;

	private:
		template<int ColumnIndex>
		QModelIndex findHelper(const QString & addr) const;

		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_IPCONNECTIONPOLICYMODEL_H
