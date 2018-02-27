/// \file main.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February 2018
///
/// \brief Main entry point for the EquitWebServer application.
///
/// \todo decide on application license.
///
/// \par Changes
/// - (2012-06-19) file documentation created.


#include "application.h"


int main(int argc, char ** argv) {
	EquitWebServer::Application app(argc, argv);
	return app.exec();
}
