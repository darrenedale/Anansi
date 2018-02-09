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
        "src/configuration.cpp",
        "src/configurationwidget.cpp",
        "src/connectioncountlabel.cpp",
        "src/connectionpolicycombo.cpp",
        "src/dditabletreewidget.cpp",
        "src/hostnetworkinfo.cpp",
        "src/ipaddressconnectionpolicytreeitem.cpp",
        "src/iplistwidget.cpp",
        "src/main.cpp",
        "resources/resources.qrc",
        "resources/mimeicons.qrc",
        "resources/stylesheets.qrc",
        "src/mainwindow.cpp",
        "src/requesthandler.cpp",
        "src/server.cpp",
        "src/serverconfigwidget.cpp",
        "ui/accesscontrolwidget.ui",
        "ui/serverconfigwidget.ui",
    ]

	Group {
		name: "Headers"
		files: [
         "src/accesscontrolwidget.h",
         "src/configuration.h",
         "src/configurationwidget.h",
         "src/connectioncountlabel.h",
         "src/connectionpolicycombo.h",
         "src/editabletreewidget.h",
         "src/hostnetworkinfo.h",
         "src/ipaddressconnectionpolicytreeitem.h",
         "src/iplistwidget.h",
         "src/mainwindow.h",
         "src/requesthandler.h",
         "src/server.h",
         "src/serverconfigwidget.h",
         "src/qtstdhash.h",
         "src/strings.h",
         "src/scopeguard.h",
     ]
    }
    Group {
        name: "Stylesheets"
        files: [
            "resources/stylesheets/directory-listing.css"
        ]
    }
}
