#ifndef EQUITWEBSERVER_CONTENTENCODER_H
#define EQUITWEBSERVER_CONTENTENCODER_H

#include <optional>

#include <QByteArray>
#include <QIODevice>
#include <QBuffer>

#include "types.h"


namespace EquitWebServer {

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
			int64_t bytesWritten = 0;

			// TODO read error checking
			while(!in.atEnd() && (!size || bytesWritten < *size)) {
				const auto & data = in.read(size ? qMin(BufferSize, *size - bytesWritten) : BufferSize);

				if(!encodeTo(out, data)) {
					return false;
				}

				bytesWritten += data.size();
			}

			return true;
		}

		virtual bool finishEncoding(QIODevice &) {
			return true;
		}

	private:
		static constexpr const int64_t BufferSize = 4096 * 1024;
	};

}  // namespace EquitWebServer

#endif  // CONTENTENCODER_H
