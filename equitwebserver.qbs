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
        "../../../.config/Equit/equitwebserver/defaultsettings.ewcx",
        "src/crc32.cpp",
        "src/accesscontrolwidget.cpp",
        "src/accesslogtreeitem.cpp",
        "src/accesslogwidget.cpp",
        "src/application.cpp",
        "src/configuration.cpp",
        "src/configurationwidget.cpp",
        "src/connectionpolicycombo.cpp",
        "src/counterlabel.cpp",
        "src/deflatecontentencoder.cpp",
        "src/deflatecontentencoder.h",
        "src/fileassociationsitemdelegate.cpp",
        "src/fileassociationswidget.cpp",
        "src/filenamewidget.cpp",
        "src/gzipcontentencoder.cpp",
        "src/identitycontentencoder.cpp",
        "src/inlinenotificationwidget.cpp",
        "src/iplineeditaction.cpp",
        "src/ippolicydelegate.cpp",
        "src/main.cpp",
        "resources/resources.qrc",
        "resources/mimeicons.qrc",
        "resources/stylesheets.qrc",
        "src/mainwindow.cpp",
        "src/mainwindowstatusbar.cpp",
        "src/mimecombo.cpp",
        "src/mimecombowidgetaction.cpp",
        "src/mimeicons.cpp",
        "src/mimetypeactionsdelegate.cpp",
        "src/mimetypeactionswidget.cpp",
        "src/requesthandler.cpp",
        "src/server.cpp",
        "src/serverdetailswidget.cpp",
        "src/serverfileassociationsmodel.cpp",
        "src/serveripconnectionpolicymodel.cpp",
        "src/servermimeactionsmodel.cpp",
        "src/webserveractioncombo.cpp",
        "src/window.cpp",
        "ui/accesscontrolwidget.ui",
        "ui/configurationwidget.ui",
        "ui/fileassociationswidget.ui",
        "ui/filenamewidget.ui",
        "ui/inlinenotificationwidget.ui",
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
         "src/application.h",
         "src/configuration.h",
         "src/configurationwidget.h",
         "src/connectionpolicycombo.h",
         "src/contentencoder.h",
         "src/counterlabel.h",
         "src/crc32.h",
         "src/fileassociationsitemdelegate.h",
         "src/fileassociationswidget.h",
         "src/filenamewidget.h",
         "src/gzipcontentencoder.h",
         "src/identitycontentencoder.h",
         "src/inlinenotificationwidget.h",
         "src/iplineeditaction.h",
         "src/ippolicydelegate.h",
         "src/mainwindow.h",
         "src/mainwindowstatusbar.h",
         "src/metatypes.h",
         "src/mimecombo.h",
         "src/mimecombowidgetaction.h",
         "src/mimeicons.h",
         "src/mimetypeactionsdelegate.h",
         "src/mimetypeactionswidget.h",
         "src/numerics.h",
         "src/requesthandler.h",
         "src/server.h",
         "src/serverdetailswidget.h",
         "src/serverfileassociationsmodel.h",
         "src/servermimeactionsmodel.h",
         "src/serveripconnectionpolicymodel.h",
         "src/qtstdhash.h",
         "src/strings.h",
         "src/scopeguard.h",
         "src/types.h",
         "src/webserveractioncombo.h",
         "src/window.h",
     ]
    }
    Group {
        name: "Stylesheets"
        files: [
            "resources/stylesheets/directory-listing.css"
        ]
    }
}
