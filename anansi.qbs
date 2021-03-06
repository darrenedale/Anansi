import qbs

CppApplication {
    name: "Anansi web server"
    targetName: "anansi"
    cpp.cxxLanguageVersion: "c++17"
	cpp.enableRtti: false

    Properties {
        condition: qbs.targetOS.contains("linux")
        cpp.includePaths: ["."]
        cpp.dynamicLibraries: ["z"]
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.includePaths: [".", "C:/Development/includes/"]
        cpp.dynamicLibraries: ["C:/Development/libs/msvc/x64/zlibwapi"]
    }

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
        "src/eqassert.cpp",
        "src/configuration.cpp",
        "src/configurationwidget.cpp",
        "src/connectionpolicycombo.cpp",
        "src/counterlabel.cpp",
        "src/directorylistingsortordercombo.cpp",
        "src/display_strings.cpp",
        "src/fileassociationsitemdelegate.cpp",
        "src/fileassociationsmodel.cpp",
        "src/fileassociationswidget.cpp",
        "src/filesystempathwidget.cpp",
        "src/identitycontentencoder.cpp",
        "src/inlinenotificationwidget.cpp",
        "src/ipconnectionpolicymodel.cpp",
        "src/iplineeditaction.cpp",
        "src/ippolicydelegate.cpp",
        "src/main.cpp",
        "src/mainwindow.cpp",
        "src/mainwindowstatusbar.cpp",
        "src/mediatypeactionsdelegate.cpp",
        "src/mediatypeactionsmodel.cpp",
        "src/mediatypeactionswidget.cpp",
        "src/mediatypecombo.cpp",
        "src/mediatypecombowidgetaction.cpp",
        "src/mediatypeicons.cpp",
        "src/requesthandler.cpp",
        "src/selectorpanel.cpp",
        "src/server.cpp",
        "src/serverdetailswidget.cpp",
        "src/startstopbutton.cpp",
        "src/webserveractioncombo.cpp",
        "src/windowbase.cpp",
        "src/zlibcontentencoder.cpp",
        "src/zlibdeflater.cpp",
        "resources/mediatypeicons.qrc",
        "resources/resources.qrc",
        "resources/stylesheets.qrc",
        "resources/inlinenotificationwidget.qrc",
        "ui/accesscontrolwidget.ui",
        "ui/accesslogwidget.ui",
        "ui/configurationwidget.ui",
        "ui/fileassociationswidget.ui",
        "ui/filesystempathwidget.ui",
        "ui/inlinenotificationwidget.ui",
        "ui/mainwindow.ui",
        "ui/mediatypeactionswidget.ui",
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
         "src/eqassert.h",
         "src/fileassociationsitemdelegate.h",
         "src/fileassociationsmodel.h",
         "src/fileassociationswidget.h",
         "src/filesystempathwidget.h",
         "src/gzipcontentencoder.h",
         "src/identitycontentencoder.h",
         "src/inlinenotificationwidget.h",
         "src/ipconnectionpolicymodel.h",
         "src/iplineeditaction.h",
         "src/ippolicydelegate.h",
         "src/macros.h",
         "src/mainwindow.h",
         "src/mainwindowstatusbar.h",
         "src/mediatypeactionsdelegate.h",
         "src/mediatypeactionsmodel.h",
         "src/mediatypeactionswidget.h",
         "src/mediatypecombo.h",
         "src/mediatypecombowidgetaction.h",
         "src/mediatypeicons.h",
         "src/metatypes.h",
         "src/notifications.h",
         "src/numerics.h",
         "src/qtmetatypes.h",
         "src/qtstdhash.h",
         "src/requesthandler.h",
         "src/scopeguard.h",
         "src/selectorpanel.h",
         "src/server.h",
         "src/serverdetailswidget.h",
         "src/startstopbutton.h",
         "src/strings.h",
         "src/types.h",
         "src/webserveractioncombo.h",
         "src/windowbase.h",
         "src/zlibcontentencoder.h",
         "src/zlibdeflater.h",
     ]
    }
    Group {
        name: "Documentation"
        files: [
            "docs/*.md",
            "docs/src/*.dox",
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
