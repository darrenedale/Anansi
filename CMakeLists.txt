cmake_minimum_required(VERSION 3.8)

# name the project
project(equitwebserver)

# include necessary packages - just Qt4 for equitwebserver
find_package(Qt5 COMPONENTS Core Gui Widgets Network Xml REQUIRED)

# define our main target - the equitwebserver executable
add_executable(equitwebserver
	src/bpEditableTreeWidget.cpp
	src/ConnectionCountLabel.cpp
	src/bpIpListWidget.cpp
	src/HostNetworkInfo.cpp
	src/main.cpp
	src/MainWindow.cpp
	src/Configuration.cpp
	src/RequestHandler.cpp
	src/ConfigurationWidget.cpp
	src/Server.cpp
)

set_target_properties(equitwebserver PROPERTIES
	AUTOMOC ON
	AUTOUIC ON
	AUTOUIC_SEARCH_PATHS "ui"
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

target_link_libraries(equitwebserver Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Xml)
