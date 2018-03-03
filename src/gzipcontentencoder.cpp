/// \file deflatecontentencoder.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Definition of the DeflateContentEncoder class for Equit.
///
/// \todo Producing invalid gzip streams. I think it's CRC32-related
///
/// \par Changes
/// - (2018-02) First release.

#include "gzipcontentencoder.h"

#include <QIODevice>
#include <QByteArray>
#include <QtEndian>


namespace EquitWebServer {


	static constexpr const char * GzipHeader = "\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03";


	GzipContentEncoder::GzipContentEncoder(int compressionLevel)
	: DeflateContentEncoder(compressionLevel),
	  m_uncompressedSize(0) {
	}


	HttpHeaders GzipContentEncoder::headers() const {
		return {HttpHeaders::value_type{"content-encoding", "gzip"}};
	}


	bool GzipContentEncoder::startEncoding(QIODevice & out) {
		out.write(GzipHeader, 10);
		m_crc32.reset();
		m_uncompressedSize = 0;
		return true;
	}


	bool GzipContentEncoder::encodeTo(QIODevice & out, const QByteArray & data) {
		if(data.isEmpty()) {
			return true;
		}

		m_crc32.addData(data);
		m_uncompressedSize += static_cast<unsigned int>(data.size());
		return DeflateContentEncoder::encodeTo(out, data);
	}


	bool GzipContentEncoder::finishEncoding(QIODevice & out) {
		DeflateContentEncoder::finishEncoding(out);
		uint32_t crc32 = qToLittleEndian(m_crc32.intResult());
		uint32_t iSize = static_cast<uint32_t>(0xffffffff & qToLittleEndian(m_uncompressedSize));
		out.write(reinterpret_cast<const char *>(&crc32), 4);
		out.write(reinterpret_cast<const char *>(&iSize), 4);
		std::cout << std::flush;
		return true;
	}


}  // namespace EquitWebServer
