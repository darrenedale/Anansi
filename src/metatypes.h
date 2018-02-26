#ifndef EQUIT_METATYPES_H
#define EQUIT_METATYPES_H

#include <type_traits>

namespace Equit {

	/// \brief Compile time `false` value that is dependent on a type.
	///
	/// This helps create template specialisations for cases that are invalid
	/// (i.e. to make sure that invalid uses of templates are reported at
	/// compile-time). Using a static assert with `dependent_false_type<T>::value`
	/// is guaranteed to create a template that cannot be instantiated. This can't
	/// be done with just a static assert on `false` because the compiler will
	/// report the static_assert failure regardless of whether template is
	/// instantiated or not. `dependent_false_type<T>` always evaluates to `false`
	/// at compile time, but only if it is instantiated. This means it can be used
	/// in a static assertion that will only trigger when the (template) code
	template<typename T>
	struct dependent_false_type : std::false_type {};

	/// \brief Template alias for a binary selection function.
	///
	/// A binary selection function takes two values, compares them in some way,
	/// and returns (a const reference to) the "winner". The meaning of "winner"
	/// is defined by the function. One trivial example would be a `least` function
	/// that returns the argument with the lowest value of the two.
	template<typename T, typename U = T>
	using BinarySelector = const T & (*) (const T & lhs, const U & rhs);

}  // namespace Equit


#endif  // EQUIT_METATYPES_H
