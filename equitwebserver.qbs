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
        "src/editabletreewidget.cpp",
        "src/fileassociationsitemdelegate.cpp",
        "src/fileassociationswidget.cpp",
        "src/ipaddressconnectionpolicytreeitem.cpp",
        "src/iplistwidget.cpp",
        "src/main.cpp",
        "resources/resources.qrc",
        "resources/mimeicons.qrc",
        "resources/stylesheets.qrc",
        "src/mainwindow.cpp",
        "src/mimeicons.cpp",
        "src/mimetypecombo.cpp",
        "src/requesthandler.cpp",
        "src/server.cpp",
        "src/serverconfigwidget.cpp",
        "src/serverfileassociationsmodel.cpp",
        "src/webserveractioncombo.cpp",
        "ui/accesscontrolwidget.ui",
        "ui/fileassociationswidget.ui",
        "ui/serverconfigwidget.ui",
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
         "src/editabletreewidget.h",
         "src/fileassociationsitemdelegate.h",
         "src/fileassociationextensionitem.h",
         "src/fileassociationmimetypeitem.h",
         "src/fileassociationswidget.h",
         "src/ipaddressconnectionpolicytreeitem.h",
         "src/iplistwidget.h",
         "src/mainwindow.h",
         "src/mimeicons.h",
         "src/mimetypecombo.h",
         "src/requesthandler.h",
         "src/server.h",
         "src/serverconfigwidget.h",
         "src/serverfileassociationsmodel.h",
         "src/qtstdhash.h",
         "src/strings.h",
         "src/scopeguard.h",
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
