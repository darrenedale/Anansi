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

/// \file qtmetatypes.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declare various Anansi types to the Qt MetaObject system.
///
/// This makes those types usable in QVariant objects and in queued signal-slot
/// connections (e.g. those that operate across thread boundaries).
///
/// \dep
/// - <QMetaType>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_QTMETATYPES_H
#define ANANSI_QTMETATYPES_H

#include <QMetaType>

#include "types.h"

Q_DECLARE_METATYPE(Anansi::ConnectionPolicy)
Q_DECLARE_METATYPE(Anansi::WebServerAction)
Q_DECLARE_METATYPE(Anansi::DirectoryListingSortOrder)

#endif  // ANANSI_QTMETATYPES_H
