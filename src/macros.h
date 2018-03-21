#ifndef EQUIT_MACROS_H
#define EQUIT_MACROS_H

/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Anansi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Anansi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file macros.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief C++ preprocessor macro definitions for Anansi.

#define EQ_COMPILER_UNKNOWN 0
#define EQ_COMPILER_CLANG 1
#define EQ_COMPILER_GCC 2
#define EQ_COMPILER_MSVC 3

#if defined(__clang__)

#define EQ_COMPILER EQ_COMPILER_CLANG
#define EQ_PRETTY_FUNCTION __PRETTY_FUNCTION__

#elif defined(__GNUC__)

#define EQ_COMPILER EQ_COMPILER_GCC
#define EQ_PRETTY_FUNCTION __PRETTY_FUNCTION__

#elif defined(_MSC_VER)

#define EQ_COMPILER EQ_COMPILER_MSVC
#define EQ_PRETTY_FUNCTION __FUNCSIG__

#else

#define EQ_COMPILER EQ_COMPILER_UNKNOWN
#define EQ_PRETTY_FUNCTION __func__

#endif

#endif  // EQUIT_MACROS_H
