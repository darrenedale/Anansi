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

/// \file ippolicydelegate.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the IpPolicyDelegate class.
///
/// \dep
/// - types.h
/// - serveripconnectionpolicymodel.h
/// - accesscontrolwidget.h
/// - connectionpolicycombo.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "ippolicydelegate.h"

#include "types.h"
#include "serveripconnectionpolicymodel.h"
#include "accesscontrolwidget.h"
#include "connectionpolicycombo.h"
#include "qtmetatypes.h"


namespace Anansi {


	IpPolicyDelegate::IpPolicyDelegate(AccessControlWidget * parent)
	: QStyledItemDelegate(parent),
	  m_parent(parent) {
	}

	QWidget * IpPolicyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const {
		if(!index.isValid()) {
			return nullptr;
		}

		switch(index.column()) {
			case ServerIpConnectionPolicyModel::IpAddressColumnIndex:
				return nullptr;

			case ServerIpConnectionPolicyModel::PolicyColumnIndex:
				return new ConnectionPolicyCombo(parent);
		}

		return nullptr;
	}


	void IpPolicyDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		if(index.column() == ServerIpConnectionPolicyModel::PolicyColumnIndex) {
			auto * combo = qobject_cast<ConnectionPolicyCombo *>(editor);
			Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
			combo->setConnectionPolicy(index.data(Qt::EditRole).value<ConnectionPolicy>());
			return;
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	void IpPolicyDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		if(index.column() == ServerIpConnectionPolicyModel::PolicyColumnIndex) {
			auto * combo = qobject_cast<ConnectionPolicyCombo *>(editor);
			Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
			model->setData(index, QVariant::fromValue(combo->connectionPolicy()));
		}
	}


	IpPolicyDelegate::~IpPolicyDelegate() = default;


}  // namespace Anansi
