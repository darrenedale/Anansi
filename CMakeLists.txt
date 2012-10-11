cmake_minimum_required(VERSION 2.6)

# name the project
project(equitwebserver)

# include necessary packages - just Qt4 for equitwebserver
find_package(Qt4 REQUIRED)

# set the parts of QT we want to use Core and GUI are included by default
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTXML TRUE)

# initialise qt for use with cmake
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

# set the project source files and header files
set(equitwebserver_sources bpEditableTreeWidget.cpp ConnectionCountLabel.cpp bpIpListWidget.cpp HostNetworkInfo.cpp main.cpp MainWindow.cpp Configuration.cpp RequestHandler.cpp ConfigurationWidget.cpp Server.cpp)
set(equitwebserver_headers bpEditableTreeWidget.h ConnectionCountLabel.h bpIpListWidget.h HostNetworkInfo.h MainWindow.h RequestHandler.h Configuration.h RequestHandlerResponseCodes.h ConfigurationWidget.h Server.h)
set(equitwebserver_resources mimeicons.qrc resources.qrc)

# run moc on all headers
qt4_wrap_cpp(equitwebserver_mocs ${equitwebserver_headers})
qt4_add_resources(equitwebserver_rccs ${equitwebserver_resources})

# define our main target - the equitwebserver executable
add_executable(equitwebserver ${equitwebserver_sources} ${equitwebserver_mocs} ${equitwebserver_rccs})
target_link_libraries(equitwebserver ${QT_LIBRARIES})

