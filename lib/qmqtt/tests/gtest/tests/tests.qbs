import qbs
import qbs.Process

Product {
    name: "Tests"
    type: [
        "application",
    ]
    targetName: "qmqtt_tests"

    cpp.defines: [
        "QMQTT_LIBRARY_TESTS"
    ]

    files: [
        "clienttest.cpp",
        "tcpserver.cpp",
        "main.cpp",
        "customprinter.cpp",
        "networktest.cpp",
        "messagetest.cpp",
        "frametest.cpp",
        "sockettest.cpp",
        "tcpserver.h",
        "customprinter.h",
        "networkmock.h",
        "socketmock.h",
        "timermock.h",
        "iodevicemock.h",
    ]

    Depends {
        name: "Qt"
        submodules: [
            "core",
            "network",
            "testlib",
        ]
    }

    Depends {
        name: "Google-Test"
    }

    Depends {
        name: "qmqtt"
    }
}
