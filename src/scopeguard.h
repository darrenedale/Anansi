/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of the Equit library.
 *
 * The Equit library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Equit library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Equit library. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file scopeguard.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the ScopeGuard class for Equit.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQ_SCOPEGUARD_H
#define EQ_SCOPEGUARD_H

#include <functional>
#include <iostream>

namespace Equit {

	/// \class ScopeGuard
	///
	/// \brief Runs a function when the object goes out of scope.
	///
	/// This is useful for cleanup code that is common to all exit paths from a
	/// scope. Instead of having to repeat the code at all exit points, wrap it
	/// in a lambda and wrap that lambda in an object of this class (allocated on
	/// the stack). When the scope exits, this object will be destroyed and the
	/// lambda will be invoked, executing all the cleanup code.
	///
	/// Example:
	/// auto cleanup = ScopeGuard{[]( void ) {
	///     std::cout << "cleaning up\\n";
	/// }};
	template<typename FunctionType>
	class ScopeGuard final {
	public:
		/// \brief Create a guard by copying a function object. */
		ScopeGuard(const FunctionType & fn)
		: m_exitFn(fn) {
		}

		/// \brief Create a guard by moving a function object. */
		ScopeGuard(FunctionType && fn)
		: m_exitFn(std::move(fn)) {
		}

		/// \brief Destroy the guard, invoking the cleanup function.
		~ScopeGuard(void) {
			m_exitFn();
		}

		/// \brief Dismiss the guard.
		///
		/// The cleanup function is removed so that when the scope is
		/// exited no code is executed. There is no way to recover the
		/// function object once dismiss() has been called.
		inline void dismiss(void) {
			m_exitFn = [](void) {};
		}

	private:
		FunctionType m_exitFn;
	};
}  // namespace Equit

#endif  // EQ_SCOPEGUARD_H
