#!/bin/bash
if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
  echo Before install on OSX.
  ./CI/travis.osx.before_install.sh;
fi
