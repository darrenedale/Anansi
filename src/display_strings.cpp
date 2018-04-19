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

/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation file for display string functions for Anansi.
///
/// \dep
/// - display_strings.h
/// - <QApplication>
/// - assert.h
///
/// \par Changes
/// - (2018-03) First release.

#include "display_strings.h"

#include <QApplication>

#include "eqassert.h"


namespace Anansi {


	QString displayString(ConnectionPolicy policy) {
		switch(policy) {
			case ConnectionPolicy::None:
				return QApplication::tr("No Policy");

			case ConnectionPolicy::Accept:
				return QApplication::tr("Accept Connection");

			case ConnectionPolicy::Reject:
				return QApplication::tr("Reject Connection");
		}

		eqAssert(false, "unhandled enumerator value " << static_cast<int>(policy));
		return {};
	}


	QString displayString(WebServerAction action) {
		switch(action) {
			case WebServerAction::Serve:
				return QApplication::tr("Serve");

			case WebServerAction::CGI:
				return QApplication::tr("CGI");

			case WebServerAction::Forbid:
				return QApplication::tr("Forbid");

			case WebServerAction::Ignore:
				return QApplication::tr("Ignore");
		}

		eqAssert(false, "unhandled enumerator value " << static_cast<int>(action));
		return {};
	}


	QString displayString(DirectoryListingSortOrder sortOrder) {
		switch(sortOrder) {
			case DirectoryListingSortOrder::Ascending:
				return QApplication::tr("Ascending");

			case DirectoryListingSortOrder::AscendingDirectoriesFirst:
				return QApplication::tr("Ascending, directories first");

			case DirectoryListingSortOrder::AscendingFilesFirst:
				return QApplication::tr("Ascending, files first");

			case DirectoryListingSortOrder::Descending:
				return QApplication::tr("Descending");

			case DirectoryListingSortOrder::DescendingDirectoriesFirst:
				return QApplication::tr("Descending, directories first");

			case DirectoryListingSortOrder::DescendingFilesFirst:
				return QApplication::tr("Descending, files first");
		}

		eqAssert(false, "unhandled enumerator value " << static_cast<int>(sortOrder));
		return {};
	}


}  // namespace Anansi
