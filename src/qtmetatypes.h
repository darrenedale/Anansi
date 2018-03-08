/// \file qtmetatypes.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declare various Anansi types to the Qt MetaObject system.
///
/// This makes those types usable in QVariant objects and in queued signal-slot
/// connections (e.g. those that operate across thread boundaries).
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
