/*  Copyright (C) 2016 Darren Edale <darren@equituk.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// \file crc32.h
/// \author Darren Edale
/// \date 23rd June, 2012
/// \version 1.0.0
///
/// \brief Declaration of the Crc32 class.

#ifndef EQUIT_CRC32_H
#define EQUIT_CRC32_H

#include <QByteArray>

class QIODevice;

namespace Equit {
	/// \class Crc32
	/// \author Darren Edale
	/// \date 23rd June, 2012
	/// \version 1.0.0
	///
	/// \brief An implementation of the Crc32 checksum algorithm.
	class Crc32 final {
	public:
		inline Crc32() {
			reset();
		}

		static uint32_t intChecksum(const char * data, int len);

		inline static quint32 intChecksum(const QByteArray & data) {
			return intChecksum(data.data(), data.size());
		}

		inline static QByteArray checksum(const char * data, int len) {
			auto ret = intChecksum(data, len);
			return QByteArray(reinterpret_cast<const char *>(&ret), 4);
		}

		inline static QByteArray checksum(const QByteArray & data) {
			return checksum(data.data(), data.size());
		}

		inline void reset() {
			m_checksum = 0;
		}

		bool addData(const char * data, int len);
		bool addData(QIODevice & device);

		inline bool addData(const QByteArray & data) {
			return addData(data.data(), data.size());
		}

		inline uint32_t intResult() const {
			return m_checksum;
		}

		inline QByteArray result() const {
			return {reinterpret_cast<const char *>(&m_checksum), 4};
		}

	private:
		uint32_t m_checksum;
	};
}  // namespace Equit

#endif  // CRC32_H
