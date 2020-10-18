mkdir build
cd build
g++ --version
qmake --version
qmake ../*.pro
make -j 4
