#include "zlibdeflater.h"

namespace Equit {

	namespace Detail {
		std::optional<int64_t> stdioRead(std::istream & in, char * data, int64_t max) {
			if(!in.read(data, max)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
				return {};
			}

			return in.gcount();
		}


		std::optional<int64_t> stdioWrite(std::ostream & out, const char * data, int64_t size) {
			assert(0 <= size);

			if(0 == size) {
				return 0;
			}

			const auto pos = out.tellp();

			if(!out.write(data, size)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
				return {};
			}

			return out.tellp() - pos;
		}


		bool stdioEof(std::istream & in) {
			return in.eof();
		}
	}  // namespace Detail

}  // namespace Equit
