/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Equit.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Qonvince. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EQUIT_QTSTDHASH_H
#define EQUIT_QTSTDHASH_H

#include <functional>
#include <QString>

namespace Equit {
	template<class QtClass>
	struct QtHash {
		typedef std::size_t result_type;
		typedef QtClass argument_type;

		result_type operator()(const argument_type & arg) const {
			return static_cast<std::size_t>(qHash(arg));
		}
	};
}  // namespace Equit

#endif  // EQUIT_QTSTDHASH_H
