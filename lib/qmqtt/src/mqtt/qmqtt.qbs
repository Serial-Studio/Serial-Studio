import qbs
import qbs.TextFile
import qbs.FileInfo

Product {
    name: "qmqtt"
    type: [libraryType]
    property bool webSocketSupport: false
    property string libraryType: "dynamiclibrary"
    targetName: "qmqtt"
    version: "1.0.3"

    cpp.defines: [
        "QT_BUILD_QMQTT_LIB",
        "QT_NO_CAST_TO_ASCII",
        "QT_NO_CAST_FROM_ASCII",
    ]

    cpp.includePaths: sourceDirectory

    files: [
        "*.cpp"
    ]

    Group {
        name: "Public Headers"
        fileTags: [
            "hpp",
            "public_headers"
        ]
        files: [
            "*.h"
        ]
        excludeFiles: privateHeaders.files
    }

    Group {
        id: privateHeaders
        name: "Private Headers"
        fileTags: [
            "hpp",
            "private_headers",
        ]
        files: [
            "*_p.h"
        ]
    }

    Group {
        fileTagsFilter: libraryType
        qbs.install: true
        qbs.installDir: "lib"
    }

    Group {
        fileTagsFilter: "public_headers"
        qbs.install: true
        qbs.installDir: FileInfo.joinPaths("include", product.name)
    }

    Group {
        fileTagsFilter: "private_headers"
        qbs.install: true
        qbs.installDir: FileInfo.joinPaths("include", product.name, version, product.name)
    }

    Group {
        fileTagsFilter: "Exporter.qbs.module"
        qbs.installDir: FileInfo.joinPaths("qbs", "modules", product.name, "private")
        qbs.install: true
    }

    Group {
        fileTagsFilter: "Exporter.pkgconfig.pc"
        qbs.installDir: FileInfo.joinPaths("lib" ,"pkgconfig")
        qbs.install: true
    }

    Depends {
        name: "cpp"
    }

    Depends {
        name: "Qt"
        property var baseModules: ["core", "network"]

        Properties {
            condition: webSocketSupport
            submodules: baseModules.concat(["websockets"])
        }
        Properties {
            condition: !webSocketSupport
            submodules: baseModules
        }
    }

    Depends {
        name: "Exporter.qbs"
    }
    Depends {
        name: "Exporter.pkgconfig"
    }

    Export {
        Depends {
            name: "cpp"
        }

        Depends {
            name: "Qt"
            property var baseModules: ["core", "network"]
            Properties {
                condition: product.webSocketSupport
                submodules: baseModules.concat(["websockets"])
            }
            Properties {
                condition: !product.webSocketSupport
                submodules: baseModules
            }
        }

        cpp.includePaths: [
            product.sourceDirectory,
            FileInfo.joinPaths(qbs.installRoot, qbs.installPrefix, "include"),
        ]
        prefixMapping: [
            {
                prefix: product.sourceDirectory,
                replacement: FileInfo.joinPaths(qbs.installRoot, qbs.installPrefix, "include",
                                                product.name)
            }
        ]
    }
}
