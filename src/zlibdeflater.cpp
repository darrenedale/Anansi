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
/// \brief Implementation file for the ZLibDeflater class.
///
/// \dep
/// - zlibdeflater.h
/// - assert.h
///
/// \par Changes
/// - (2018-03) First release.

#include "zlibdeflater.h"
#include "eqassert.h"


namespace Equit::Detail {


	std::optional<int64_t> stdioRead(std::istream & in, char * data, int64_t max) {
		if(!in.read(data, max)) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to read from input stream\n";
			return {};
		}

		return in.gcount();
	}


	std::optional<int64_t> stdioWrite(std::ostream & out, const char * data, int64_t size) {
		eqAssert(0 <= size, "invalid write size (received " << size << ", expected >= 0)");

		if(0 == size) {
			return 0;
		}

		const auto pos = out.tellp();

		if(!out.write(data, size)) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to write to output stream\n";
			return {};
		}

		return out.tellp() - pos;
	}


	bool stdioEof(std::istream & in) {
		return in.eof();
	}


}  // namespace Equit::Detail
