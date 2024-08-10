import qbs

DynamicLibrary {
    name: "Google-Test"
    targetName: "gtest"

    cpp.defines: [
        "GTEST_LIBRARY"
    ]

    files: [
        "googletest/googletest/src/gtest-all.cc",
        "googletest/googlemock/src/gmock-all.cc",
    ]

    cpp.includePaths: [
        "googletest/googletest/include",
        "googletest/googletest",
        "googletest/googlemock/include",
        "googletest/googlemock",
    ]

    Depends {
        name: "Qt"
        submodules: [
            "core"
        ]
    }

    Export {
        Depends {
            name: "cpp"
        }

        cpp.includePaths: [
            "googletest/googletest/include",
            "googletest/googlemock/include",
        ]
    }
}
