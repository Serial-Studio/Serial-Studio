sudo add-apt-repository ppa:beineri/opt-qt-5.15.0-xenial -y
sudo apt-get update
sudo apt-get install qt515base qt515charts-no-lgpl qt515declarative qt515graphicaleffects qt515imageformats qt515location qt515quickcontrols qt515quickcontrols2 qt515serialport qt515svg qt515tools qt515x11extras libgl-dev checkinstall -y
source /opt/qt515/bin/qt515-env.sh
