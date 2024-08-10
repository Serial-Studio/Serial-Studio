import qbs

Project {
    name: "GTest"
    property bool runUnitTests: true

    references: [
        "gtest",
        "tests",
    ]

    AutotestRunner {
        condition: runUnitTests && qbs.targetOS.contains("unix")

        Properties {
            condition: qbs.targetOS.contains("macx")

            /* TODO: if needed, add "install_name_tool" usage */
        }

        Depends {
            name: "Google-Test"
        }
    }
}
