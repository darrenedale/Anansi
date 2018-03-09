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
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation file for the ZLibContentEncoder class.
///
/// \dep
/// - zlibdeflater.h
///
/// \par Changes
/// - (2018-03) First release.

#include "zlibcontentencoder.h"

namespace Anansi {

	namespace Detail {
		std::optional<int64_t> qiodeviceDeflaterRead(QIODevice & in, char * data, int64_t max) {
			auto ret = in.read(data, max);

			if(-1 == ret) {
				return {};
			}

			return ret;
		}


		std::optional<int64_t> qiodeviceDeflaterWrite(QIODevice & out, char * data, int64_t size) {
			auto ret = out.write(data, size);

			if(-1 == ret) {
				return {};
			}

			return ret;
		}


		bool qiodeviceDeflateStreamEnd(const QIODevice & in) {
			return in.atEnd();
		}
	}  // namespace Detail

}  // namespace Anansi
