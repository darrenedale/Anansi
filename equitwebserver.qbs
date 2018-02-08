import qbs

CppApplication {
	name: "EquitWebServer"
	targetName: "equitwebserver"
    cpp.cxxLanguageVersion: "c++17"
	cpp.enableRtti: false

    Group {
		condition: qbs.buildVariant.contains("release")
		cpp.useRPaths: false
	}

	Depends {
		name: "Qt"
		submodules: ["core", "gui", "widgets", "network", "xml"]
	}

    files: [
        "src/IpListWidget.cpp",
        "src/bpEditableTreeWidget.cpp",
        "src/ConnectionCountLabel.cpp",
        "src/HostNetworkInfo.cpp",
        "src/connectionpolicycombo.cpp",
        "src/connectionpolicycombo.h",
        "src/main.cpp",
        "src/MainWindow.cpp",
        "src/Configuration.cpp",
        "src/RequestHandler.cpp",
        "src/ConfigurationWidget.cpp",
        "src/Server.cpp",
        "resources/resources.qrc",
        "resources/mimeicons.qrc",
        "resources/stylesheets.qrc",
        "src/serverconfigwidget.cpp",
        "ui/serverconfigwidget.ui",
    ]

	Group {
		name: "Headers"
		files: [
            "src/IpListWidget.h",
            "src/bpEditableTreeWidget.h",
            "src/ConnectionCountLabel.h",
            "src/HostNetworkInfo.h",
            "src/MainWindow.h",
            "src/RequestHandler.h",
            "src/Configuration.h",
            "src/ConfigurationWidget.h",
            "src/Server.h",
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
