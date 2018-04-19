
/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of the Equit library.
 *
 * The Equit library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The Equit library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the Equit library. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file eqassert.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Assertion support for the Equit library.

#ifndef EQ_ASSERT_H
#define EQ_ASSERT_H

#ifndef NDEBUG

#include <string>
#include <sstream>

#include "macros.h"


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

#define eqAssert(EXPRESSION, MESSAGE) ((EXPRESSION) ? ((void) 0) : Equit::Detail::assertionFailure(#EXPRESSION, __FILE__, EQ_PRETTY_FUNCTION, __LINE__, Equit::Detail::AssertionMessageFormatter() << MESSAGE))
#define eqAssertNoMsg(EXPRESSION) eqAssert(EXPRESSION, (static_cast<const char * const>(nullptr)))

#else

#define eqAssertNoMsg(EXPRESSION) ((void) 0)
#define eqAssert(EXPRESSION, MESSAGE) ((void) 0)

#endif

#endif  // EQ_ASSERT_H
