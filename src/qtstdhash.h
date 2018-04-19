/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of the Equit library.
 *
 * The Equit library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Equit library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Equit library. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file qtstdhash.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the QtHash template struct for Equit.
///
/// \dep
/// - <functional>
/// - <QHash>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQ_QTSTDHASH_H
#define EQ_QTSTDHASH_H

#include <functional>
#include <QHash>

namespace Equit {
	template<class QtClass>
	struct QtHash {
		using result_type = std::size_t;
		using argument_type = QtClass;

		result_type operator()(const argument_type & arg) const {
			return static_cast<std::size_t>(qHash(arg));
		}
	};
}  // namespace Equit

#endif  // EQ_QTSTDHASH_H
