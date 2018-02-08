/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of the Équit library.
 *
 * The Équit library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Équit library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Équit library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EQ_SCOPEGUARD_H
#define EQ_SCOPEGUARD_H

/**
 * \file scopeguard.h
 *
 * \brief Defines the ScopeGuard class.
 */

#include <functional>
#include <iostream>

namespace Equit {

	/**
	 * \class ScopeGuard
	 *
	 * \brief Runs a function when the object goes out of scope.
	 *
	 * This is useful for cleanup code that is common to all exit paths from a
	 * scope. Instead of having to repeat the code at all exit points, wrap it
	 * in a lambda and wrap that lambda in an object of this class (allocated on
	 * the stack). When the scope exits, this object will be destroyed and the
	 * lambda will be invoked, executing all the cleanup code.
	 *
	 * Example:
	 * auto cleanup = ScopeGuard{[]( void ) {
	 *     std::cout << "cleaning up\\n";
	 * }};
	 */
	class ScopeGuard final {
		public:
			using Function = std::function<void(void)>;

			/** \brief Create a guard by copying a function object. */
			ScopeGuard( const Function & fn )
			: m_exitFn(fn) {
			}

			/** \brief Create a guard by moving a function object. */
			ScopeGuard( Function && fn )
			: m_exitFn(fn) {
			}

			/** \brief Destroy the guard, invoking the cleanup function. */
			~ScopeGuard( void ) {
				std::cerr << "invoking scope guard function" << std::endl;
				m_exitFn();
			}

			/**
			 * \brief Dismiss the guard.
			 *
			 * The cleanup function is removed so that when the scope is
			 * exited no code is executed. There is no way to recover the
			 * function object once dismiss() has been called.
			 */
			inline void dismiss( void ) {
				m_exitFn = []( void ) {};
			}

		private:
			Function m_exitFn;
	};
}

#endif // EQ_SCOPEGUARD_H
