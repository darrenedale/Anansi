cmake_minimum_required(VERSION 3.8)

# name the project
project(Anansi)

# include necessary packages - just Qt4 for anansi
find_package(Qt5 COMPONENTS Core Gui Widgets Network Xml REQUIRED)

# define our main target - the anansi executable
add_executable(anansi
        src/accesscontrolwidget.cpp
        src/accesslogtreeitem.cpp
        src/accesslogwidget.cpp
        src/application.cpp
        src/configuration.cpp
        src/configurationwidget.cpp
        src/connectionpolicycombo.cpp
        src/counterlabel.cpp
        src/deflatecontentencoder.cpp
        src/directorylistingsortordercombo.cpp
        src/deflater.cpp
        src/display_strings.cpp
        src/fileassociationsitemdelegate.cpp
        src/fileassociationswidget.cpp
        src/filenamewidget.cpp
        src/gzipcontentencoder.cpp
        src/identitycontentencoder.cpp
        src/inlinenotificationwidget.cpp
        src/iplineeditaction.cpp
        src/ippolicydelegate.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindowstatusbar.cpp
        src/mimeactionswidget.cpp
        src/mimecombo.cpp
        src/mimecombowidgetaction.cpp
        src/mimeicons.cpp
        src/mimetypeactionsdelegate.cpp
        src/requesthandler.cpp
        src/server.cpp
        src/serverdetailswidget.cpp
        src/serverfileassociationsmodel.cpp
        src/serveripconnectionpolicymodel.cpp
        src/servermimeactionsmodel.cpp
        src/webserveractioncombo.cpp
        src/window.cpp
        src/zlibcontentencoder.cpp
        src/zlibdeflater.cpp
        resources/mimeicons.qrc
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

target_link_libraries(anansi Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Xml z)
