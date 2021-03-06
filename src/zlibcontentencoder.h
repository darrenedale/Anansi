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

/// \file zlibcontentencoder.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March, 2018
///
/// \brief Definition of the ZLibContentEncoder template class for Anansi.
///
/// \dep
/// - <cstdint>
/// - <optional>
/// - <QByteArray>
/// - <QIODevice>
/// - types.h
/// - contentencoder.h
/// - zlibdeflater.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_ZLIBCONTENTENCODER_H
#define ANANSI_ZLIBCONTENTENCODER_H

#include <cstdint>
#include <optional>

#include <QByteArray>
#include <QIODevice>

#include "types.h"
#include "contentencoder.h"
#include "zlibdeflater.h"

namespace Anansi {


	namespace Detail {
		std::optional<int64_t> qiodeviceDeflaterRead(QIODevice & in, char * data, int64_t max);
		std::optional<int64_t> qiodeviceDeflaterWrite(QIODevice & out, char * data, int64_t size);
		bool qiodeviceDeflateStreamEnd(const QIODevice & in);
	}  // namespace Detail


	using Equit::ZLibDeflaterHeaderType;
	using QtZLibDeflater = Equit::ZLibDeflater<QByteArray, QIODevice, QIODevice, Detail::qiodeviceDeflaterRead, Detail::qiodeviceDeflaterWrite, Detail::qiodeviceDeflateStreamEnd, QByteArray::size_type>;


	template<ZLibDeflaterHeaderType headerType>
	class ZLibContentEncoder : public ContentEncoder {
	public:
		ZLibContentEncoder(int compressionLevel = QtZLibDeflater::DefaultCompressionLevel)
		: ContentEncoder(),
		  m_deflater(headerType, compressionLevel) {
		}


		virtual QByteArray encode(QIODevice & in, const std::optional<int64_t> & size = {}) {
			auto ret = m_deflater.addData(in, size);

			if(!ret) {
				return {};
			}

			return *ret;
		}


		QByteArray encode(const QByteArray & data) override {
			return m_deflater.addData(data);
		}


		bool encodeTo(QIODevice & out, const QByteArray & data) override {
			if(data.isEmpty()) {
				return true;
			}

			if(!m_deflater.addDataTo(out, data)) {
				return false;
			}

			return true;
		}


		bool encodeTo(QIODevice & out, QIODevice & in, const std::optional<int> & size = {}) override {
			return !!m_deflater.addDataTo(out, in, size);
		}


		bool finishEncoding(QIODevice & out) override {
			return static_cast<bool>(m_deflater.finish(out));
		}


	private:
		QtZLibDeflater m_deflater;
	};

}  // namespace Anansi

#endif  // ANANSI_ZLIBCONTENTENCODER_H
