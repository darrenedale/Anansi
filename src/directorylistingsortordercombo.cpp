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

/// \file directorylistingsortordercombo.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the DirectoryListingSortOrderCombo class.
///
/// \dep
/// - directorylistingsortordercombo.h
/// - <QVariant>
/// - <QIcon>
/// - qtmetatypes.h
/// - display_strings.h
///
/// \par Changes
/// - (2018-03) First release.

#include "directorylistingsortordercombo.h"

#include <QtGlobal>
#include <QVariant>
#include <QIcon>

#include "qtmetatypes.h"
#include "display_strings.h"


namespace Anansi {


	DirectoryListingSortOrderCombo::DirectoryListingSortOrderCombo(QWidget * parent)
	: QComboBox(parent) {
		QComboBox::addItem(QIcon::fromTheme("view-sort-ascending"), displayString(DirectoryListingSortOrder::Ascending), QVariant::fromValue(DirectoryListingSortOrder::Ascending));
		QComboBox::addItem(QIcon::fromTheme("view-sort-ascending"), displayString(DirectoryListingSortOrder::AscendingDirectoriesFirst), QVariant::fromValue(DirectoryListingSortOrder::AscendingDirectoriesFirst));
		QComboBox::addItem(QIcon::fromTheme("view-sort-ascending"), displayString(DirectoryListingSortOrder::AscendingFilesFirst), QVariant::fromValue(DirectoryListingSortOrder::AscendingFilesFirst));
		QComboBox::addItem(QIcon::fromTheme("view-sort-descending"), displayString(DirectoryListingSortOrder::Descending), QVariant::fromValue(DirectoryListingSortOrder::Descending));
		QComboBox::addItem(QIcon::fromTheme("view-sort-descending"), displayString(DirectoryListingSortOrder::DescendingDirectoriesFirst), QVariant::fromValue(DirectoryListingSortOrder::DescendingDirectoriesFirst));
		QComboBox::addItem(QIcon::fromTheme("view-sort-descending"), displayString(DirectoryListingSortOrder::DescendingFilesFirst), QVariant::fromValue(DirectoryListingSortOrder::DescendingFilesFirst));
		setToolTip(tr("<p>Choose how to sort the entries in generated directory listings.</p>"));

        // can't use qOverload() with MSVC because it doesn't implement SD-6 (feature
        // detection macros)
        connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT sortOrderChanged(sortOrder());
		});
	}


	DirectoryListingSortOrder DirectoryListingSortOrderCombo::sortOrder() {
		return currentData().value<DirectoryListingSortOrder>();
	}


	void DirectoryListingSortOrderCombo::setSortOrder(DirectoryListingSortOrder order) {
		setCurrentIndex(findData(QVariant::fromValue(order)));
	}


}  // namespace Anansi
