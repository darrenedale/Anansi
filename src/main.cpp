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

/// \file main.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Main entry point for Anansi.
///
/// \dep application.h
///
/// NEXTRELEASE Much of the backend is a bit of a std/Qt hybrid. Some of this
/// is "necessary" (e.g. use of QProcess makes things much easier) but some is
/// just remnants from before the transition to "std-where-possible".
///
/// \par Changes
/// - (2018-03) First release.

#include "application.h"


int main(int argc, char ** argv) {
	Anansi::Application app(argc, argv);
	return app.exec();
}
