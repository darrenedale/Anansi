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

/// \file contentencoder.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the ContentEncoder base class for Anansi.
///
/// \dep
/// - <optional>
/// - <array>
/// - <iostream>
/// - <QByteArray>
/// - <QIODevice>
/// - <QBuffer>
/// - types.h
///
/// NEXTRELEASE Review for performance.
///
/// \par Changes
/// - (2018-03) First release.


#ifndef ANANSI_CONTENTENCODER_H
#define ANANSI_CONTENTENCODER_H

#include <optional>
#include <array>
#include <iostream>

#include <QByteArray>
#include <QIODevice>
#include <QBuffer>

#include "types.h"

namespace Anansi {

	class RequestHandler;

	class ContentEncoder {
	public:
		ContentEncoder() = default;
		virtual ~ContentEncoder() = default;

		virtual HttpHeaders headers() const {
			return {};
		}

		virtual bool startEncoding(QIODevice &) {
			return true;
		}

		virtual QByteArray encode(QIODevice & dataSource, const std::optional<int> & size = {}) {
			QByteArray out;
			QBuffer outBuffer(&out);
			encodeTo(outBuffer, dataSource, size);
			return out;
		}

		virtual QByteArray encode(const QByteArray & data) {
			QByteArray out;
			QBuffer outBuffer(&out);
			encodeTo(outBuffer, data);
			return out;
		}

		virtual bool encodeTo(QIODevice &, const QByteArray & data) = 0;

		virtual bool encodeTo(QIODevice & out, QIODevice & in, const std::optional<int> & size = {}) {
			static constexpr const int64_t BufferSize = 16384;
			std::array<char, BufferSize> readBuffer;
			int64_t bytesWritten = 0;

			while(!in.atEnd() && (!size || bytesWritten < *size)) {
				const auto bytesRead = in.read(&readBuffer[0], size ? qMin(BufferSize, *size - bytesWritten) : BufferSize);

				if(-1 == bytesRead) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: error reading data to encode (\"" << qPrintable(in.errorString()) << "\")\n";
					return false;
				}

				if(!encodeTo(out, QByteArray::fromRawData(&readBuffer[0], static_cast<int>(bytesRead)))) {
					return false;
				}

				bytesWritten += bytesRead;
			}

			return !size || bytesWritten == *size;
		}

		virtual bool finishEncoding(QIODevice &) {
			return true;
		}
	};

}  // namespace Anansi

#endif  // ANANSI_CONTENTENCODER_H
