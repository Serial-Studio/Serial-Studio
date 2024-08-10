import qbs

QtApplication {
    name: "Client"
    targetName: "qmqtt_example"

    files: [
        "example.cpp"
    ]

    Depends {
        name: "Qt"
        submodules: [
            "core",
            "network",
        ]
    }

    Depends {
        name: "qmqtt"
    }

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
