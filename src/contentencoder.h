#ifndef ANANSI_CONTENTENCODER_H
#define ANANSI_CONTENTENCODER_H

#include <optional>
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
