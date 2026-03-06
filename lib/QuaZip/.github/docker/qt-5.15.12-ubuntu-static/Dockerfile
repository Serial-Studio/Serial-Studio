FROM ubuntu:22.04 as builder

WORKDIR /build

RUN apt-get -y update && apt-get -y install build-essential cmake tar wget python3

RUN wget -qO- https://ftp.fau.de/qtproject/archive/qt/5.15/5.15.12/single/qt-everywhere-opensource-src-5.15.12.tar.xz | tar xJ

RUN qt-everywhere-src-5.15.12/configure -qt-zlib \
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
-skip qtimageformats \
-skip qtlocation \
-skip qtmultimedia \
-skip qtopcua \
-skip qtremoteobjects \
-skip qtscxml \
-skip qtsensors \
-skip qtserialbus \
-skip qtserialport \
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
-skip qtquicktimeline \
-skip qtvirtualkeyboard \
-skip qtwayland \
-skip qtcanvas3d \
-skip qtgamepad \
-skip qtpurchasing \
-skip qtscript

RUN make -j$(nproc) && make install

FROM ubuntu:22.04

RUN apt-get -y update && apt-get -y install git build-essential cmake

COPY --from=builder /usr/local/Qt-5.15.12 /usr/local/Qt-5.15.12

# Temporary workaround for https://github.com/stachenov/quazip/issues/127
ENV LC_ALL=C.UTF-8