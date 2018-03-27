cmake_minimum_required(VERSION 3.8)

project(Anansi)
find_package(Qt5 COMPONENTS Core Gui Widgets Network Xml REQUIRED)

# main target - the anansi executable
add_executable(anansi
        src/accesscontrolwidget.cpp
        src/accesslogtreeitem.cpp
        src/accesslogwidget.cpp
        src/application.cpp
        src/assert.cpp
        src/configuration.cpp
        src/configurationwidget.cpp
        src/connectionpolicycombo.cpp
        src/counterlabel.cpp
        src/directorylistingsortordercombo.cpp
        src/display_strings.cpp
        src/fileassociationsitemdelegate.cpp
        src/fileassociationsmodel.cpp
        src/fileassociationswidget.cpp
        src/filesystempathwidget.cpp
        src/identitycontentencoder.cpp
        src/inlinenotificationwidget.cpp
        src/ipconnectionpolicymodel.cpp
        src/iplineeditaction.cpp
        src/ippolicydelegate.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindowstatusbar.cpp
        src/mediatypeactionsdelegate.cpp
        src/mediatypeactionsmodel.cpp
        src/mediatypeactionswidget.cpp
        src/mediatypecombo.cpp
        src/mediatypecombowidgetaction.cpp
        src/mediatypeicons.cpp
        src/requesthandler.cpp
        src/selectorpanel.cpp
        src/server.cpp
        src/serverdetailswidget.cpp
        src/startstopbutton.cpp
        src/webserveractioncombo.cpp
        src/windowbase.cpp
        src/zlibcontentencoder.cpp
        src/zlibdeflater.cpp

        resources/mediatypeicons.qrc
        resources/resources.qrc
        resources/stylesheets.qrc
)

set_target_properties(anansi PROPERTIES
	AUTOMOC ON
	AUTOUIC ON
	AUTORCC ON
	AUTOUIC_SEARCH_PATHS "ui"
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries(anansi Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Xml)

if(MSVC)
	target_link_libraries(anansi zlibwapi)
else()
	target_link_libraries(anansi z)
endif()
