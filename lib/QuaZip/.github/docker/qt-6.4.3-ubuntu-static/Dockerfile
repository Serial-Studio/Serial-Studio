FROM ubuntu:22.04 AS builder

WORKDIR /build

RUN apt-get -y update && apt-get -y install build-essential cmake tar wget python3

RUN wget -qO- https://ftp.fau.de/qtproject/archive/qt/6.4/6.4.3/single/qt-everywhere-src-6.4.3.tar.xz | tar xJ

RUN qt-everywhere-src-6.4.3/configure -qt-zlib \
-static \
-opensource \
-confirm-license \
-optimize-size \
-nomake examples \
-nomake tests \
-no-dbus \
-no-fontconfig \
-no-opengl \
-no-openssl \
-no-gui \
-skip qtconnectivity \
-skip qtdatavis3d \
-skip qtdeclarative \
-skip qtdoc \
-skip qtactiveqt \
-skip qt3d \
-skip qtgraphs \
-skip qtgrpc \
-skip qtimageformats \
-skip qtlanguageserver \
-skip qtlocation \
-skip qthttpserver \
-skip qtmultimedia \
-skip qtopcua \
-skip qtpositioning \
-skip qtremoteobjects \
-skip qtscxml \
-skip qtsensors \
-skip qtserialbus \
-skip qtserialport \
-skip qtshadertools \
-skip qtspeech \
-skip qtsvg \
-skip qttools \
-skip qttranslations \
-skip qtwebchannel \
-skip qtwebengine \
-skip qtwebsockets \
-skip qtwebview \
-skip qtcharts \
-skip qtcoap \
-skip qtlottie \
-skip qtmqtt \
-skip qtnetworkauth \
-skip qtquick3d \
-skip qtquick3dphysics \
-skip qtquickeffectmaker \
-skip qtquicktimeline \
-skip qtvirtualkeyboard \
-skip qtwayland \
-skip qtcanvas3d \
-skip qtgamepad \
-skip qtpurchasing \
-skip qtscript

RUN cmake --build . --parallel --target install

FROM ubuntu:22.04

RUN apt-get -y update && apt-get -y install build-essential cmake

COPY --from=builder /usr/local/Qt-6.4.3 /usr/local/Qt-6.4.3
