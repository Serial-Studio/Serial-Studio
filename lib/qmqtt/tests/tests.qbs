import qbs

Project {
    name: "Tests"
    /* NOTE: to change this just add "Tests.buildUnitTests:false" or "Tests.buildUnitTests:true" to qbs call */
    property bool buildUnitTests: true

    references: [
        "auto"
    ]

    SubProject {
        filePath: "gtest/gtest.qbs"
        Properties {
            condition: qbs.targetOS.contains("unix") && buildUnitTests
        }
    }
}
