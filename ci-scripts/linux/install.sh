sudo apt-get -qq update
sudo apt-get -qq install libgl1-mesa-dev libxkbcommon-x11-0
sudo apt-add-repository -y ppa:ubuntu-toolchain-r/test
sudo add-apt-repository -y ppa:jonathonf/gcc-9
sudo apt-get update
sudo apt-get -qq install libc6-i386 build-essential libgl1-mesa-dev mesa-common-dev libgles2-mesa-dev libxkbcommon-x11-0 libxcb-icccm4-dev libxcb-xinerama0 libxcb-image0 libxcb-keysyms1
sudo apt-get -qq install libxcb-*
sudo apt-get -qq qtbase5-dev qt5-qmake qt5-default libqt5opengl5 qml-module-qtquick-controls2 libqt5charts5 libqt5charts5-dev
sudo apt-get -qq remove gcc g++
sudo apt-get install gcc-9 g++-9
sudo ln -s /usr/bin/g++-9 /usr/bin/g++
sudo ln -s /usr/bin/gcc-9 /usr/bin/gcc
