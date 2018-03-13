#ifndef EQ_ASSERT_H
#define EQ_ASSERT_H

#ifndef NDEBUG

#include <string>
#include <sstream>

namespace Equit::Detail {


	struct AssertionMessageFormatter {
		operator std::string() {
			return m_stream.str();
		}

		template<typename T>
		AssertionMessageFormatter & operator<<(const T & value) {
			m_stream << value;
			return *this;
		}

	private:
		std::ostringstream m_stream;
	};


	void assertionFailure(const char * const expression, const char * const file, const char * const func, int line, const std::string & msg = {});


}  // namespace Equit::Detail

#define eqAssert(EXPRESSION, MESSAGE) ((EXPRESSION) ? ((void) 0) : Equit::Detail::assertionFailure(#EXPRESSION, __FILE__, __PRETTY_FUNCTION__, __LINE__, Equit::Detail::AssertionMessageFormatter() << MESSAGE))
#define eqAssertNoMsg(EXPRESSION) eqAssert(EXPRESSION, (static_cast<const char * const>(nullptr)))

#else

#define eqAssertNoMsg(EXPRESSION) ((void) 0)
#define eqAssert(EXPRESSION, MESSAGE) ((void) 0)

#endif

#endif  // EQ_ASSERT_H
