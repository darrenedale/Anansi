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

/// \file identitycontentencoder.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the IdentityContentEncoder class.
///
/// \dep
/// - identitycontentencoder.h
/// - <iostream>
/// - <QIODevice>
/// - <QByteArray>
/// - macros.h
///
/// \par Changes
/// - (2018-03) First release.

#include "identitycontentencoder.h"

#include <iostream>

#include <QIODevice>
#include <QByteArray>

#include "macros.h"


namespace Anansi {


	bool IdentityContentEncoder::encodeTo(QIODevice & out, const QByteArray & data) {
		int64_t written = 0;
		auto length = static_cast<int64_t>(data.size());
		int failCount = 0;
		const auto * buffer = data.data();

		while(3 > failCount && written < length) {
			auto thisWrite = out.write(buffer + written, length);

			if(-1 == thisWrite) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed writing to socket\n";
				++failCount;
			}
			else {
				length -= thisWrite;
				failCount = 0;
			}
		}

		return 0 == length;
	}


}  // namespace Anansi
