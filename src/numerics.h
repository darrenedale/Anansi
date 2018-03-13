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

/// \file numerics.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Numeric (template) functions.
///
/// \dep
/// - <algorithm>
/// - <numeric>
/// - <array>
/// - metatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQ_NUMERICS_H
#define EQ_NUMERICS_H

#include <algorithm>
#include <numeric>
#include <array>

#include "metatypes.h"


namespace Equit {


	/// \brief Compile-time selection of one of a set of constant values.
	///
	/// We use both X1 and variadic Xs to force at least one value in the template
	/// instantiation. There is a specialisation for the case where no values are
	/// provided, which emits a descriptive compiler error.
	template<typename T, BinarySelector<T> Selector, const T X1, const T... Xs>
	constexpr T select() {
		constexpr const std::array<T, sizeof...(Xs)> values{Xs...};
		T ret = X1;

		for(const T & value : values) {
			ret = Selector(value, ret);
		}

		return ret;
	}


	/// \brief Compile-time computation of the max of a set of numeric constants.
	///
	/// While this is intended for use with numeric constants, it can technically
	/// be used with values of any type for which `operator>` is defined.
	///
	/// We use both X1 and variadic Xs to force at least one value in the template
	/// instantiation. There is a specialisation for the case where no values are
	/// provided, which emits a descriptive compiler error.
	template<typename T, const T X1, const T... Xs>
	constexpr T max() {
		constexpr const std::array<T, 1 + sizeof...(Xs)> values{X1, Xs...};
		return *std::max_element(values.cbegin(), values.cend());
	}


	/// \brief Compile-time computation of the min of a set of numeric constants.
	///
	/// While this is intended for use with numeric constants, it can technically
	/// be used with values of any type for which `operator<` is defined.
	///
	/// We use both X1 and variadic Xs to force at least one value in the template
	/// instantiation. There is a specialisation for the case where no values are
	/// provided, which emits a descriptive compiler error.
	template<typename T, const T X1, const T... Xs>
	constexpr T min() {
		constexpr const std::array<T, 1 + sizeof...(Xs)> values{X1, Xs...};
		return *std::min_element(values.cbegin(), values.cend());
	}


	/// \brief Specialisation of select() to fail at compile-time if given no values.
	///
	/// The compile-time failure uses a static_assert to present a useful error
	/// message, rather than just stating that the template can't be instantiated.
	/// The downside is that in mose cases the error will be reported here rather
	/// than at the site that attempted the instantiation (though most compilers
	/// will also report this in a secondary message that most IDEs should pick up.
	template<typename T, BinarySelector<T> Selector>
	constexpr T select() {
		static_assert(dependent_false_type<T>::value, "can't instantiate Equit::select<>() template with no values");
	}


	/// \brief Specialisation of max() to fail at compile-time if given no values.
	///
	/// The compile-time failure uses a static_assert to present a useful error
	/// message, rather than just stating that the template can't be instantiated.
	/// The downside is that in mose cases the error will be reported here rather
	/// than at the site that attempted the instantiation (though most compilers
	/// will also report this in a secondary message that most IDEs should pick up.
	template<typename T>
	constexpr T max() {
		static_assert(dependent_false_type<T>::value, "can't instantiate Equit::max<>() template with no values");
	}


	/// \brief Specialisation of min() to fail at compile-time if given no values.
	///
	/// The compile-time failure uses a static_assert to present a useful error
	/// message, rather than just stating that the template can't be instantiated.
	/// The downside is that in mose cases the error will be reported here rather
	/// than at the site that attempted the instantiation (though most compilers
	/// will also report this in a secondary message that most IDEs should pick up.
	template<typename T>
	constexpr T min() {
		static_assert(dependent_false_type<T>::value, "can't instantiate Equit::min<>() template with no values");
	}


}  // namespace Equit

#endif  // EQ_NUMERICS_H
