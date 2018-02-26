import qbs

CppApplication {
	name: "EquitWebServer"
	targetName: "equitwebserver"
    cpp.cxxLanguageVersion: "c++17"
	cpp.enableRtti: false
    cpp.includePaths: ["."]

    Group {
		condition: qbs.buildVariant.contains("release")
		cpp.useRPaths: false
	}

	Depends {
		name: "Qt"
		submodules: ["core", "gui", "widgets", "network", "xml"]
	}

    files: [
        "src/accesscontrolwidget.cpp",
        "src/accesslogtreeitem.cpp",
        "src/accesslogwidget.cpp",
        "src/configuration.cpp",
        "src/configurationwidget.cpp",
        "src/connectionpolicycombo.cpp",
        "src/counterlabel.cpp",
        "src/fileassociationsitemdelegate.cpp",
        "src/fileassociationswidget.cpp",
        "src/filenamewidget.cpp",
        "src/ipaddressconnectionpolicytreeitem.cpp",
        "src/iplistwidget.cpp",
        "src/main.cpp",
        "resources/resources.qrc",
        "resources/mimeicons.qrc",
        "resources/stylesheets.qrc",
        "src/mainwindow.cpp",
        "src/mainwindowstatusbar.cpp",
        "src/mimeicons.cpp",
        "src/mimetypeactionsdelegate.cpp",
        "src/mimetypeactionswidget.cpp",
        "src/mimetypecombo.cpp",
        "src/mimetypecomboaction.cpp",
        "src/requesthandler.cpp",
        "src/server.cpp",
        "src/serverdetailswidget.cpp",
        "src/serverfileassociationsmodel.cpp",
        "src/servermimeactionsmodel.cpp",
        "src/webserveractioncombo.cpp",
        "ui/accesscontrolwidget.ui",
        "ui/configurationwidget.ui",
        "ui/fileassociationswidget.ui",
        "ui/filenamewidget.ui",
        "ui/mainwindow.ui",
        "ui/mimetypeactionswidget.ui",
        "ui/serverdetailswidget.ui",
    ]

	Group {
		name: "Headers"
		files: [
         "src/accesscontrolwidget.h",
         "src/accesslogtreeitem.h",
         "src/accesslogwidget.h",
         "src/configuration.h",
         "src/configurationwidget.h",
         "src/connectionpolicycombo.h",
         "src/counterlabel.h",
         "src/fileassociationsitemdelegate.h",
         "src/fileassociationswidget.h",
         "src/filenamewidget.h",
         "src/ipaddressconnectionpolicytreeitem.h",
         "src/iplistwidget.h",
         "src/mainwindow.h",
         "src/mainwindowstatusbar.h",
         "src/metatypes.h",
         "src/mimeicons.h",
         "src/mimetypeactionsdelegate.h",
         "src/mimetypeactionswidget.h",
         "src/mimetypecomboaction.h",
         "src/mimetypecombo.h",
         "src/numerics.h",
         "src/requesthandler.h",
         "src/server.h",
         "src/serverdetailswidget.h",
         "src/serverfileassociationsmodel.h",
         "src/servermimeactionsmodel.h",
         "src/qtstdhash.h",
         "src/strings.h",
         "src/scopeguard.h",
         "src/types.h",
         "src/webserveractioncombo.h",
     ]
    }
    Group {
        name: "Stylesheets"
        files: [
            "resources/stylesheets/directory-listing.css"
        ]
    }
}
