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
		"src/bpEditableTreeWidget.cpp",
		"src/ConnectionCountLabel.cpp",
		"src/bpIpListWidget.cpp",
		"src/HostNetworkInfo.cpp",
		"src/main.cpp",
		"src/MainWindow.cpp",
		"src/Configuration.cpp",
		"src/RequestHandler.cpp",
		"src/ConfigurationWidget.cpp",
		"src/Server.cpp",
	]

	Group {
		name: "Headers"
		files: [
			"src/bpEditableTreeWidget.h",
			"src/ConnectionCountLabel.h",
			"src/bpIpListWidget.h",
			"src/HostNetworkInfo.h",
			"src/MainWindow.h",
			"src/RequestHandler.h",
			"src/Configuration.h",
			"src/RequestHandlerResponseCodes.h",
			"src/ConfigurationWidget.h",
			"src/Server.h",
		]
}
