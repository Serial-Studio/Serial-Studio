TEMPLATE = subdirs
SUBDIRS += \
    auto

unix:!NO_UNIT_TESTS: SUBDIRS += gtest

# benchmarks in debug mode is rarely sensible
contains(QT_CONFIG, release): SUBDIRS += benchmarks
