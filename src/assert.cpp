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

/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation file for Equit library assertions.
///
/// \dep
/// - assert.h
/// - <iostream>
///
/// \par Changes
/// - (2018-03) First release.

#include "assert.h"

#include <iostream>

namespace Equit::Detail {

	void assertionFailure(const char * const expression, const char * const file, const char * const func, int line, const std::string & msg) {
		std::cerr << "assertion \"" << expression << "\" failed in " << func << "() [" << file << ":" << line << "]";

		if(!msg.empty()) {
			std::cerr << ' ' << msg;
		}

		std::cerr << '\n';
		abort();
	}

}  // namespace Equit::Detail
