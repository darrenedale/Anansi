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

/// \file zlibcontentencoder.cpp
/// \author Darren Edale
/// \version 1.0.0
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


	/// \class ZLibContentEncoder
	/// \brief Template base class for content encoders using zlib compression.
	///
	/// This is a template base class for content encoders that use zlib (via the
	/// Deflater class) to compress (deflate) content for transfer to the user agent.
	/// It is templated on the type of header that the Deflater class will use.
	/// Instantiating with HeaderType::Deflate creates a content encoder suitable for
	/// use with the "deflate" content encoding; instantiating with HeaderType::Gzip
	/// creates a content encoder suitable for use with the "gzip" content encoding.
	///
	/// It is strongly recommended that this template is not instantiated directly;
	/// rather, it should be used via an inheriting class that reimplements the
	/// headers() method to provide the appropriate headers for the response to the
	/// user agent. The two template instantiations DeflateContentEncoder and
	/// GzipContentEncoder do this.


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
