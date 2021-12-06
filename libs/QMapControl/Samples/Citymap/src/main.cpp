#include <QApplication>
#include "citymap.h"

int main(int argc, char *argv[])
{
      QApplication app(argc, argv);
      Citymap * mw = new Citymap();
		
      mw->resize(400,590);
      mw->setWindowTitle("City Map Mainz");
      mw->show();
      return app.exec();
}

