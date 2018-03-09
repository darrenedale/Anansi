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

/// \file serveripconnectionpolicymodel.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the ServerIpConnectionPolicyModel class.
///
/// \dep
/// - <iostream>
/// - <QIcon>
/// - types.h
/// - numerics.h
/// - server.h
/// - display_strings.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "serveripconnectionpolicymodel.h"

#include <iostream>

#include <QIcon>

#include "types.h"
#include "numerics.h"
#include "server.h"
#include "display_strings.h"
#include "qtmetatypes.h"


namespace Anansi {


	template<int ColumnIndex>
	QModelIndex ServerIpConnectionPolicyModel::findHelper(const QString & addr) const {
		const auto addresses = m_server->configuration().registeredIpAddresses();
		const auto & begin = addresses.cbegin();
		const auto & end = addresses.cend();
		const auto addrIt = std::find(begin, end, addr);

		if(end == addrIt) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, addrIt)), ColumnIndex);
	}


	ServerIpConnectionPolicyModel::ServerIpConnectionPolicyModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server to observe must not be null");
	}


	QModelIndex ServerIpConnectionPolicyModel::findIpAddress(const QString & addr) const {
		return findHelper<IpAddressColumnIndex>(addr);
	}


	QModelIndex ServerIpConnectionPolicyModel::findIpAddressPolicy(const QString & addr) const {
		return findHelper<PolicyColumnIndex>(addr);
	}


	QModelIndex ServerIpConnectionPolicyModel::index(int row, int column, const QModelIndex &) const {
		if(0 > column || Equit::max<int, IpAddressColumnIndex, PolicyColumnIndex>() < column) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid column (" << column << ")\n";
			return {};
		}

		if(0 > row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid row (" << row << ")\n";
			return {};
		}

		// for anything else, return a top-level extension item index
		if(rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: row for item index is out of bounds\n";
			return {};
		}

		return createIndex(row, column);
	}


	QModelIndex ServerIpConnectionPolicyModel::parent(const QModelIndex &) const {
		return {};
	}


	int ServerIpConnectionPolicyModel::rowCount(const QModelIndex &) const {
		return m_server->configuration().registeredIpAddressCount();
	}


	int ServerIpConnectionPolicyModel::columnCount(const QModelIndex &) const {
		return 1 + Equit::max<int, IpAddressColumnIndex, PolicyColumnIndex>();
	}


	QVariant ServerIpConnectionPolicyModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if(Qt::DisplayRole != role) {
			return QAbstractItemModel::headerData(section, orientation, role);
		}

		switch(section) {
			case IpAddressColumnIndex:
				return tr("IP address");

			case PolicyColumnIndex:
				return tr("Policy");
		}

		return {};
	}


	QVariant ServerIpConnectionPolicyModel::data(const QModelIndex & index, int role) const {
		if(Qt::DisplayRole != role && Qt::EditRole != role && Qt::DecorationRole != role) {
			return {};
		}

		if(!index.isValid()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		auto row = index.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		const auto & config = m_server->configuration();
		const auto addresses = config.registeredIpAddresses();
		const auto & addr = addresses[static_cast<std::size_t>(row)];

		switch(index.column()) {
			case IpAddressColumnIndex:
				if(Qt::DecorationRole == role) {
					return {};
				}

				return addr;
				break;

			case PolicyColumnIndex:
				switch(role) {
					case Qt::DecorationRole:
						switch(config.ipAddressConnectionPolicy(addr)) {
							case ConnectionPolicy::None:
								return {};

							case ConnectionPolicy::Reject:
								return QIcon::fromTheme("cards-block");

							case ConnectionPolicy::Accept:
								return QIcon::fromTheme("dialog-ok");
						}

					case Qt::DisplayRole:
						return displayString(config.ipAddressConnectionPolicy(addr));

					case Qt::EditRole:
						return QVariant::fromValue(config.ipAddressConnectionPolicy(addr));
				}
				break;
		}

		return {};
	}


	Qt::ItemFlags ServerIpConnectionPolicyModel::flags(const QModelIndex & index) const {
		auto ret = QAbstractItemModel::flags(index);

		if(!index.isValid()) {
			return ret;
		}

		ret |= Qt::ItemNeverHasChildren;

		if(index.column() == PolicyColumnIndex) {
			ret |= Qt::ItemIsEditable;
		}

		return ret;
	}

	bool ServerIpConnectionPolicyModel::setData(const QModelIndex & index, const QVariant & value, int role) {
		if(!index.isValid()) {
			return false;
		}

		if(Qt::EditRole != role) {
			return QAbstractItemModel::setData(index, value, role);
		}

		auto row = index.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __FILE__ << "]: invalid index - row does not exist\n";
			return false;
		}

		auto & config = m_server->configuration();

		switch(index.column()) {
			case IpAddressColumnIndex:
				std::cerr << __PRETTY_FUNCTION__ << " [" << __FILE__ << "]: can't set the IP address for a policy\n";
				return false;

			case PolicyColumnIndex: {
				const auto addr = config.registeredIpAddresses()[static_cast<std::size_t>(row)];
				const auto oldPolicy = config.ipAddressConnectionPolicy(addr);
				const auto policy = value.value<ConnectionPolicy>();

				if(policy == oldPolicy) {
					// no change
					return true;
				}

				if(!config.setIpAddressConnectionPolicy(addr, policy)) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to set policy for \"" << qPrintable(addr) << "\"\n";
					return false;
				}

				Q_EMIT policyChanged(addr, policy);
				return true;
			}
		}

		return QAbstractItemModel::setData(index, value, role);
	}


	QModelIndex ServerIpConnectionPolicyModel::addIpAddress(QString addr, ConnectionPolicy policy) {
		auto & config = m_server->configuration();

		if(addr.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: can't add policy for empty IP address\n";
			return {};
		}
		else if(config.ipAddressIsRegistered(addr)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: IP address \"" << qPrintable(addr) << "\" policy already exists\n";
			return {};
		}

		if(!config.setIpAddressConnectionPolicy(addr, policy)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to set policy for IP address \"" << qPrintable(addr) << "\"\n";
			return {};
		}

		beginResetModel();
		endResetModel();
		return findIpAddressPolicy(addr);
	}

	bool ServerIpConnectionPolicyModel::removeRows(int row, int count, const QModelIndex & parent) {
		if(1 > count) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: count of items to remove must be > 01\n";
			return false;
		}

		auto & config = m_server->configuration();
		const int addrCount = config.registeredIpAddressCount();

		if(0 > row || addrCount <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
			return false;
		}

		int endRow = row + count - 1;

		if(addrCount <= endRow) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
			return false;
		}

		beginRemoveRows(parent, row, endRow);
		const auto addrs = config.registeredIpAddresses();
		auto begin = addrs.cbegin() + row;

		std::for_each(begin, begin + count, [&config](const auto & addr) {
			config.unsetIpAddressConnectionPolicy(addr);
		});

		endRemoveRows();
		return true;
	}

}  // namespace Anansi
