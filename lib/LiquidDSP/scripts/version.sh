#!/bin/sh

# This script parses the global header and extracts the major.minor.patch version for
# liquid-dsp and prints to the screen. This is a simplified version of:
# https://github.com/tinyalsa/tinyalsa/blob/master/scripts/version.sh

INCLUDE_FILE="include/liquid.h"

# exit program with error
die()
{
    echo "error: $@" 1>&2
    exit 1
}

# check that file exists
check_files()
{
    [ -f ${INCLUDE_FILE}   ] || die "No ${INCLUDE_FILE} found!";
}

# get a part of the version from the header, e.g. LIQUID_VERSION_MAJOR
get_version_part()
{
    set -- "$1" "$(grep -m 1 "^#define\([ \t]*\)$1" ${INCLUDE_FILE} | sed 's/[^0-9]*//g')"

    if [ -z "$2" ]; then
        die "Could not get $1 from ${INCLUDE_FILE}"
    fi

    echo "$2"
}

# gets the complete version from the include file
get_version()
{
    VERSION_MAJOR=$(get_version_part "LIQUID_VERSION_MAJOR")
    VERSION_MINOR=$(get_version_part "LIQUID_VERSION_MINOR")
    VERSION_PATCH=$(get_version_part "LIQUID_VERSION_PATCH")
}

print_version()
{
    printf "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${LF}"
    return 0
}

check_files
get_version
print_version "$2"
exit $?

# The script should never reach this place.
die "Internal error. Please report this."

