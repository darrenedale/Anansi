/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of EquitWebServer.
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
 * along with EquitWebServer. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file identitycontentencoder.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the IdentityContentEncoder class.
///
/// \dep
/// - <iostream>
/// - <QIODevice>
/// - <QByteArray>
///
/// \par Changes
/// - (2018-03) First release.

#include "identitycontentencoder.h"

#include <iostream>

#include <QIODevice>
#include <QByteArray>


namespace EquitWebServer {


	bool IdentityContentEncoder::encodeTo(QIODevice & out, const QByteArray & data) {
		uint64_t written = 0;
		uint64_t length = static_cast<uint64_t>(data.size());
		int failCount = 0;

		while(3 > failCount && written < length) {
			auto thisWrite = out.write(data.data() + written, static_cast<int>(length));

			if(-1 == thisWrite) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to socket\n";
				++failCount;
			}
			else {
				length -= static_cast<uint64_t>(thisWrite);
				failCount = 0;
			}
		}

		return 0 == length;
	}


}  // namespace EquitWebServer
