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

/// \file connectionpolicycombo.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the ConnectionPolicyCombo class.
///
/// \dep
/// - connectionpolicycombo.h
/// - <QVariant>
/// - <QIcon>
/// - qtmetatypes.h
/// - display_strings.h
///
/// \par Changes
/// - (2018-03) First release.

#include "connectionpolicycombo.h"

#include <QVariant>
#include <QIcon>

#include "qtmetatypes.h"
#include "display_strings.h"


namespace Anansi {


	ConnectionPolicyCombo::ConnectionPolicyCombo(QWidget * parent)
	: QComboBox(parent) {
		QComboBox::addItem(QIcon(":/icons/connectionpolicies/nopolicy"), displayString(ConnectionPolicy::None), QVariant::fromValue(ConnectionPolicy::None));
		QComboBox::addItem(QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/connectionpolicies/accept")), displayString(ConnectionPolicy::Accept), QVariant::fromValue(ConnectionPolicy::Accept));
		QComboBox::addItem(QIcon::fromTheme("dialog-cancel", QIcon(":/icons/connectionpolicies/reject")), displayString(ConnectionPolicy::Reject), QVariant::fromValue(ConnectionPolicy::Reject));
		setToolTip(tr("<p>Choose the policy to use for HTTP connections from IP addresses that do not have a specific policy, including those for which <strong>No Policy</strong> has been chosen.</p>"));

        // can't use qOverload() with MSVC because it doesn't implement SD-6 (feature
        // detection macros)
        connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT connectionPolicyChanged(connectionPolicy());
		});
	}


	ConnectionPolicy ConnectionPolicyCombo::connectionPolicy() {
		return currentData().value<ConnectionPolicy>();
	}


	void ConnectionPolicyCombo::setConnectionPolicy(ConnectionPolicy policy) {
		setCurrentIndex(findData(QVariant::fromValue(policy)));
	}


}  // namespace Anansi
