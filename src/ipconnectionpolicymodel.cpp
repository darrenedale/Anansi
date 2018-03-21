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

/// \file ipconnectionpolicymodel.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the IpConnectionPolicyModel class.
///
/// \dep
/// - ipconnectionpolicymodel.h
/// - <iostream>
/// - <QIcon>
/// - assert.h
/// - types.h
/// - qtmetatypes.h
/// - numerics.h
/// - server.h
/// - display_strings.h
///
/// \par Changes
/// - (2018-03) First release.

#include "ipconnectionpolicymodel.h"

#include <iostream>

#include <QIcon>

#include "assert.h"
#include "types.h"
#include "qtmetatypes.h"
#include "numerics.h"
#include "server.h"
#include "display_strings.h"


namespace Anansi {


	using Equit::max;


	template<int ColumnIndex>
	QModelIndex IpConnectionPolicyModel::findHelper(const QString & addr) const {
		const auto addresses = m_server->configuration().registeredIpAddresses();
		const auto & begin = addresses.cbegin();
		const auto & end = addresses.cend();
		const auto addrIt = std::find(begin, end, addr);

		if(end == addrIt) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, addrIt)), ColumnIndex);
	}


	IpConnectionPolicyModel::IpConnectionPolicyModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		eqAssert(m_server, "server to observe must not be null");
	}


	QModelIndex IpConnectionPolicyModel::findIpAddress(const QString & addr) const {
		return findHelper<IpAddressColumnIndex>(addr);
	}


	QModelIndex IpConnectionPolicyModel::findIpAddressPolicy(const QString & addr) const {
		return findHelper<PolicyColumnIndex>(addr);
	}


	QModelIndex IpConnectionPolicyModel::index(int row, int column, const QModelIndex &) const {
		if(0 > column || max<int, IpAddressColumnIndex, PolicyColumnIndex>() < column) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid column (" << column << ")\n";
			return {};
		}

		if(0 > row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid row (" << row << ")\n";
			return {};
		}

		if(rowCount() <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: row (" << row << ") for item index is out of bounds\n";
			return {};
		}

		return createIndex(row, column);
	}


	QModelIndex IpConnectionPolicyModel::parent(const QModelIndex &) const {
		return {};
	}


	int IpConnectionPolicyModel::rowCount(const QModelIndex &) const {
		return m_server->configuration().registeredIpAddressCount();
	}


	int IpConnectionPolicyModel::columnCount(const QModelIndex &) const {
		return 1 + max<int, IpAddressColumnIndex, PolicyColumnIndex>();
	}


	QVariant IpConnectionPolicyModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


	QVariant IpConnectionPolicyModel::data(const QModelIndex & idx, int role) const {
		if(Qt::DisplayRole != role && Qt::EditRole != role && Qt::DecorationRole != role) {
			return {};
		}

		if(!idx.isValid()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		auto row = idx.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		const auto & config = m_server->configuration();
		const auto addresses = config.registeredIpAddresses();
		const auto & addr = addresses[static_cast<std::size_t>(row)];

		switch(idx.column()) {
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
								return QIcon(QStringLiteral(":/icons/connectionpolicies/reject"));

							case ConnectionPolicy::Reject:
								return QIcon::fromTheme(QStringLiteral("cards-block"), QIcon(QStringLiteral(":/icons/connectionpolicies/reject")));

							case ConnectionPolicy::Accept:
								return QIcon::fromTheme(QStringLiteral("dialog-ok-accept"), QIcon(QStringLiteral(":/icons/connectionpolicies/accept")));
						}
						break;

					case Qt::DisplayRole:
						return displayString(config.ipAddressConnectionPolicy(addr));

					case Qt::EditRole:
						return QVariant::fromValue(config.ipAddressConnectionPolicy(addr));
				}
				break;
		}

		return {};
	}


	Qt::ItemFlags IpConnectionPolicyModel::flags(const QModelIndex & idx) const {
		auto ret = QAbstractItemModel::flags(idx);

		if(!idx.isValid()) {
			return ret;
		}

		ret |= Qt::ItemNeverHasChildren;

		if(idx.column() == PolicyColumnIndex) {
			ret |= Qt::ItemIsEditable;
		}

		return ret;
	}


	bool IpConnectionPolicyModel::setData(const QModelIndex & idx, const QVariant & value, int role) {
		if(!idx.isValid()) {
			return false;
		}

		if(Qt::EditRole != role) {
			return QAbstractItemModel::setData(idx, value, role);
		}

		auto row = idx.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __FILE__ << "]: invalid index - row " << row << " does not exist\n";
			return false;
		}

		auto & config = m_server->configuration();

		switch(idx.column()) {
			case IpAddressColumnIndex:
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __FILE__ << "]: can't change the IP address for a policy\n";
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
					std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to set policy " << enumeratorString<std::string>(policy) << " for \"" << qPrintable(addr) << "\"\n";
					return false;
				}

				Q_EMIT policyChanged(addr, policy);
				return true;
			}
		}

		return QAbstractItemModel::setData(idx, value, role);
	}


	QModelIndex IpConnectionPolicyModel::addIpAddress(QString addr, ConnectionPolicy policy) {
		auto & config = m_server->configuration();

		if(addr.isEmpty()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: can't add policy for empty IP address\n";
			return {};
		}
		else if(config.ipAddressIsRegistered(addr)) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: IP address \"" << qPrintable(addr) << "\" policy already exists\n";
			return {};
		}

		if(!config.setIpAddressConnectionPolicy(addr, policy)) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to set policy " << enumeratorString<std::string>(policy) << " for IP address \"" << qPrintable(addr) << "\"\n";
			return {};
		}

		beginResetModel();
		endResetModel();
		return findIpAddressPolicy(addr);
	}


	bool IpConnectionPolicyModel::removeRows(int row, int count, const QModelIndex & parent) {
		if(1 > count) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: count of items to remove must be > 01\n";
			return false;
		}

		auto & config = m_server->configuration();
		const int addrCount = config.registeredIpAddressCount();

		if(0 > row || addrCount <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
			return false;
		}

		int endRow = row + count - 1;

		if(addrCount <= endRow) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
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
