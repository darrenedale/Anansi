import qbs

CppApplication {
    name: "Anansi web server"
    targetName: "anansi"
    cpp.cxxLanguageVersion: "c++17"
	cpp.enableRtti: false
    cpp.includePaths: ["."]
    cpp.dynamicLibraries: ["z"]

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
        "src/application.cpp",
        "src/configuration.cpp",
        "src/configurationwidget.cpp",
        "src/connectionpolicycombo.cpp",
        "src/counterlabel.cpp",
        "src/deflatecontentencoder.cpp",
        "src/directorylistingsortordercombo.cpp",
        "src/display_strings.cpp",
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
        "src/mimeactionswidget.cpp",
        "src/mimecombo.cpp",
        "src/mimecombowidgetaction.cpp",
        "src/mimeicons.cpp",
        "src/mimetypeactionsdelegate.cpp",
        "src/requesthandler.cpp",
        "src/selectorpanel.cpp",
        "src/server.cpp",
        "src/serverdetailswidget.cpp",
        "src/serverfileassociationsmodel.cpp",
        "src/serveripconnectionpolicymodel.cpp",
        "src/servermimeactionsmodel.cpp",
        "src/startstopbutton.cpp",
        "src/webserveractioncombo.cpp",
        "src/window.cpp",
        "src/zlibcontentencoder.cpp",
        "src/zlibdeflater.cpp",
        "ui/accesscontrolwidget.ui",
        "ui/accesslogwidget.ui",
        "ui/configurationwidget.ui",
        "ui/fileassociationswidget.ui",
        "ui/filenamewidget.ui",
        "ui/inlinenotificationwidget.ui",
        "ui/mainwindow.ui",
        "ui/mimeactionswidget.ui",
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
         "src/deflatecontentencoder.h",
         "src/directorylistingsortordercombo.h",
         "src/display_strings.h",
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
         "src/mimeactionswidget.h",
         "src/mimecombo.h",
         "src/mimecombowidgetaction.h",
         "src/mimeicons.h",
         "src/mimetypeactionsdelegate.h",
         "src/notifications.h",
         "src/numerics.h",
         "src/qtmetatypes.h",
         "src/requesthandler.h",
         "src/selectorpanel.h",
         "src/server.h",
         "src/serverdetailswidget.h",
         "src/serverfileassociationsmodel.h",
         "src/servermimeactionsmodel.h",
         "src/serveripconnectionpolicymodel.h",
         "src/qtstdhash.h",
         "src/strings.h",
         "src/scopeguard.h",
         "src/startstopbutton.h",
         "src/types.h",
         "src/webserveractioncombo.h",
         "src/window.h",
         "src/zlibcontentencoder.h",
         "src/zlibdeflater.h",
     ]
    }
    Group {
        name: "Documentation"
        files: [
            "docs/*.md",
            "docs/src/*.md",
        ]
    }

    Group {
        name: "Stylesheets"
        files: [
            "resources/stylesheets/directory-listing.css"
        ]
    }
}
