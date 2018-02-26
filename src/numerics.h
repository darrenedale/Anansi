#ifndef EQUIT_NUMERICS_H
#define EQUIT_NUMERICS_H

#include <algorithm>
#include <numeric>
#include <array>

#include "metatypes.h"

namespace Equit {


	/// \brief Template alias for a binary selection function.
	///
	/// A binary selection function takes two values, compares them in some way,
	/// and returns (a const reference to) the "winner". The meaning of "winner"
	/// is defined by the function. One trivial example would be a `least` function
	/// that returns the argument with the lowest value of the two.
	///
	/// \todo this belongs in a different header file since it applies more
	/// generally than just the templates defined here.
	template<typename T, typename U = T>
	using BinarySelector = const T & (*) (const T & lhs, const U & rhs);


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

#endif  // EQUIT_NUMERICS_H
