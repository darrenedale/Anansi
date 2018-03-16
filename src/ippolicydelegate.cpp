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
/// - ippolicydelegate.h
/// - <Qt>
/// - <QModelIndex>
/// - <QVariant>
/// - assert.h
/// - types.h
/// - ipconnectionpolicymodel.h
/// - accesscontrolwidget.h
/// - connectionpolicycombo.h
///
/// \par Changes
/// - (2018-03) First release.

#include "ippolicydelegate.h"

#include <Qt>
#include <QModelIndex>
#include <QVariant>

#include "assert.h"
#include "types.h"
#include "qtmetatypes.h"
#include "ipconnectionpolicymodel.h"
#include "accesscontrolwidget.h"
#include "connectionpolicycombo.h"


namespace Anansi {


	IpPolicyDelegate::IpPolicyDelegate(AccessControlWidget * parent)
	: QStyledItemDelegate(parent) {
	}


	QWidget * IpPolicyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & idx) const {
		if(!idx.isValid()) {
			return nullptr;
		}

		switch(idx.column()) {
			case IpConnectionPolicyModel::IpAddressColumnIndex:
				return nullptr;

			case IpConnectionPolicyModel::PolicyColumnIndex:
				return new ConnectionPolicyCombo(parent);
		}

		return nullptr;
	}


	void IpPolicyDelegate::setEditorData(QWidget * editor, const QModelIndex & idx) const {
		if(!idx.isValid()) {
			return;
		}

		if(idx.column() == IpConnectionPolicyModel::PolicyColumnIndex) {
			auto * combo = qobject_cast<ConnectionPolicyCombo *>(editor);
			eqAssert(combo, "expected editor to be a ConnectionPolicyCombo (it's a " << qPrintable(editor->metaObject()->className()) << ")");
			combo->setConnectionPolicy(idx.data(Qt::EditRole).value<ConnectionPolicy>());
			return;
		}

		QStyledItemDelegate::setEditorData(editor, idx);
	}


	void IpPolicyDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & idx) const {
		if(!idx.isValid()) {
			return;
		}

		if(idx.column() == IpConnectionPolicyModel::PolicyColumnIndex) {
			auto * combo = qobject_cast<ConnectionPolicyCombo *>(editor);
			eqAssert(combo, "expected editor to be a ConnectionPolicyCombo (it's a " << qPrintable(editor->metaObject()->className()) << ")");
			model->setData(idx, QVariant::fromValue(combo->connectionPolicy()));
		}
	}


}  // namespace Anansi
